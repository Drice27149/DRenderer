#pragma once

#include "Global.hpp"

class Texture
{
public:
    Texture();
    void LoadTexture(string fn, bool vflip = false);
private:
    unsigned int id;
};

