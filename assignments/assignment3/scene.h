#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"
#include "../libs/bob/framebuffer.h"

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:
    void CreateDepthBuffer();
    void CreateFrameBuffer();
    void CreateInstanceData();
    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> toon;
    std::unique_ptr<ew::Shader> depth;
    std::unique_ptr<ew::Shader> shadowMap;

    
    std::vector<glm::mat4> modelInstances;
    batteries::light_t light;

    struct 
    {
      glm::vec3 color1;
      glm::vec3 color2;
    }pallete;

    std::unique_ptr<ew::Texture>  brickTexture;
    unsigned int frameBuffer;
    unsigned int fboDepth;

    unsigned int hdrFBO;
    unsigned int colorBuffer;

    // depth buffer
    unsigned int shadowFbo;
    unsigned int shadowDepth;

    // instanced buffer
    unsigned int instanced_buffer; 
    
        
    ew::Mesh plane; 
};
