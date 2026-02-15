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

#include "iostream"

glm::mat4 lightMatrix = glm::mat4(1.0f);
glm::vec3 lightColor = glm::vec3(1.0f);

struct{
    float alpha = 128.0f;
    glm::vec3  ambient = {1.0f, 1.0f, 1.0f};
    glm::vec3  diffuse = {0.5f, 0.5f, 0.5f};
    glm::vec3  specular = {0.5f, 0.5f, 0.5f};


} debug;


struct FullScreenQuad
{
    GLuint vao; 
    GLuint vbo; 

    void Initalize()
    {
        float vertices[] = {
            // pos (x,y), texcoord {u,v}
           // triangle 1
            -1.0f, 1.0f, 0.0f, 1.0f,   
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            // triangle 2
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0 , 2 , GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1 , 2 , GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
    }

} fullscreen_quad;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");

        light = {
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };

    //ew::Texture brickTexture = ew::Texture("assets/textures/bricks.jpg");

    postprocess  = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/blur.fs");
    

    pallete = {
        .color1 = { 1.0f, 0.0f, 1.0f},
        .color2 = {0.0f, 1.0f, 1.0f}

    };

    fullscreen_quad.Initalize();

    // frame buffer setup
    glCreateFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    {
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);  

        glGenTextures(1, &fboDepth);
        glBindTexture(GL_TEXTURE_2D, fboDepth);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fboDepth, 0);  

        glBindTexture(GL_TEXTURE_2D, 0);
    }

  


    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

        //return;
    }
	    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightColor = light.color;
}

Scene::~Scene()
{
    glDeleteFramebuffers(1, &frameBuffer);
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

auto matrix = glm::mat4(1.0f);

void Scene::Render(void)
{
   
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer)
    /*
        Todo: 
        1. Apply material effects onto Sussane
        2. Add Imgui sliders for the effects
         
     */
    const auto view_proj = camera.Projection() * camera.View();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_DEPTH_TEST);

    
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, brickTexture.getID());

    toon->use();

    // scene matrices
    toon->setInt("zatoon", 1);
    toon->setMat4("model", matrix);
    toon->setMat4("view_proj", view_proj);
    toon->setVec3("camera", camera.position);
    toon->setVec3("light.position", light.position);
    toon->setVec3("light.color", light.color);
    toon->setVec3("material.ambient", debug.ambient);
    toon->setVec3("material.diffuse", debug.diffuse);
    toon->setVec3("material.specular", debug.specular);
    toon->setVec3("pallete.color1", pallete.color1);
    toon->setVec3("pallete.color1", pallete.color2);


    toon->setFloat("material.shininess", 32.0f);
    toon->setFloat("alpha", debug.alpha);

    // draw suzanne
    suzanne->draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

   {
    
    postprocess->use();
    postprocess->setInt("screen", 0);
 

    glDisable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(fullscreen_quad.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    }
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

    /* build debug ui here */

    ImGui::SliderFloat3("Ambient", &debug.ambient[0], 0.01f, 1.0f);
    ImGui::SliderFloat3("Diffuse", &debug.diffuse[0], 0.01f, 1.0f);
    ImGui::SliderFloat3("Specular", &debug.specular[0], 0.01f, 1.0f);

    ImGui::ColorEdit3("Color1", &pallete.color1[0], 0.0f);
    ImGui::ColorEdit3("Color2", &pallete.color2[0], 0.0f);
    

     ImGui::Image(
        //(void*)(intptr_t)fboTexture,
        //ImVec2(400, 300),
        //ImVec2(0, 1), ImVec2(1, 0));

        (void*)(intptr_t)fboDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}