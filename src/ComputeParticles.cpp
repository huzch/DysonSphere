#include "ComputeParticles.h"
#include "ParticleSystem.h"
#include "GLUtils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

ComputeParticles::ComputeParticles() 
    : mEnableAttractor(false),
    mAnimate(true),
    mTime(0.0f),
    mWidth(800),
    mHeight(600),
    mCameraPos(0.0f, 0.0f, -3.0f),
    mCameraTarget(0.0f, 0.0f, 0.0f),
    mRenderProg(nullptr),
    mUBO(0),
    mVBO(0),
    mVAO(0),
    mLeftMousePressed(false),
    mRightMousePressed(false),
    mLastMouseX(0.0),
    mLastMouseY(0.0),
    mCameraDistance(3.0f),
    mCameraAzimuth(0.0f),
    mCameraElevation(0.0f),
    mParticleState(HeartShape),
    mTargetShapeState(HeartShape),
    mStateTime(0.0f),
    mAbsorbDuration(2.0f),
    mHeartDuration(5.0f),
    mSceneFBO(0),
    mSceneTexture(0),
    mBloomExtractProg(nullptr),
    mBloomDownsampleProg(nullptr),
    mBloomUpsampleProg(nullptr),
    mBloomCombineProg(nullptr),
    mScreenQuadVAO(0),
    mScreenQuadVBO(0)
{
    for (int i = 0; i < 4; i++) {
        mBloomFBO[i] = 0;
        mBloomTexture[i] = 0;
    }
    for (int i = 0; i < 3; i++) {
        mBloomUpsampleFBO[i] = 0;
        mBloomUpsampleTexture[i] = 0;
    }
}

ComputeParticles::~ComputeParticles()
{
    destroyBloomResources();
    
    if (mRenderProg) {
        delete mRenderProg;
        mRenderProg = nullptr;
    }
    
    if (mParticles) {
        delete mParticles;
        mParticles = nullptr;
    }
    
    if (mUBO) {
        glDeleteBuffers(1, &mUBO);
        mUBO = 0;
    }
    if (mVBO) {
        glDeleteBuffers(1, &mVBO);
        mVBO = 0;
    }
    if (mVAO) {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }

}

bool ComputeParticles::init(GLFWwindow* window)
{
    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    if (major < 4 || (major == 4 && minor < 3)) {
        std::cerr << "错误: 需要OpenGL 4.3或更高版本来支持计算着色器" << std::endl;
        return false;
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    CHECK_GL_ERROR();
    
    const char* shaderPrefix = "#version 430\n";
    
    // 加载渲染着色器文件
    std::ifstream vsFile("assets/shaders/basePass.verrt");
    if (!vsFile.is_open()) {
        std::cerr << "错误: 无法打开顶点着色器文件: assets/shaders/basePass.verrt" << std::endl;
        return false;
    }
    std::stringstream vsBuffer;
    vsBuffer << vsFile.rdbuf();
    std::string renderVS = vsBuffer.str();
    vsFile.close();
    
    // 加载渲染着色器文件
    std::ifstream fsFile("assets/shaders/basePass.frag");
    if (!fsFile.is_open()) {
        std::cerr << "错误: 无法打开片段着色器文件: assets/shaders/basePass.frag" << std::endl;
        return false;
    }
    std::stringstream fsBuffer;
    fsBuffer << fsFile.rdbuf();
    std::string renderFS = fsBuffer.str();
    fsFile.close();
    
    std::string renderVSWithVersion = renderVS;
    std::string renderFSWithVersion = renderFS;
    
    mRenderProg = new ShaderProgram();
    if (!mRenderProg->loadFromStrings(renderVSWithVersion.c_str(), renderFSWithVersion.c_str())) {
        std::cerr << "错误: 加载渲染着色器失败" << std::endl;
        return false;
    }
    CHECK_GL_ERROR();
    
    glGenBuffers(1, &mUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderParams), &mShaderParams, GL_STREAM_DRAW);
    CHECK_GL_ERROR();
    
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    CHECK_GL_ERROR();
    
    float vtx_data[] = { 0.0f, 0.0f, 0.0f, 1.0f};
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_data), vtx_data, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    
    glBindVertexArray(0);
    CHECK_GL_ERROR();
    
    mParticleCount = mNumParticles;
    mParticles = new ParticleSystem(mParticleCount, shaderPrefix);
    
    mParticles->resetToHeartShape(0.3f);
    CHECK_GL_ERROR();
    
    //int cx, cy, cz;
    //glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &cx);
    //glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &cy);
    //glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &cz);
    //std::cout << "最大计算工作组数量 = " << cx << ", " << cy << ", " << cz << std::endl;
    //
    //int sx, sy, sz;
    //glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &sx);
    //glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &sy);
    //glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &sz);
    //std::cout << "最大计算工作组大小 = " << sx << ", " << sy << ", " << sz << std::endl;
    
    CHECK_GL_ERROR();
    
    mCameraAzimuth = glm::pi<float>();
    mCameraElevation = 0.0f;
    
    mViewMatrix = glm::lookAt(mCameraPos, mCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    
    initBloomResources();
    
    return true;
}

