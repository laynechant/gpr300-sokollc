#version 410

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform float time;
uniform float strength;

float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    float grain = random(vs_texcoord + time) * strength;
    vec3 col = texture(screen, vs_texcoord).rgb;
    FragColor = vec4(col + grain, 1.0);
}