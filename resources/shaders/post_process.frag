#version 450

layout(binding = 0) uniform sampler2D inputTexture;
layout(binding = 1) uniform EffectUniforms {
    int effectType;
    float intensity;
    float exposure;
    float padding; // std140 requires uniform buffer blocks to be aligned to 16-byte boundaries.
} ubo;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 uv;

void main() {
    vec4 color = texture(inputTexture, uv);

    switch (ubo.effectType) {
    case 0: // None
        outColor = color;
        break;
    case 1: {// Tonemap
        vec3 c = color.rgb * ubo.exposure;
        vec3 tonemapped = c / (c + vec3(1.0));
        tonemapped = pow(tonemapped, vec3(1.0 / 2.2));
        outColor = vec4(tonemapped, 1.0);
        break;
    }
    case 2: { // Grayscale
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        vec3 result = mix(color.rgb, vec3(gray), ubo.intensity);
        outColor = vec4(result, 1.0);
        break;
    }
    case 3: { // Invert
        vec3 inverted = 1.0 - color.rgb;
        vec3 result = mix(color.rgb, inverted, ubo.intensity);
        outColor = vec4(result, 1.0);
        break;
    }
    case 4: { // Sepia
        float gray2 = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        vec3 sepia = vec3(gray2) * vec3(1.2, 1.0, 0.8);
        vec3 result = mix(color.rgb, sepia, ubo.intensity);
        outColor = vec4(result, 1.0);
        break;
    }
    default:
        outColor = color;
        break;
    }
}