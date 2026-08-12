#version 450

layout(binding = 0) uniform sampler2D yTexture;
layout(binding = 1) uniform sampler2D uTexture;
layout(binding = 2) uniform sampler2D vTexture;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
    // Sample YUV values
    float y = texture(yTexture, uv).r;
    float u = texture(uTexture, uv).r - 0.5;
    float v = texture(vTexture, uv).r - 0.5;

    // Convert YUV to RGB (BT.709 standard)
    vec3 rgb;
    rgb.r = y + 1.5748 * v;
    rgb.g = y - 0.1873 * u - 0.4681 * v;
    rgb.b = y + 1.8556 * u;

    outColor = vec4(rgb, 1.0);
}