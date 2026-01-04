#version 430

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;

void main() {
    vec2 texelSize = 1.0 / textureSize(inputTexture, 0);
    
    // Sample 4 corners and center
    vec3 a = texture(inputTexture, TexCoord + vec2(-texelSize.x, -texelSize.y)).rgb;
    vec3 b = texture(inputTexture, TexCoord + vec2(texelSize.x, -texelSize.y)).rgb;
    vec3 c = texture(inputTexture, TexCoord + vec2(-texelSize.x, texelSize.y)).rgb;
    vec3 d = texture(inputTexture, TexCoord + vec2(texelSize.x, texelSize.y)).rgb;
    vec3 e = texture(inputTexture, TexCoord).rgb;
    
    // Downsample with weighted average (center has more weight)
    vec3 color = (a + b + c + d) * 0.125 + e * 0.5;
    
    // Tone mapping to prevent excessive brightness accumulation
    // Simple reinhard tone mapping: color / (1.0 + color)
    color = color / (1.0 + color * 0.5);
    
    FragColor = vec4(color, 1.0);
}

