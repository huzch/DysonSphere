#version 430

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform float threshold;

void main() {
    vec3 color = texture(sceneTexture, TexCoord).rgb;
    
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    float excess = max(0.0, brightness - threshold);
    if (excess > 0.0 && brightness > 0.0) {
        vec3 bloom = color * (excess / brightness);
        FragColor = vec4(bloom, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

