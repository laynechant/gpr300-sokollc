#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"

#include "ew/texture.h"

#include "ew/procGen.h" 

glm::mat4 lightMatrix = glm::mat4(1.0f);
glm::vec3 lightColor = glm::vec3(1.0f);

struct{
    float alpha = 128.0f;
    glm::vec3  ambient = {1.0f, 1.0f, 1.0f};
    glm::vec3  diffuse = {0.5f, 0.5f, 0.5f};
    glm::vec3  specular = {0.5f, 0.5f, 0.5f};
    glm::vec3 water_color = {1.0f, 0.0f, 1.0f};


} debug;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    water = std::make_unique<ew::Shader>("assets/shaders/doubledash.vs", "assets/shaders/doubledash.fs");

        light = {
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };

    wave_spec = std::make_unique<ew::Texture>("assets/textures/doubledash/wave_spec.png");
    wave_tex = std::make_unique<ew::Texture>("assets/textures/doubledash/wave_tex.png");
    wave_warp = std::make_unique<ew::Texture>("assets/textures/doubledash/wave_warp.png");
    

    lightColor = light.color;

    plane.load(ew::createPlane(100.0f, 100.0f, 10));
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

auto matrix = glm::mat4(1.0f);

void Scene::Render(void)
{
    
    const auto view_proj = camera.Projection() * camera.View();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    water->use();


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wave_tex->getID());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, wave_warp->getID());

    glActiveTexture(GL_TEXTURE2);
     glBindTexture(GL_TEXTURE_2D, wave_spec->getID());


    water->setInt("wave_tex", 0);
    water->setInt("wave_warp", 1);
    water->setInt("wave_spec", 2);

    water->setFloat("time", (float) time.absolute);
    water->setFloat("scale", (float) 10.0);

    // scene matrices
    water->setMat4("model", matrix);
    water->setMat4("view_proj", view_proj);
    water->setVec3("camera", camera.position);
    water->setFloat("alpha", debug.alpha);
    water->setVec3("water_color", debug.water_color);
    

    // draw suzanne
    plane.draw();
}

void Scene::Debug(void)
{
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    glm::mat4 m{1.0f};
    auto *view = glm::value_ptr(camera.View());
    auto *proj = glm::value_ptr(camera.Projection());
    
    ImGuizmo::DrawGrid(view, proj, glm::value_ptr(m), 100.0f);

    ImGuizmo::Manipulate(
        view,
        proj,
        ImGuizmo::TRANSLATE,
        ImGuizmo::WORLD,
        glm::value_ptr(lightMatrix)
    );

    if(ImGuizmo::IsUsing())
    {
        light.position = glm::vec3(lightMatrix[3]);
    }

    cameracontroller.Debug();

    ImGui::Begin("Controlls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", &lightColor.x);

    light.color = lightColor;

    /* build debug ui here */

    ImGui::SliderFloat3("Ambient", &debug.ambient[0], 0.01f, 1.0f);
    ImGui::SliderFloat3("Diffuse", &debug.diffuse[0], 0.01f, 1.0f);
    ImGui::SliderFloat3("Specular", &debug.specular[0], 0.01f, 1.0f);
    ImGui::SliderFloat3("WaterColor", &debug.water_color[0], 0.0f, 1.0f);
    ImGui::End();
}