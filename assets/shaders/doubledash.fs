#version 410

out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform vec3 camera;

uniform sampler2D _MainTex; 
uniform sampler2D wave_tex;
uniform sampler2D wave_spec;
uniform sampler2D wave_warp;

uniform float time;
uniform float scale;
uniform vec3 water_color;

void main()
{
  // offset texcoords in some direction
  vec2 dir = vec2(1.0, 0.0);
  vec2 uv = vs_texcoord + vec2(time * dir);
  uv.x += 0.01 * sin(uv.x * 3.5 + time);
  uv.y +=  -.35 *sin(uv.y * 3.5 + time);
  vec3 texture_color = texture(wave_tex, uv).rgb; 

  vec4 sample1 = texture(wave_tex, uv * 1.0);
  vec4 sample2 = texture(wave_tex, uv * 1.2);

  // warp
  vec2 warp_uv = vs_texcoord * scale;
  vec2 warp_scroll = vec2 (0.5 , 0.5) * time;
  vec2 warp = texture(wave_tex, warp_uv + warp_scroll).xy;

  vec2 albedo_uv = vs_texcoord * scale;
  vec4 albedo = texture(wave_warp, warp_uv + warp_scroll).rgba;

  vec3 finalColor = water_color + vec3(albedo.a);

  //specular 
  vec2 spec_uv = vs_texcoord * 1.0;
  vec2 spec_scroll = vec2 (0.5 , 0.5) * time;
  //vec3 spec = texture(wave_spec, spec_uv + warp_scroll).rgb;

  vec3 specSample1 = texture(wave_spec, spec_uv + vec2(0.5, 0.5)).rgb;
  vec3 specSample2 = texture(wave_spec, spec_uv + vec2(-0.5, -0.5)).rgb;
  vec3 spec = specSample1 + specSample2;

  // fresnal
  float fresnal = dot(normalize(camera), vec3(0.0, 1.0 , 0.0));

  //vec3 fresnal_debug = mix(water_color + albedo + spec, vec3(1.0, 0.0, 0.0), fresnal);
  const vec3 bright = vec3(0.299, 0.587, 0.114); 
  float brightness = dot(spec, bright);

  if (brightness <= 0.5 || brightness > 95.0)
  {
    finalColor = mix(finalColor, finalColor + spec, fresnal);
  }

  vec3 object_color = vec3((sample1 * 0.75) -( sample2 * 0.25));
  FragColor = vec4( object_color, 1.0);
}