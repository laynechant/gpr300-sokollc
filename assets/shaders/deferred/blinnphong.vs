#version 410


// attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

// uniforms
uniform mat4 view_proj;
uniform mat4 model;


out vec2 vs_texcoord;

void main()
{
  vec4 world_position = model * vec4(in_position, 1.0);
  vec4 clip_position = view_proj * world_position;

  gl_Position = clip_position;

  vec2 ndc = clip_position.xy / clip_position.w; 
  vs_texcoord = ndc * 0.5 + 0.5;
}