void ComputeParticles::reshape(int width, int height)
{
    mWidth = width;
    mHeight = height;
    glViewport(0, 0, width, height);
    CHECK_GL_ERROR();
    
    destroyBloomResources();
    initBloomResources();
}

void ComputeParticles::handleKey(int key, int action)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_SPACE:
                mAnimate = !mAnimate;
                std::cout << "Animate: " << (mAnimate ? "On" : "Off") << std::endl;
                break;
                
            case GLFW_KEY_A:
                mEnableAttractor = !mEnableAttractor;
                break;                
            case GLFW_KEY_R:
                reset();
                break;
        }
    }
}

void ComputeParticles::reset()
{
    mTime = 0.0f;
    mParticleState = Normal;
    mStateTime = 0.0f;
    
    if (mParticles) {
        mParticles->reset(0.5f);
    }
}

void ComputeParticles::handleMouseButton(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mLeftMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            mLeftMousePressed = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            mRightMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            mRightMousePressed = false;
        }
    }
}

void ComputeParticles::handleMouseMove(double xpos, double ypos)
{
    if (mLeftMousePressed || mRightMousePressed) {
        double dx = xpos - mLastMouseX;
        double dy = ypos - mLastMouseY;
        
        if (mLeftMousePressed) {
            const float sensitivity = 0.005f; 
            mCameraAzimuth -= static_cast<float>(dx) * sensitivity;
            mCameraElevation -= static_cast<float>(dy) * sensitivity;
            
            const float maxElevation = glm::pi<float>() * 0.5f - 0.1f;
            mCameraElevation = glm::clamp(mCameraElevation, -maxElevation, maxElevation);
        }
        else if (mRightMousePressed) {
            const float sensitivity = 0.002f; 
            
            glm::vec3 forward = glm::normalize(mCameraTarget - mCameraPos);
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::cross(right, forward);
            
            glm::vec3 pan = -right * static_cast<float>(dx) * sensitivity * mCameraDistance +
                            up * static_cast<float>(dy) * sensitivity * mCameraDistance;
            
            mCameraTarget += pan;
        }
    }
    
    mLastMouseX = xpos;
    mLastMouseY = ypos;
}

void ComputeParticles::handleScroll(double xoffset, double yoffset)
{
    const float zoomSpeed = 0.1f;
    mCameraDistance -= static_cast<float>(yoffset) * zoomSpeed;
    
    mCameraDistance = glm::clamp(mCameraDistance, 0.5f, 10.0f);
}

