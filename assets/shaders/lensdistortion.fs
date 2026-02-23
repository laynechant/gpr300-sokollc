#version 410

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform float strength;

vec2 distorition() {
    vec2 centered = vs_texcoord - vec2(0.5);
    float dist = length(centered);
    vec2 effect = centered * (1.0 + strength * dist * dist);
    return effect + vec2(0.5);
}

void main()
{
    vec2 var = distorition();
    vec3 col = texture(screen, var).rgb;
    FragColor = vec4(col, 1.0);
}