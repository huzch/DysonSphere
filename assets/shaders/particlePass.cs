#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

precision highp sampler3D;

layout(std140, binding=1) uniform ShaderParams {
    mat4 ModelView;
    mat4 ModelViewProjection;
    mat4 ProjectionMatrix;

    vec4 attractor;

    uint numParticles;
    float spriteSize;
    float damping;
    float particleScale;

    float noiseFreq;
    float noiseStrength;
    
    // Shape effect parameters
    float particleState; 
    float stateTime;  
    float heartScale; 
};

#define WORK_GROUP_SIZE 128

uniform float invNoiseSize;
uniform sampler3D noiseTex3D;

// SSBO binding points: use 2 and 3 to avoid conflict with UBO at binding=1
layout( std140, binding=2 ) buffer Pos {
    vec4 pos[];
};

layout( std140, binding=3 ) buffer Vel {
    vec4 vel[];
};

layout(local_size_x = WORK_GROUP_SIZE,  local_size_y = 1, local_size_z = 1) in;


vec3 noise3f(vec3 p) {
    return texture(noiseTex3D, p * invNoiseSize).xyz;
}

// fractal sum
vec3 fBm3f(vec3 p, int octaves, float lacunarity, float gain) {
    float freq = 1.0, amp = 0.5;
    vec3 sum = vec3(0.0);
    for(int i=0; i<octaves; i++) {
        sum += noise3f(p*freq)*amp;
        freq *= lacunarity;
        amp *= gain;
    }
    return sum;
}

vec3 attract(vec3 p, vec3 p2) {
    const float softeningSquared = 0.01;
    vec3 v = p2 - p;
    float r2 = dot(v, v);
    r2 += softeningSquared;
    float invDist = 1.0f / sqrt(r2);
    float invDistCubed = invDist*invDist*invDist;
    return v * invDistCubed;
}

vec3 getHeartPosition(uint particleIndex) {
    float t = float(particleIndex) / float(numParticles);
    
    float u = t * 6.28318530718;
    
    float x = 16.0 * sin(u) * sin(u) * sin(u);
    float y = 13.0 * cos(u) - 5.0 * cos(2.0 * u) - 2.0 * cos(3.0 * u) - cos(4.0 * u);
    
    float scale = heartScale / 20.0; 
    x *= scale;
    y *= scale;
    
    float z = sin(u * 2.0) * scale * 0.5;
    
    return vec3(x, y, z);
}

vec3 getStarPosition(uint particleIndex) {
    float t = float(particleIndex) / float(numParticles);
    
    float u = t * 6.28318530718; // 2π
    
    // Five-pointed star using polar coordinates
    // For a pentagram, we alternate between outer radius and inner radius
    // Outer radius at angles: 0, 72°, 144°, 216°, 288° (0, π*2/5, π*4/5, π*6/5, π*8/5)
    // Inner radius at angles: 36°, 108°, 180°, 252°, 324° (π/5, π*3/5, π, π*7/5, π*9/5)
    
    float angle = u;
    float outerRadius = 1.0;
    float innerRadius = 0.382;
    
    float segmentAngle = mod(angle, 1.25663706144); 
    float segmentIndex = floor(angle / 1.25663706144);
    
    float radius;
    if (segmentAngle < 0.62831853072) {
        float localAngle = segmentAngle / 0.62831853072; // normalize to [0, 1]
        radius = mix(outerRadius, innerRadius, localAngle);
    } else {
        float localAngle = (segmentAngle - 0.62831853072) / 0.62831853072; // normalize to [0, 1]
        radius = mix(innerRadius, outerRadius, localAngle);
    }
    
    float scale = heartScale * 0.5; // Scale for star
    float x = cos(angle) * radius * scale;
    float y = sin(angle) * radius * scale;
    
    float z = sin(u * 3.0) * scale * 0.3;
    
    return vec3(x, y, z);
}

void main() {
    uint i = gl_GlobalInvocationID.x;

    if (i >= numParticles) return;

    vec3 p = pos[i].xyz;
    vec3 v = vel[i].xyz;
    
    if (particleState < 0.5) {
        v += fBm3f(p*noiseFreq,4,2.0,0.5)*noiseStrength;
        v += attract(p, attractor.xyz)*attractor.w;
        
        p += v;
        v *= damping;
    } else if (particleState < 1.5) {
        vec3 center = vec3(0.0, 0.0, 0.0);
        vec3 toCenter = center - p;
        float dist = length(toCenter);
        
        float absorbStrength = 5.0;
        if (dist > 0.001) {
            v += normalize(toCenter) * absorbStrength * (1.0 / (dist + 0.01));
        } else {
            v = vec3(0.0);
            p = center;
        }
        
        p += v * 0.5;
        v *= 0.8;
    } else if (particleState < 2.5) {
        vec3 targetPos = getHeartPosition(i);
        
        vec3 toTarget = targetPos - p;
        float dist = length(toTarget);
        
        if (dist > 0.001) {
            float springStrength = 2.0;
            v += normalize(toTarget) * springStrength * dist;
            
            p += v * 0.3;
            v *= 0.85; // Damping
        } else {
            p = targetPos;
            v = vec3(0.0);
        }
    } else {
        vec3 targetPos = getStarPosition(i);
        
        vec3 toTarget = targetPos - p;
        float dist = length(toTarget);
        
        if (dist > 0.001) {
            float springStrength = 2.0;
            v += normalize(toTarget) * springStrength * dist;
            
            p += v * 0.3;
            v *= 0.85; // Damping
        } else {
            p = targetPos;
            v = vec3(0.0);
        }
    }

    pos[i] = vec4(p, 1.0);
    vel[i] = vec4(v, 0.0);
}
