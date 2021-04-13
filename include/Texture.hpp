#pragma once

#include "Global.hpp"

class Texture
{
public:
    Texture();
public:
    unsigned int id;
};

class CubeMap: public Texture
{
public:
    CubeMap();
};

