#version 410


out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform sampler2D screen; 
uniform float strength = 16.0f;


const float offset = 1.0 / 300.0;
const vec2 offsets[9] = vec2[](
  vec2(-offset, offset), // top left
  vec2(0.0, offset), // top middle
  vec2(offset, offset), // top right

  vec2(-offset, 0.0), // center left
  vec2(0.0, 0.0), // center middle
  vec2(offset, 0.0), // center right

  vec2(-offset, -offset), // bottom left
  vec2(0.0, -offset), // bottom middle
  vec2(offset, -offset) // bottom right
);

 const float kernel[9] = float[]( 
 -1.0f, -1.0f, -1.0f,
 -1.0f,  9.0f, -1.0f,
 -1.0f, -1.0f, -1.0f
  );


void main()
{
 
  //vec3 color = texture(screen, vs_texcoord).rgb; 
  vec3 color = vec3(0.0,0.0,0.0); 
  for(int i = 0; i < 9; i++)
  {
    vec3 local = vec3(texture(screen, vs_texcoord.xy + offsets[i]));
    color += local * kernel[i];
  }

  //float gray = (color.r + color.g + color.b) / 3.0; 
  
  FragColor = vec4(color, 1.0);
}