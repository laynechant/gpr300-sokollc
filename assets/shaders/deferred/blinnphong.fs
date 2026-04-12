#version 410

uniform sampler2D g_position; 
uniform sampler2D g_normal; 
uniform sampler2D g_albedo; 
uniform sampler2D g_material;

struct Material{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light{
  vec3 color;
  vec3 position;
  float radius;
};

uniform vec3 camera;
uniform Light light;
uniform mat4 model;

out vec4 FragLighting;

in vec2 vs_texcoord;

float attenuateLinear(float distance, float radius)
{
  return clamp((radius - distance) / radius, 0.0, 1.0);
}


vec3 blinnphong(vec3 normal, vec3 frag_pos, Material material) {


  vec3 view_dir = normalize(camera - frag_pos);
  vec3 light_dir = normalize(light.position - frag_pos);

  vec3 half_dir = normalize(light_dir + view_dir);

  vec3 ambient = material.ambient * light.color;

  //Calculate diffuse lighting (light diffusion w/ normal)
  vec3 diffuse = material.diffuse * max(dot(normal, light_dir), 0) * light.color;

  //Calculate specular lighting
  float specScalar = pow(max(dot(normal, half_dir), 0), material.shininess);
  vec3 specular = specScalar * material.specular * light.color;

  //Our uncolored lighting model
  vec3 lighting = (ambient + diffuse + specular);

  // calculate the distance
  float distance = length(light.position - frag_pos);
  float attenuation = attenuateLinear(distance, light.radius);

  lighting *= attenuation;

  return lighting;
}


void main()
{
    vec3 position = texture(g_position, vs_texcoord).rgb;
    vec3 normal = normalize(texture(g_normal, vs_texcoord).rgb * 2.0 - 1.0);
    vec3 albedo = texture(g_albedo, vs_texcoord).rgb;
    vec4 matSample = texture(g_material, vs_texcoord);

    Material mat;
    mat.ambient = vec3(matSample.r);
    mat.diffuse = vec3(matSample.g);
    mat.specular = vec3(matSample.b);
    mat.shininess = matSample.a * 128.0;

    FragLighting = vec4(blinnphong(normal, position, mat) * albedo, 1.0);    
}