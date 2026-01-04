#include "ParticleSystem.h"
#include <GL/gl3w.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "GLUtils.h"
#include "noise.h"
#include "uniforms.h"

static float frand()
{
    return rand() / (float) RAND_MAX;
}

static float sfrand()
{
    return frand()*2.0f-1.0f;
}

ParticleSystem::ParticleSystem(size_t size, const char* shaderPrefix) :
    m_size(size),
    m_noiseTex(0),
    m_noiseSize(16),
    m_updateProg(0),
    m_shaderPrefix(shaderPrefix)
{
    m_pos = new ShaderBuffer<glm::vec4>(size);
    m_vel = new ShaderBuffer<glm::vec4>(size);
    m_indices = new ShaderBuffer<uint32_t>(size*6);

    uint32_t *indices = m_indices->map();
    for(size_t i=0; i<m_size; i++) {
        uint32_t index = uint32_t(i<<2);
        *(indices++) = index;
        *(indices++) = index+1;
        *(indices++) = index+2;
        *(indices++) = index;
        *(indices++) = index+2;
        *(indices++) = index+3;
    }
    m_indices->unmap();

    m_noiseTex = createNoiseTexture4f3D(m_noiseSize, m_noiseSize, m_noiseSize, GL_RGBA8_SNORM);

    loadShaders();

    reset(0.5f);
}

static inline const char *GetShaderStageName(GLenum target)
{
    switch(target) {
    case GL_VERTEX_SHADER:
        return "VERTEX_SHADER";
        break;
    case GL_FRAGMENT_SHADER:
        return "FRAGMENT_SHADER";
        break;
    case GL_COMPUTE_SHADER:
        return "COMPUTE_SHADER";
        break;
    }
    return "";
}

GLuint ParticleSystem::createComputeProgram(const char* src)
{
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    if (computeShader == 0) {
        std::cerr << "Failed to create compute shader" << std::endl;
        CHECK_GL_ERROR();
        return 0;
    }

    std::string srcStr(src);
    bool hasVersion = srcStr.find("#version") != std::string::npos;
    
    const GLchar* fullSrc[2];
    int srcCount = 2;
    if (hasVersion) {
        fullSrc[0] = src;
        srcCount = 1;
    } else {
        fullSrc[0] = m_shaderPrefix;
        fullSrc[1] = src;
    }

    glShaderSource(computeShader, srcCount, fullSrc, nullptr);
    glCompileShader(computeShader);
    
    GLint compileStatus;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
        GLint logLength;
        glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            char *log = new char [logLength];
            glGetShaderInfoLog(computeShader, logLength, 0, log);
            std::cerr << "=== Compute shader compilation failed ===" << std::endl;
            std::cerr << log << std::endl;
            std::cerr << "=========================================" << std::endl;
            delete [] log;
        } else {
            std::cerr << "Compute shader compilation failed (no log available)" << std::endl;
        }
        glDeleteShader(computeShader);
        CHECK_GL_ERROR();
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        std::cerr << "Failed to create program" << std::endl;
        glDeleteShader(computeShader);
        CHECK_GL_ERROR();
        return 0;
    }

    glAttachShader(program, computeShader);
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            char *log = new char [logLength];
            glGetProgramInfoLog(program, logLength, 0, log);
            std::cerr << "=== Compute shader program link failed ===" << std::endl;
            std::cerr << log << std::endl;
            std::cerr << "==========================================" << std::endl;
            delete [] log;
        } else {
            std::cerr << "Compute shader program link failed (no log available)" << std::endl;
        }
        glDeleteProgram(program);
        glDeleteShader(computeShader);
        CHECK_GL_ERROR();
        return 0;
    }

    glDeleteShader(computeShader);

    GLboolean isProgram = glIsProgram(program);
    if (isProgram == GL_FALSE) {
        std::cerr << "Error: Created program is not a valid program object" << std::endl;
        glDeleteProgram(program);
        CHECK_GL_ERROR();
        return 0;
    }

    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        std::cerr << "Error: Program link status check failed after creation" << std::endl;
        glDeleteProgram(program);
        CHECK_GL_ERROR();
        return 0;
    }

    {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1) { 
            char *log = new char [logLength];
            glGetProgramInfoLog(program, logLength, 0, log);
            std::cout << "Compute shader program info:\n" << log << std::endl;
            delete [] log;
        }
    }

    CHECK_GL_ERROR();
    std::cout << "Compute program created successfully, ID: " << program << std::endl;
    return program;
}

