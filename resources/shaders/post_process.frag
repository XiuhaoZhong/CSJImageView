#version 450

layout(binding = 0) uniform sampler2D inputTexture;
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 uv;

void main() {
    vec4 color = texture(inputTexture, uv);

    // Simple tonemapping (Reinhard)
    vec3 tonemapped = color.rgb / (color.rgb + vec3(1.0));

    // Gamma correction
    tonemapped = pow(tonemapped, vec3(1.0 / 2.2));

    outColor = vec4(tonemapped, 1.0);
}