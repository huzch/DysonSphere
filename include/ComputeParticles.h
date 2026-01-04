#ifndef COMPUTE_PARTICLES_H
#define COMPUTE_PARTICLES_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderUtils.h" 
#include "noise.h"
#include "uniforms.h"

class ParticleSystem;

enum ParticleState {
    Normal,   
    Absorbing, 
    HeartShape,
    StarShape 
};

class ComputeParticles
{
public:
    ComputeParticles();
    ~ComputeParticles();
    
    bool init(GLFWwindow* window);
    void draw(float deltaTime);
    void reshape(int width, int height);
    void handleKey(int key, int action);
    void handleMouseButton(int button, int action, int mods);
    void handleMouseMove(double xpos, double ypos);
    void handleScroll(double xoffset, double yoffset);
    
    void reset();

private:
    ShaderParams mShaderParams;
    ShaderProgram* mRenderProg;
    
    const static int mNumParticles = 1<<20;
    ParticleSystem* mParticles;
    int32_t mParticleCount;
    GLuint mUBO;
    GLuint mVBO;
    GLuint mVAO;
    
    bool mEnableAttractor;
    bool mAnimate;
    float mTime;
    
    // 形状效果状态
    ParticleState mParticleState;      // 当前粒子状态
    ParticleState mTargetShapeState;   // 吸收后的目标形状
    float mStateTime;                  // 状态改变后的时间
    float mAbsorbDuration;       
    float mHeartDuration;    
    
    int mWidth;
    int mHeight;
    
    glm::mat4 mViewMatrix;
    glm::vec3 mCameraPos;
    glm::vec3 mCameraTarget;
    
    bool mLeftMousePressed;  
    bool mRightMousePressed;  
    double mLastMouseX;    
    double mLastMouseY;     
    float mCameraDistance;   
    float mCameraAzimuth;      
    float mCameraElevation;  
    
    // Bloom后处理效果
    GLuint mSceneFBO;          
    GLuint mSceneTexture;  
    GLuint mBloomFBO[4];      
    GLuint mBloomTexture[4];   
    GLuint mBloomUpsampleFBO[3];   
    GLuint mBloomUpsampleTexture[3];
    
    ShaderProgram* mBloomExtractProg;
    ShaderProgram* mBloomDownsampleProg;
    ShaderProgram* mBloomUpsampleProg;
    ShaderProgram* mBloomCombineProg;
    
    GLuint mScreenQuadVAO;
    GLuint mScreenQuadVBO;
    
    void initBloomResources();
    void destroyBloomResources();
    void renderBloom();
    void createScreenQuad();
};

#endif // COMPUTE_PARTICLES_H
