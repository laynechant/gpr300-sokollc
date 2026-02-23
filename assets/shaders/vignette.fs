#version 410

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;

float vignette() {
    vec2 quad_center = vec2(0.5, 0.5);
    float dist = length(vs_texcoord - quad_center);
    float intensity = 1.0 - smoothstep(0.3, 0.75, dist);
    return intensity;
}

void main()
{
    vec3 col = texture(screen, vs_texcoord).rgb;
    FragColor = vec4(col * vignette(), 1.0);
}