#ifndef UNIFORMS_H
#define UNIFORMS_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>

struct ShaderParams
{
    glm::mat4 ModelView;
    glm::mat4 ModelViewProjection;
    glm::mat4 ProjectionMatrix;

    glm::vec4 attractor;

    unsigned int numParticles;
    float spriteSize;
    float damping;
    float particleScale;

    float noiseFreq;
    float noiseStrength;
    
    float particleState;
    float stateTime;  
    float heartScale;

    ShaderParams() :
        spriteSize(0.015f),
        attractor(0.0f, 0.0f, 0.0f, 0.0f),
        damping(0.95f),
        particleScale(1.0f),
        noiseFreq(10.0f),
        noiseStrength(0.001f),
        particleState(0.0f),
        stateTime(0.0f),
        heartScale(0.3f)
        {}
};

#define WORK_GROUP_SIZE 128

#endif // UNIFORMS_H
