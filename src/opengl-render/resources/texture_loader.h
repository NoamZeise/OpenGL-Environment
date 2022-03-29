#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "resources.h"
#include "stb_image.h"

#include <string>
#include <vector>
#include <iostream>

#include <glad/glad.h>

namespace Resource
{

class TextureLoader
{
public:
	TextureLoader();
	~TextureLoader();
	Texture LoadTexture(std::string path);
	void Bind(Texture tex);
private:

struct LoadedTex
{
	LoadedTex(std::string path);
	~LoadedTex();
	void Bind();
	unsigned int ID;
	int width;
	int height;
};

	std::vector<LoadedTex*> textures;
};


}


#endif