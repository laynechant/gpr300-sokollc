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

#include "ew/procGen.h" 

#include <filesystem>


glm::mat4 lightMatrix = glm::mat4(1.0f);
glm::vec3 lightColor = glm::vec3(1.0f);

struct{
    float alpha = 128.0f;
    float bias = 0.008f;
    glm::vec3  ambient = {0.3f, 0.3f, 0.3f};
    //glm::vec3  ambient = {0.1f, 0.1f, 0.1f};
    glm::vec3  diffuse = {0.5f, 0.5f, 0.5f};
    glm::vec3  specular = {0.5f, 0.5f, 0.5f};


} debug;

struct {
    GLuint fbo; 
    GLuint depth; 
} shadowBuffer;

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
    // dont need this on desktop but its needed on my laptop ?!?!
    //std::filesystem::current_path("C:/Users/layne/Desktop/Graphics/gpr300-sokollc");


    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");

    // default rendering shader
    toon = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");

        light = {
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };


    shadowMap = std::make_unique<ew::Shader>("assets/shaders/default_shadowmap.vs", "assets/shaders/shadowmap.fs");

    depth = std::make_unique<ew::Shader>("assets/shaders/depth.vs", "assets/shaders/depth.fs");



    brickTexture = std::make_unique<ew::Texture>("assets/textures/bricks.jpg");


    lightColor = light.color;


    pallete = {
        .color1 = { 1.0f, 0.0f, 1.0f},
        .color2 = {0.0f, 1.0f, 1.0f}

    };


    
    fullscreen_quad.Initalize();    
   
    
    CreateDepthBuffer();
 
    CreateFrameBuffer();
    

    plane.load(ew::createPlane(100.0f, 100.0f, 10));
}

Scene::~Scene()
{
}

// correct
void Scene::CreateFrameBuffer()
{
    glCreateFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    {

        // color buffer
        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        //256
        //224
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0); 

        // depth buffer
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
    }
	    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Scene::CreateDepthBuffer()
{
    glCreateFramebuffers(1, &shadowFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    
    {

        // color buffer
        glGenTextures(1, &shadowDepth);
        glBindTexture(GL_TEXTURE_2D, shadowDepth);


        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth, 0); 

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);       
        
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, shadowDepth, 0);  

        glBindTexture(GL_TEXTURE_2D, 0);
    }


    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
	    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

auto matrix = glm::mat4(1.0f);

void Scene::Render(void)
{
   

    /* 
        Todo: 
        Make Suzzane thats rendered in shadowDepth render to fullscreen quad
        add texture to the plane
        fix shadow for suzzane
    */

    /* 
        Steps for rendering 
        1. render depth to the scene
        2. render scene normally
        

        Shaders should be fine as is
    */

     // render to the depth buffer
     glViewport(0, 0, 800, 600);
     glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
     glClear(GL_DEPTH_BUFFER_BIT);

     const auto light_proj = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
     const auto light_view = glm::lookAt(light.position, glm::vec3(0.0), glm::vec3(0.0f, -1.0f, 0.0f));
     const auto light_view_proj = light_proj * light_view;
     const auto projection = camera.Projection();
     {

      


        depth->use();

    
        depth->setMat4("model", matrix);
        depth->setMat4("light_view_proj", light_view_proj);

        suzanne->draw();

        // create the render details
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
  
    }



    // render scene as we normally do
    {    

        // then render scene as we normally would
        //glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);


        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickTexture->getID());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepth);



        const auto plane_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0));
        const auto view_proj = camera.Projection() * camera.View();

   
        shadowMap->use();

        shadowMap->setInt("zatoon", 0);
        shadowMap->setInt("shadowMap", 1);
        
        shadowMap->setMat4("model", matrix);
        shadowMap->setMat4("projection",  projection);
        shadowMap->setMat4("view_proj", view_proj);
        shadowMap->setMat4("light_view_proj", light_view_proj);
        shadowMap->setVec3("light.position", light.position);
        shadowMap->setVec3("camera", camera.position);


        shadowMap->setVec3("light.color", light.color);
        shadowMap->setVec3("material.ambient", debug.ambient);
        shadowMap->setVec3("material.diffuse", debug.diffuse);
        shadowMap->setVec3("material.specular", debug.specular);
        shadowMap->setFloat("material.shininess", 32.0f);
        shadowMap->setFloat("alpha", debug.alpha);
        shadowMap->setVec3("pallete.color1", pallete.color1);
        shadowMap->setVec3("pallete.color2", pallete.color2);
    
        shadowMap->setFloat("material.shininess", 32.0f);
        shadowMap->setFloat("alpha", debug.alpha);
        shadowMap->setFloat("bias", debug.bias);
      
        suzanne->draw();

        shadowMap->setMat4("model", plane_matrix);
        plane.draw();

    }

    // render the full screen quad
 


    // {
        

    //     // disable depth testing
    //     //glDisable(GL_DEPTH_TEST);
    
    //     glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //     glBindVertexArray(fullscreen_quad.vao);

    //     toon->setInt("_MainTex", 0);
    //     toon->setInt("zatoon", 1);

    //     // i dont think i want to draw it here
    //     // i need one suzzane not multiple 
    //     // maybe use the one thats being drawn on framebuffer
    //     // or shadowDepth
    //     //suzanne->draw();
        
    //     // figure out want we want to display on this full screen quad 
    //     // the size of the quad is off too 
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, colorBuffer);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);

      
 
    //  }


    

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



    ImGui::Image(

        (void*)(intptr_t)shadowDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}