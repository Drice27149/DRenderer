#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
{
    glGenTextures(1, &id);
}

void Texture::LoadTexture(string fn, bool vflip)
{
    glBindTexture(GL_TEXTURE_2D, this->id);
    int width, height, nrChannels;
    if(vflip) stbi_set_flip_vertically_on_load(true);  // 竖直反转
    unsigned char *data = stbi_load(fn.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D); 
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }
}
