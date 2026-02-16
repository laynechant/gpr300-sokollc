// batteries
#include "batteries/opengl.h"

#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


#include "ew/texture.h"


glm::mat4 lightMatrix = glm::mat4(1.0f);
glm::vec3 lightColor = glm::vec3(1.0f);

struct{
    float alpha = 128.0f;
    glm::vec3  ambient = {1.0f, 1.0f, 1.0f};
    glm::vec3  diffuse = {0.5f, 0.5f, 0.5f};
    glm::vec3  specular = {0.5f, 0.5f, 0.5f};


} debug;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    blinnphong = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/blinnphong.fs");

        light = {
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };

    lightColor = light.color;

    fboDepth = 0;

    //frameBuffer = bob::createFramebuffer(800, 600, GL_RGB16F, fboDepth);

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

    //glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    // set it back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    blinnphong->use();

    



    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture.getID());
    blinnphong->setInt("_MainTex", 0);

    // scene matrices
    blinnphong->setMat4("model", matrix);
    blinnphong->setMat4("view_proj", view_proj);
    blinnphong->setVec3("camera", camera.position);
    blinnphong->setVec3("light.position", light.position);
    blinnphong->setVec3("light.color", light.color);
    blinnphong->setVec3("material.ambient", debug.ambient);
    blinnphong->setVec3("material.diffuse", debug.diffuse);
    blinnphong->setVec3("material.specular", debug.specular);
    blinnphong->setFloat("material.shininess", 32.0f);
    blinnphong->setFloat("alpha", debug.alpha);

    // draw suzanne
    suzanne->draw();
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
    ImGui::End();
}