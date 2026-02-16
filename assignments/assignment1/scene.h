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
    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> blinnphong;
    
    batteries::light_t light;

    ew::Texture  brickTexture = ew::Texture("assets/textures/bricks.jpg");

    unsigned int fboTexture; 
    unsigned int fboDepth;

    //bob::Framebuffer frameBuffer;
};
