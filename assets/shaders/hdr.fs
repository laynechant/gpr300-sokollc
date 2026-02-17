#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D hdrBuffer; 


void main()
{
  const float gamma = 2.2;
  vec3 hdrColor = texture(hdrBuffer, vs_texcoord).rgb;

  // tone mapping
  vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
  // gamma correction
  mapped = pow(mapped, vec3(1.0/gamma));
  FragColor = vec4(mapped, 1.0);
}