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
    float bias = 0.2f;
    glm::vec3  ambient = {1.0f, 1.0f, 1.0f};
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
    std::filesystem::current_path("C:/Users/layne/Desktop/Graphics/gpr300-sokollc");

    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");

    blinnphong = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");

        light = {
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };


    shadowMap = std::make_unique<ew::Shader>("assets/shaders/default_shadowmap.vs", "assets/shaders/shadowmap.fs");
    
    depth = std::make_unique<ew::Shader>("assets/shaders/depth.vs", "assets/shaders/depth.fs");


    blurEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/blur.fs");
    hdrEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/hdr.fs");
    sharpenEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/sharpen.fs");
    edgeDetEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/edge.fs");
    greyScaleEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/greyscale.fs");
    vignetteEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/vignette.fs");
    lensDistEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/lensdistortion.fs");
    filmGrainEffect = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/filmgrain.fs");

    brickTexture = std::make_unique<ew::Texture>("assets/textures/bricks.jpg");

    lightColor = light.color;


    pallete = {
        .color1 = { 1.0f, 0.0f, 1.0f},
        .color2 = {0.0f, 1.0f, 1.0f}

    };

    isBlurEnabled = false;
    isHdrEnabled = false;
    isvignetteEnabled = false; 
    isLensDistEnabled = false; 
    isFilmGrainEnabled = false; 
    isSharpenEnabled = false; 
    isEdgeEnabled = false; 
    isGreyScaleEnabled = false;

    filmGrainStrength = 0.5f;
    lensDistStrength = 1.5f;
    blurStrength = 16.0f;

    fullscreen_quad.Initalize();    
   
    
    CreateDepthBuffer();
    CreateFrameBuffer();

    plane.load(ew::createPlane(100.0f, 100.0f, 10));
}

Scene::~Scene()
{
}

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


        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth, 0); 

        glDrawBuffers(0, nullptr);
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
   
    //glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

     // render scene from light (world must exist)
     glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
     {

        const auto light_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
        const auto light_view = glm::lookAt(light.position, glm::vec3(0.0), glm::vec3(0.0f, -1.0f, 0.0f));
        const auto light_view_proj = light_proj * light_view;


        // create the render details
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, 800, 600);

        glClear(GL_DEPTH_BUFFER_BIT);


        depth->use();

    
        depth->setMat4("model", matrix);
        depth->setMat4("light_view_proj", light_view_proj);

        suzanne->draw();

        // scene matrices
        
    }
  
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

   

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


    const auto view_proj = camera.Projection() * camera.View();
   
    shadowMap->use();

    shadowMap->setInt("zatoon", 1);
    shadowMap->setInt("shadowMap", 1);
    shadowMap->setMat4("model", matrix);
    shadowMap->setMat4("view_proj", view_proj);
    shadowMap->setVec3("camera", camera.position);
    shadowMap->setVec3("light.position", light.position);
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


    // scene matrices


    blinnphong->use();
    blinnphong->setInt("_MainTex", 0);

    // scene matrices
    blinnphong->setInt("zatoon", 1);
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
    blinnphong->setVec3("pallete.color1", pallete.color1);
    blinnphong->setVec3("pallete.color1", pallete.color2);

    blinnphong->setFloat("material.shininess", 32.0f);
    blinnphong->setFloat("alpha", debug.alpha);

    
    // draw suzanne
    suzanne->draw();

    const auto plane_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0));
    //blinnphong->setMat4("model", plane_matrix);
    plane.draw();

    // for the full screen quad
 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        if(isBlurEnabled)
        { 

            blurEffect->use();
            blurEffect->setInt("screen", 0);
            blurEffect->setFloat("strength", blurStrength);
        }

        else if (isHdrEnabled)
        {
             hdrEffect->use();
             hdrEffect->setInt("hdrBuffer", 0);
        }

        else if (isSharpenEnabled)
        {
            sharpenEffect->use();
            sharpenEffect->setInt("screen", 0);
        }

        else if (isEdgeEnabled)
        {
            edgeDetEffect->use();
            edgeDetEffect->setInt("screen", 0);
        }

        else if (isGreyScaleEnabled)
        {
            greyScaleEffect->use();
            greyScaleEffect->setInt("screen", 0);
        }

        else if(isvignetteEnabled)
        {
            vignetteEffect->use();
            vignetteEffect->setInt("screen", 0);
        }

        else if (isLensDistEnabled)
        {
            lensDistEffect->use();
            lensDistEffect->setInt("screen", 0);
            lensDistEffect->setFloat("strength", lensDistStrength);
        }
    
        else if (isFilmGrainEnabled)
        {
            filmGrainEffect->use();
            filmGrainEffect->setInt("screen", 0);
            filmGrainEffect->setFloat("time", time.frame);
            filmGrainEffect->setFloat("strength", filmGrainStrength);
        }
  

        glDisable(GL_DEPTH_TEST);
    
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glBindVertexArray(fullscreen_quad.vao);

      
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
 
     }


    
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    ImGui::Checkbox("Blur Effect", &isBlurEnabled);
    ImGui::Checkbox("Hdr Effect", &isHdrEnabled);
    ImGui::Checkbox("Vignette Effect", &isvignetteEnabled);
    ImGui::Checkbox("Lens Distorion Effect", &isLensDistEnabled);
    ImGui::Checkbox("Film Grain Effect", &isFilmGrainEnabled);

    ImGui::Checkbox("Grey Scale Effect", &isGreyScaleEnabled);
    ImGui::Checkbox("Edge Detection Effect", &isEdgeEnabled);
    ImGui::Checkbox("Sharpen Effect", &isSharpenEnabled);

    ImGui::SliderFloat("Blur Strength", &blurStrength, 0, 18);
    ImGui::SliderFloat("Film Grain Strength", &filmGrainStrength, 0, 3);
    ImGui::SliderFloat("Lens Distortion Strength", &lensDistStrength, 0, 2);

    
    ImGui::Image(
        (void*)(intptr_t)frameBuffer,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image(

        (void*)(intptr_t)shadowDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image(
        
        (void*)(intptr_t)fboDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}