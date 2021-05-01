#include "Global.hpp"
#include "Texture.hpp"

class TextureMgr {
public:
	TextureMgr();
	Texture GetTextureByFileName(string fn);
private:
	std::map<string, Texture> TexturePool;
};