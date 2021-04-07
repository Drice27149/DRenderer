#include "Global.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

class Renderer {
public:
    Renderer();
    void InitRender();
    void InitSSAO();
    void ShadowPass(Shader* shader);
    void GPass(Shader* shader);
    void SSAOPass(Shader* shader);
    void Render(Shader* shader);
    int width, height;
    Scene* scene;
    Camera* camera;
    vec3 eyePos, eyeDir;
    mat4 projection;
private:
    unsigned int ssaoFBO, ssaoTex;
    unsigned int shadowFBO, shadowTex;
    unsigned int gFBO, normalTex, coordTex, colorTex;
    std::vector<glm::vec3> ssaoKernel;
};