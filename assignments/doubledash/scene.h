#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"
#include "ew/mesh.h"

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
    std::unique_ptr<ew::Shader> water;
    
    batteries::light_t light;

    std::unique_ptr<ew::Texture> wave_spec; // 0
    std::unique_ptr<ew::Texture> wave_tex;
    std::unique_ptr<ew::Texture> wave_warp;
    

    
    ew::Mesh plane; 
    

    ew::Texture  brickTexture = ew::Texture("assets/textures/bricks.jpg");
};
