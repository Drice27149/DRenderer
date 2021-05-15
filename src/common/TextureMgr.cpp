#include "TextureMgr.hpp"
#include "GraphicAPI.hpp"

TextureMgr::TextureMgr()
{
	TexturePool.clear();
}

Texture TextureMgr::GetTextureByFileName(string fn)
{
	// if (TexturePool.count(fn))
	// 	return TexturePool[fn];
	// else {
	// 	cout << "Texture name: " << fn << "\n";
	// 	Texture tex;
	// 	GraphicAPI::LoadImageTexture(tex, fn);
	// 	return TexturePool[fn] = tex;
	// }
	return TexturePool[fn];
}

