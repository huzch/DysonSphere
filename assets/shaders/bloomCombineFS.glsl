#version 430

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float bloomIntensity;

void main() {
    vec3 sceneColor = texture(sceneTexture, TexCoord).rgb;
    vec3 bloom = texture(bloomTexture, TexCoord).rgb;
    
    // Combine original scene with bloom
    vec3 result = sceneColor + bloom * bloomIntensity;
    
    FragColor = vec4(result, 1.0);
}