void ComputeParticles::draw(float deltaTime)
{
    float aspect = (float)mWidth / (float)mHeight;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
    
    float cosElevation = cosf(mCameraElevation);
    mCameraPos.x = mCameraTarget.x + mCameraDistance * cosElevation * sinf(mCameraAzimuth);
    mCameraPos.y = mCameraTarget.y + mCameraDistance * sinf(mCameraElevation);
    mCameraPos.z = mCameraTarget.z + mCameraDistance * cosElevation * cosf(mCameraAzimuth);
    
    mViewMatrix = glm::lookAt(mCameraPos, mCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    
    mShaderParams.numParticles = (unsigned int)(mParticles->getSize());
    mShaderParams.ModelView = mViewMatrix;
    mShaderParams.ModelViewProjection = projectionMatrix * mViewMatrix;
    mShaderParams.ProjectionMatrix = projectionMatrix;
    
    const float baseSpriteSize = 0.015f;
    const float breathingAmplitude = 0.005f; 
    const float breathingSpeed = 2.0f; 
    mShaderParams.spriteSize = baseSpriteSize + sinf(mTime * breathingSpeed) * breathingAmplitude;
    
    if (mParticleState == Normal) {
        const float baseScale = 1.0f;
        const float scaleAmplitude = 0.1f;
        mShaderParams.particleScale = baseScale + sinf(mTime * breathingSpeed) * scaleAmplitude;
    } else {
        mShaderParams.particleScale = 1.0f;
    }
    
    mStateTime += deltaTime;
    
    if (mParticleState == Absorbing) {
        // 吸收
        mShaderParams.particleState = 1.0f;
        mShaderParams.stateTime = mStateTime;
        mShaderParams.attractor.w = 0.0f;
        mShaderParams.heartScale = 0.3f;
        
        if (mStateTime >= mAbsorbDuration) {
            mParticleState = mTargetShapeState;
            mStateTime = 0.0f;
            if (mTargetShapeState == ParticleState::HeartShape) {
               // std::cout << "正在形成爱心形状..." << std::endl;
            } else {
              //  std::cout << "正在形成五角星形状..." << std::endl;
            }
        }
    }
    else if (mParticleState == ParticleState::HeartShape) {
        mShaderParams.particleState = 2.0f;
        mShaderParams.stateTime = mStateTime;
        mShaderParams.attractor.w = 0.0f; 
        mShaderParams.heartScale = 0.3f; 
        
        if (mStateTime >= mHeartDuration) {
            mParticleState = ParticleState::Normal;
            mStateTime = 0.0f;
        }
    }
    else if (mParticleState == ParticleState::StarShape) {
        mShaderParams.particleState = 3.0f;
        mShaderParams.stateTime = mStateTime;
        mShaderParams.attractor.w = 0.0f; 
        mShaderParams.heartScale = 0.3f;  
        
        if (mStateTime >= mHeartDuration) {
            mParticleState = ParticleState::Normal;
            mStateTime = 0.0f;
        }
    }
    else {
        mShaderParams.particleState = 0.0f;
        mShaderParams.stateTime = 0.0f;
        mShaderParams.heartScale = 0.3f; 
        
        if (mEnableAttractor) {
            const float speed = 0.2f;
            mShaderParams.attractor.x = sinf(mTime * speed);
            mShaderParams.attractor.y = sinf(mTime * speed * 1.3f);
            mShaderParams.attractor.z = cosf(mTime * speed);
            mShaderParams.attractor.w = 0.0002f;  
        } else {
            mShaderParams.attractor.w = 0.0f; 
        }
    }
    
    mTime += deltaTime;
      
    glActiveTexture(GL_TEXTURE0);
    
    glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderParams), &mShaderParams);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, mUBO);
    
    if (mAnimate) {
        mParticles->update();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mSceneFBO);
    glViewport(0, 0, mWidth, mHeight);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mRenderProg->enable();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glBindVertexArray(mVAO);
    CHECK_GL_ERROR();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mParticles->getPosBuffer()->getBuffer());
    CHECK_GL_ERROR();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mParticles->getIndexBuffer()->getBuffer());
    CHECK_GL_ERROR();
    
    glDrawElements(GL_TRIANGLES, GLsizei(mParticles->getSize() * 6), GL_UNSIGNED_INT, 0);
    CHECK_GL_ERROR();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
    CHECK_GL_ERROR();

    glDisable(GL_BLEND);
    
    mRenderProg->disable();

    renderBloom();
}

void ComputeParticles::createScreenQuad()
{
    // Create screen quad for post-processing
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &mScreenQuadVAO);
    glGenBuffers(1, &mScreenQuadVBO);
    
    glBindVertexArray(mScreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mScreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
    CHECK_GL_ERROR();
}

