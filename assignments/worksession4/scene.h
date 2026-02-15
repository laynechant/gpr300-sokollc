#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"
#include "batteries/opengl.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

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
    std::unique_ptr<ew::Shader> toon;
    std::unique_ptr<ew::Shader> postprocess;

    ew::Texture  toonTexture = ew::Texture("assets/textures/ZAtoon.png");
    ew::Texture  brickTexture = ew::Texture("assets/textures/bricks.jpg");
    
    batteries::light_t light;

    struct 
    {
      glm::vec3 color1;
      glm::vec3 color2;
    }pallete;

    GLuint frameBuffer;

    GLuint fboTexture; 
    GLuint fboDepth;
    
};
