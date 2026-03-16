#version 410 

out vec4 FragColor;


// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;
// fragPosLghtSpace
in vec4 vs_light_proj_pos;


struct Light{
  vec3 color;
  vec3 position;
};

// might be able to remove this stuff 
struct Pallete{
  vec3 color1;
  vec3 color2;
};

struct Material{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};


uniform sampler2D _MainTex; 
uniform sampler2D zatoon;
uniform sampler2D shadowMap;
uniform float minBias; 
uniform float maxBias; 


uniform vec3 camera;
uniform Light light;


uniform Material material;
uniform float alpha;

uniform Pallete pal;



float shadowCalculation(vec4 fragPosLightSpace)
{
  float shadow = 0.0f;

  vec3 proj_coords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  proj_coords = proj_coords * 0.5 + 0.5;


  float closest = texture(shadowMap, proj_coords.xy).r;

  float current = proj_coords.z;
  vec3 light_dir = normalize(light.position);
  vec3 normal = normalize(vs_normal);

  float slope = clamp(1.0 - dot(normal, light_dir), 0.0f, 1.0f);
  float slopeBias = mix(minBias, maxBias, slope);

  shadow = (current - slopeBias) > closest ? 1.0 : 0.0;

  return shadow;
}

vec3 toon(vec3 normal, vec3 frag_pos, Light light) {

  vec3 view_dir = normalize(camera - frag_pos);
  vec3 light_dir = normalize(light.position - frag_pos);
  vec3 reflect_dir = reflect(light_dir, normal);
  vec3 half_dir = normalize(light_dir + view_dir);

  //Calculate diffuse lighting (light diffusion w/ normal)
  float diffuse = max(dot(normal, light_dir), 0);

  //Calculate specular lighting
  float specular = max(dot(normal, half_dir), 0);
  specular = pow(specular, alpha * material.shininess);

  float NdotL = (dot(normal, light_dir) * 1.0) * 0.5;

  vec3 gradient = texture(zatoon, vec2(NdotL, NdotL)).rgb;

  vec3 out_color = mix(pal.color2, pal.color1, gradient);
  //Our uncolored lighting model
  vec3 lighting = diffuse * material.diffuse + specular * material.specular + material.ambient;

  //return out_color;
 return lighting * light.color;
}

void main()
{
  vec3 lighting = toon(vs_normal, vs_position, light);

  vec3 final_color = lighting + material.ambient;

  float shadow = shadowCalculation(vs_light_proj_pos);

  final_color *= (1.0 - shadow);

  FragColor = vec4(final_color, 1.0);

}