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
    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> blinnphong;
    std::unique_ptr<ew::Shader> depth;
     std::unique_ptr<ew::Shader> shadowMap;

    // Post processing effects
    std::unique_ptr<ew::Shader> blurEffect;
    std::unique_ptr<ew::Shader> hdrEffect;
    std::unique_ptr<ew::Shader> sharpenEffect;
    std::unique_ptr<ew::Shader> edgeDetEffect;
    std::unique_ptr<ew::Shader> greyScaleEffect;
    std::unique_ptr<ew::Shader> vignetteEffect;
    std::unique_ptr<ew::Shader> lensDistEffect;
    std::unique_ptr<ew::Shader> filmGrainEffect;
    
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


    float filmGrainStrength;
    float lensDistStrength; 
    float blurStrength;

    bool isBlurEnabled;
    bool isHdrEnabled;
    bool isvignetteEnabled;
    bool isLensDistEnabled;
    bool isFilmGrainEnabled;
    bool isSharpenEnabled;
    bool isEdgeEnabled;
    bool isGreyScaleEnabled;

        
    ew::Mesh plane; 
};