void ComputeParticles::initBloomResources()
{
    if (mWidth <= 0 || mHeight <= 0) return;
    
    createScreenQuad();
    
    std::ifstream extractVSFile("assets/shaders/bloomExtractVS.glsl");
    std::ifstream extractFSFile("assets/shaders/bloomExtractFS.glsl");
    std::ifstream downsampleFSFile("assets/shaders/bloomDownsampleFS.glsl");
    std::ifstream upsampleFSFile("assets/shaders/bloomUpsampleBilateralFS.glsl");
    std::ifstream combineFSFile("assets/shaders/bloomCombineFS.glsl");
    
    if (!extractVSFile.is_open() || !extractFSFile.is_open() || 
        !downsampleFSFile.is_open() || !upsampleFSFile.is_open() || !combineFSFile.is_open()) {
        std::cerr << "错误: 无法打开Bloom着色器文件" << std::endl;
        return;
    }
    
    std::stringstream extractVSBuffer, extractFSBuffer, downsampleFSBuffer, upsampleFSBuffer, combineFSBuffer;
    extractVSBuffer << extractVSFile.rdbuf();
    extractFSBuffer << extractFSFile.rdbuf();
    downsampleFSBuffer << downsampleFSFile.rdbuf();
    upsampleFSBuffer << upsampleFSFile.rdbuf();
    combineFSBuffer << combineFSFile.rdbuf();
    
    extractVSFile.close();
    extractFSFile.close();
    downsampleFSFile.close();
    upsampleFSFile.close();
    combineFSFile.close();
    
    std::string extractVS = extractVSBuffer.str();
    std::string extractFS = extractFSBuffer.str();
    std::string downsampleFS = downsampleFSBuffer.str();
    std::string upsampleFS = upsampleFSBuffer.str();
    std::string combineFS = combineFSBuffer.str();

    mBloomExtractProg = new ShaderProgram();
    if (!mBloomExtractProg->loadFromStrings(extractVS.c_str(), extractFS.c_str())) {
        std::cerr << "错误: 加载Bloom提取着色器失败" << std::endl;
        delete mBloomExtractProg;
        mBloomExtractProg = nullptr;
        return;
    }
    
    mBloomDownsampleProg = new ShaderProgram();
    if (!mBloomDownsampleProg->loadFromStrings(extractVS.c_str(), downsampleFS.c_str())) {
        std::cerr << "错误: 加载Bloom降采样着色器失败" << std::endl;
        delete mBloomDownsampleProg;
        mBloomDownsampleProg = nullptr;
        return;
    }
    
    mBloomUpsampleProg = new ShaderProgram();
    if (!mBloomUpsampleProg->loadFromStrings(extractVS.c_str(), upsampleFS.c_str())) {
        std::cerr << "错误: 加载Bloom上采样着色器失败" << std::endl;
        delete mBloomUpsampleProg;
        mBloomUpsampleProg = nullptr;
        return;
    }
    
    mBloomCombineProg = new ShaderProgram();
    if (!mBloomCombineProg->loadFromStrings(extractVS.c_str(), combineFS.c_str())) {
        std::cerr << "错误: 加载Bloom合成着色器失败" << std::endl;
        delete mBloomCombineProg;
        mBloomCombineProg = nullptr;
        return;
    }
    
    CHECK_GL_ERROR();
    
    glGenFramebuffers(1, &mSceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mSceneFBO);
    
    glGenTextures(1, &mSceneTexture);
    glBindTexture(GL_TEXTURE_2D, mSceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mSceneTexture, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "错误: 场景FBO创建不完整: " << status << std::endl;
    }
    
    CHECK_GL_ERROR();
    
    int bloomWidth = mWidth;
    int bloomHeight = mHeight;
    for (int i = 0; i < 4; i++) {
        if (i > 0) {
            bloomWidth /= 2;
            bloomHeight /= 2;
        }
        
        glGenFramebuffers(1, &mBloomFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, mBloomFBO[i]);
        
        glGenTextures(1, &mBloomTexture[i]);
        glBindTexture(GL_TEXTURE_2D, mBloomTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bloomWidth, bloomHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBloomTexture[i], 0);
        
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "错误: Bloom降采样FBO " << i << " 创建不完整: " << status << std::endl;
        }
    }
    
    // 创建Bloom上采样FBO和纹理
    // upsample[0]: 1/4分辨率
    // upsample[1]: 1/2分辨率
    // upsample[2]: 全分辨率
    for (int i = 0; i < 3; i++) {
        int level = 2 - i;
        int width = mWidth >> (level + 1);
        int height = mHeight >> (level + 1);
        
        if (level == 0) {
            width = mWidth;
            height = mHeight;
        }
        
        glGenFramebuffers(1, &mBloomUpsampleFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, mBloomUpsampleFBO[i]);
        
        glGenTextures(1, &mBloomUpsampleTexture[i]);
        glBindTexture(GL_TEXTURE_2D, mBloomUpsampleTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBloomUpsampleTexture[i], 0);

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "错误: Bloom上采样FBO " << i << " 创建不完整: " << status << std::endl;
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR();
}