void ParticleSystem::loadShaders()
{
    if (m_updateProg) {
        glDeleteProgram(m_updateProg);
        m_updateProg = 0;
    }

    std::ifstream shaderFile("assets/shaders/particlePass.cs");
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open compute shader file: assets/shaders/particlePass.cs" << std::endl;
        return;
    }
    std::stringstream shaderBuffer;
    shaderBuffer << shaderFile.rdbuf();
    std::string src = shaderBuffer.str();
    shaderFile.close();
    
    if (src.empty()) {
        std::cerr << "Failed to load compute shader source (file is empty)" << std::endl;
        return;
    }
    
    std::cout << "Compute shader source loaded, length: " << src.length() << " bytes" << std::endl;
    
    m_updateProg = createComputeProgram(src.c_str());
    if (m_updateProg == 0) {
        std::cerr << "Failed to create compute shader program - check error messages above" << std::endl;
        std::cerr << "Shader source preview (first 500 chars):" << std::endl;
        std::cerr << src.substr(0, 500) << std::endl;
        return;
    }
    
    std::cout << "Compute shader program created successfully, ID: " << m_updateProg << std::endl;

    glUseProgram(m_updateProg);

    GLint loc = glGetUniformLocation(m_updateProg, "invNoiseSize");
    if (loc >= 0) {
        glUniform1f(loc, 1.0f / m_noiseSize);
    } else {
        std::cerr << "Warning: uniform 'invNoiseSize' not found in compute shader" << std::endl;
    }

    loc = glGetUniformLocation(m_updateProg, "noiseTex3D");
    if (loc >= 0) {
        glUniform1i(loc, 0);
    } else {
        std::cerr << "Warning: uniform 'noiseTex3D' not found in compute shader" << std::endl;
    }

    glUseProgram(0);
    CHECK_GL_ERROR();
}

ParticleSystem::~ParticleSystem()
{
    delete m_pos;
    delete m_vel;
    delete m_indices;

    if (m_updateProg) {
        glDeleteProgram(m_updateProg);
    }
    if (m_noiseTex) {
        glDeleteTextures(1, &m_noiseTex);
    }
}

void ParticleSystem::reset(float size)
{
    glm::vec4 *pos = m_pos->map();
    for(size_t i=0; i<m_size; i++) {
        pos[i] = glm::vec4(sfrand()*size, sfrand()*size, sfrand()*size, 1.0f);
    }
    m_pos->unmap();

    glm::vec4 *vel = m_vel->map();
    for(size_t i=0; i<m_size; i++) {
        vel[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    m_vel->unmap();
}

void ParticleSystem::resetToHeartShape(float scale)
{
    glm::vec4 *pos = m_pos->map();
    const float pi = 3.14159265359f;
    
    for(size_t i=0; i<m_size; i++) {
        float t = float(i) / float(m_size);
        
        float u = t * 2.0f * pi;
        
        float x = 16.0f * sinf(u) * sinf(u) * sinf(u);
        float y = 13.0f * cosf(u) - 5.0f * cosf(2.0f * u) - 2.0f * cosf(3.0f * u) - cosf(4.0f * u);
        
        float heartScale = scale / 20.0f;
        x *= heartScale;
        y *= heartScale;
        
        float z = sinf(u * 2.0f) * heartScale * 0.5f;
        
        pos[i] = glm::vec4(x, y, z, 1.0f);
    }
    m_pos->unmap();
    
    glm::vec4 *vel = m_vel->map();
    for(size_t i=0; i<m_size; i++) {
        vel[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    m_vel->unmap();
}

void ParticleSystem::update()
{
    if (m_updateProg == 0) {
        std::cerr << "Error: Invalid compute shader program (m_updateProg is 0)" << std::endl;
        return;
    }

    glUseProgram(m_updateProg);
    CHECK_GL_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_noiseTex);
    CHECK_GL_ERROR();

    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2,  m_pos->getBuffer() );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3,  m_vel->getBuffer() );
    CHECK_GL_ERROR();

    GLuint numGroups = (GLuint)((m_size + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE);
    if (numGroups == 0) numGroups = 1;
    glDispatchCompute(numGroups, 1, 1);
    CHECK_GL_ERROR();

    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
    CHECK_GL_ERROR();

    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3,  0 );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2,  0 );
    glBindTexture(GL_TEXTURE_3D, 0);
    glUseProgram(0);
    CHECK_GL_ERROR();
}

