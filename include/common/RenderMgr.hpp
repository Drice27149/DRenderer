#pragma once

#include "Global.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class RenderMgr {
public:
    RenderMgr();
    void PreInit();
    void PrePass();
    void BasePass();
    void Render();
public:
    bool bgrid = true;
    bool bskybox = true;
private:
    // ×ø±êÍø¸ñ
    Grid* grid;
    Shader* g_sh;
    // Ìì¿ÕºÐ
    CubeMap* skybox;
    Shader* sk_sh;
    // temp base pass shader
    Shader* base_sh;
};