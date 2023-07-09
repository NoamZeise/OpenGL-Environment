#ifndef GLFONT_LOADER_H
#define GLFONT_LOADER_H

#include <string.h>
#include <map>
#include <vector>

#include <glm/glm.hpp>

#ifndef NO_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

#include "texture_loader.h"

namespace Resource
{

class GLFontLoader
{
public:
  GLFontLoader() {};
 ~GLFontLoader();
 Font LoadFont(std::string file, GLTextureLoader* texLoader);
 std::vector<QuadDraw> DrawString(Font drawfont, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate);
 float MeasureString(Font font, std::string text, float size);

private:
  struct Character
  {
    Character(Resource::Texture texture, glm::vec2 size, glm::vec2 bearing, double advance)
    {
    	this->texture = texture;
    	this->size = size;
    	this->bearing = bearing;
    	this->advance = advance;
    }
    Resource::Texture texture;
    glm::vec2 size;
    glm::vec2 bearing;
    double advance;
  };

  class LoadedFont
  {
  public:
	  LoadedFont(std::string file, GLTextureLoader* texLoader);
	  ~LoadedFont();
	  Character* getChar(char c);
	  float MeasureString(std::string text, float size);
  private:
	  std::map<char, Character*> _chars;
	  bool loadCharacter(GLTextureLoader* textureLoader, FT_Face f, char c);
	  const int SIZE = 100;
  };

	std::vector<LoadedFont*> fonts;
};
} // namespace Resource

#else
namespace Resource {
  class GLFontLoader {
public:
    GLFontLoader() {}
    ~GLFontLoader() {}
    Font LoadFont(std::string file, GLTextureLoader* texLoader);
    std::vector<QuadDraw> DrawString(Font drawfont, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate);
    float MeasureString(Font font, std::string text, float size);
  };


  Font GLFontLoader::LoadFont(std::string file, GLTextureLoader* texLoader) { throw std::runtime_error("Tried to use Font::LoadFont, but NO_FREETYPE is defined"); }


  std::vector<QuadDraw> GLFontLoader::DrawString(Font drawfont, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate) { throw std::runtime_error("Tried to use Font::DrawString, but NO_FREETYPE is defined"); }


  float GLFontLoader::MeasureString(Font font, std::string text, float size) { throw std::runtime_error("Tried to use Font::MesaureString, but NO_FREETYPE is defined"); }
}

#endif
#endif