void ComputeParticles::destroyBloomResources()
{
    if (mSceneFBO) {
        glDeleteFramebuffers(1, &mSceneFBO);
        mSceneFBO = 0;
    }
    if (mSceneTexture) {
        glDeleteTextures(1, &mSceneTexture);
        mSceneTexture = 0;
    }
    
    for (int i = 0; i < 4; i++) {
        if (mBloomFBO[i]) {
            glDeleteFramebuffers(1, &mBloomFBO[i]);
            mBloomFBO[i] = 0;
        }
        if (mBloomTexture[i]) {
            glDeleteTextures(1, &mBloomTexture[i]);
            mBloomTexture[i] = 0;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        if (mBloomUpsampleFBO[i]) {
            glDeleteFramebuffers(1, &mBloomUpsampleFBO[i]);
            mBloomUpsampleFBO[i] = 0;
        }
        if (mBloomUpsampleTexture[i]) {
            glDeleteTextures(1, &mBloomUpsampleTexture[i]);
            mBloomUpsampleTexture[i] = 0;
        }
    }

    if (mBloomExtractProg) {
        delete mBloomExtractProg;
        mBloomExtractProg = nullptr;
    }
    if (mBloomDownsampleProg) {
        delete mBloomDownsampleProg;
        mBloomDownsampleProg = nullptr;
    }
    if (mBloomUpsampleProg) {
        delete mBloomUpsampleProg;
        mBloomUpsampleProg = nullptr;
    }
    if (mBloomCombineProg) {
        delete mBloomCombineProg;
        mBloomCombineProg = nullptr;
    }
    
    if (mScreenQuadVAO) {
        glDeleteVertexArrays(1, &mScreenQuadVAO);
        mScreenQuadVAO = 0;
    }
    if (mScreenQuadVBO) {
        glDeleteBuffers(1, &mScreenQuadVBO);
        mScreenQuadVBO = 0;
    }
}

void ComputeParticles::renderBloom()
{
    if (!mBloomExtractProg || !mBloomDownsampleProg || !mBloomUpsampleProg || !mBloomCombineProg) {
        return;
    }
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    glBindFramebuffer(GL_FRAMEBUFFER, mBloomFBO[0]);
    glViewport(0, 0, mWidth, mHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    
    mBloomExtractProg->enable();
    glUniform1i(mBloomExtractProg->getUniformLocation("sceneTexture"), 0);
    glUniform1f(mBloomExtractProg->getUniformLocation("threshold"), 1.2f);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mSceneTexture);
    
    glBindVertexArray(mScreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    mBloomExtractProg->disable();
    
    for (int i = 0; i < 3; i++) {
        int width = mWidth >> (i + 1);
        int height = mHeight >> (i + 1);
        
        glBindFramebuffer(GL_FRAMEBUFFER, mBloomFBO[i + 1]);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        mBloomDownsampleProg->enable();
        glUniform1i(mBloomDownsampleProg->getUniformLocation("inputTexture"), 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mBloomTexture[i]);
        
        glBindVertexArray(mScreenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        mBloomDownsampleProg->disable();
    }

    for (int i = 0; i < 3; i++) {
        int level = 2 - i;  // Levels: 2, 1, 0
        int width, height;
        if (level == 0) {
            width = mWidth;
            height = mHeight;
        } else {
            width = mWidth >> (level + 1);
            height = mHeight >> (level + 1);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, mBloomUpsampleFBO[i]);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        mBloomUpsampleProg->enable();
        glUniform1i(mBloomUpsampleProg->getUniformLocation("lowResTexture"), 0);
        glUniform1i(mBloomUpsampleProg->getUniformLocation("highResTexture"), 1);
        glm::vec2 texelSize(1.0f / width, 1.0f / height);
        glUniform2fv(mBloomUpsampleProg->getUniformLocation("texelSize"), 1, &texelSize[0]);
        
        glActiveTexture(GL_TEXTURE0);
        if (i == 0) {
            // First upsampling: start from lowest resolution
            glBindTexture(GL_TEXTURE_2D, mBloomTexture[3]);
        } else {
            // Subsequent upsampling
            glBindTexture(GL_TEXTURE_2D, mBloomUpsampleTexture[i - 1]);
        }
        
        glActiveTexture(GL_TEXTURE1);
        // High-res detail from bloom level
        glBindTexture(GL_TEXTURE_2D, mBloomTexture[level]);
        
        glBindVertexArray(mScreenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        mBloomUpsampleProg->disable();
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mWidth, mHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mBloomCombineProg->enable();
    glUniform1i(mBloomCombineProg->getUniformLocation("sceneTexture"), 0);
    glUniform1i(mBloomCombineProg->getUniformLocation("bloomTexture"), 1);
    glUniform1f(mBloomCombineProg->getUniformLocation("bloomIntensity"), 0.3f);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mSceneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mBloomUpsampleTexture[2]);
    
    glBindVertexArray(mScreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    mBloomCombineProg->disable();
    
    CHECK_GL_ERROR();
}
