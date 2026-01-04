#version 430

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D lowResTexture;
uniform sampler2D highResTexture;
uniform vec2 texelSize;

// Bilateral filter parameters
const float sigmaSpace = 1.5;
const float sigmaColor = 0.15;

float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main() {
    vec2 uv = TexCoord;
    
    // Sample low-res (upsampled from previous level) and high-res (current level detail)
    vec3 lowResSample = texture(lowResTexture, uv).rgb;
    vec3 highResSample = texture(highResTexture, uv).rgb;
    vec3 centerColor = highResSample;
    
    vec3 result = vec3(0.0);
    float weightSum = 0.0;
    
    // Bilateral filtering on high-res texture to preserve edges
    const int kernelSize = 5;
    const float offset = 1.5;
    
    for (int y = -kernelSize/2; y <= kernelSize/2; y++) {
        for (int x = -kernelSize/2; x <= kernelSize/2; x++) {
            vec2 sampleCoord = uv + vec2(float(x), float(y)) * texelSize * offset;
            vec3 sampleHighRes = texture(highResTexture, sampleCoord).rgb;
            
            // Spatial weight
            float dist = length(vec2(x, y));
            float spatialWeight = gaussian(dist, sigmaSpace);
            
            // Color weight
            float colorDiff = length(sampleHighRes - centerColor);
            float colorWeight = gaussian(colorDiff, sigmaColor);
            
            float weight = spatialWeight * colorWeight;
            result += sampleHighRes * weight;
            weightSum += weight;
        }
    }
    
    if (weightSum > 0.0) {
        result /= weightSum;
    } else {
        result = highResSample;
    }
    
    FragColor = vec4(lowResSample + result, 1.0);
}

