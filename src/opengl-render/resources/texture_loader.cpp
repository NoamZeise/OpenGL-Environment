#include "texture_loader.h"

namespace Resource
{

	TextureLoader::LoadedTex::LoadedTex(std::string path)
	{
#ifndef NDEBUG
		std::cout << "loading texture: " << path << std::endl;
#endif
	//stbi_set_flip_vertically_on_load(true);
	int nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cerr << "failed to load texture at " << path << std::endl;
		return;
	}

	unsigned int format = GL_RGBA;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if(nrChannels == 4)
		format = GL_RGBA;
	else
	{
		stbi_image_free(data);
		std::cerr << "failed to load texture at " << path << " unsupported num of channels!" << std::endl;
		return;
	}
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(data);
	}

	TextureLoader::LoadedTex::~LoadedTex()
	{
		glDeleteTextures(1, &ID);
	}

	void TextureLoader::LoadedTex::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, ID);
	}


	TextureLoader::TextureLoader() {}

	TextureLoader::~TextureLoader() 
	{
		for(unsigned int i = 0; i < textures.size(); i++)
		{
			delete textures[i];
		}
	}

	Texture TextureLoader::LoadTexture(std::string path)
	{
		textures.push_back(new LoadedTex(path));
		return Texture(textures.size() - 1, glm::vec2(textures.back()->width, textures.back()->height), path);
	}

	void TextureLoader::Bind(Texture tex)
	{
		if(tex.ID >= textures.size())
		{
			std::cerr << "texture ID out of range" << std::endl;
			return;
		}
		textures[tex.ID]->Bind();
	}

}