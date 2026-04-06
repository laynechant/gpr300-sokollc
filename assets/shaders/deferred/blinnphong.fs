#version 410

uniform sampler2D g_position; 
uniform sampler2D g_normal; 
uniform sampler2D g_albedo; 
uniform sampler2D g_material;


struct Light{
  vec3 color;
  vec3 position;
};

uniform vec3 camera;
uniform Light light;
uniform mat4 model;

out vec4 FragLighting;


vec3 blinnPhong(vec3 position, vec3 normal, vec3 material)
{
    return vec3(0.0, 0.3, 0.7, 1.0);
}

void main()
{

    vec3 lighting = blinnPhong(g_position, g_normal, g_material);
    //FragLighting = vec4(light.color);
    FragLighting = lighting;
}