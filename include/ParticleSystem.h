#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include <iostream>
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include "ShaderBuffer.h"

class ParticleSystem
{
public:
    ParticleSystem(size_t size, const char* shaderPrefix);
    ~ParticleSystem();

    void loadShaders();
    void reset(float size=1.0f);
    void resetToHeartShape(float scale=0.3f);
    void update();

    size_t getSize() { return m_size; }

    ShaderBuffer<glm::vec4> *getPosBuffer() { return m_pos; }
    ShaderBuffer<glm::vec4> *getVelBuffer() { return m_vel; }
    ShaderBuffer<uint32_t> *getIndexBuffer() { return m_indices; }

private:
    GLuint createComputeProgram(const char* src);

    size_t m_size;
    ShaderBuffer<glm::vec4> *m_pos;
    ShaderBuffer<glm::vec4> *m_vel;
    ShaderBuffer<uint32_t> *m_indices;

    GLuint m_updateProg;

    GLuint m_noiseTex;
    int m_noiseSize;
    const char* m_shaderPrefix;
};
#endif // PARTICLE_SYSTEM_H