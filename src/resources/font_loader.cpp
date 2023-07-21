#include "font_loader.h"

#ifndef NO_FREETYPE
#include <resource_loader/font_loader.h>
#include <graphics/glm_helper.h>
#include <logger.h>

namespace Resource
{

  GLFontLoader::~GLFontLoader() {
      UnloadFonts();
  }

  void GLFontLoader::UnloadFonts() {
      for(int i = 0; i < fonts.size(); i++)
	  delete fonts[i];
      fonts.clear();
  }

  Font GLFontLoader::LoadFont(std::string file, GLTextureLoader *texLoader) {
      LOG("loading font: " << file << " ID: " << fonts.size());
      FontData* d = loadFont(file, FONT_LOAD_SIZE);
      Resource::Texture t = texLoader->LoadTexture(d->textureData,
						   d->width,
						   d->height,
						   d->nrChannels);
      for(auto &c: d->chars)
	  c.second.tex = t;
      d->textureData = nullptr;
      fonts.push_back(d);
      return Font((unsigned int)(fonts.size() - 1));
  }
  
  float GLFontLoader::MeasureString(Font font, std::string text, float size) {
      if (font.ID >= fonts.size()) {
	  LOG_ERROR("font is out of range, in MeasureString, returning 0");
	  return 0.0f;
      }
      return measureString(fonts[font.ID], text, size);
  }


  std::vector<QuadDraw> GLFontLoader::DrawString(Font drawfont,
                                                 std::string text,
                                                 glm::vec2 position, float size,
                                                 float depth, glm::vec4 colour,
                                                 float rotate) {
    if (drawfont.ID >= fonts.size()) {
	LOG_ERROR("font is out of range in DrawString, returing no draws!");
	return {};
    }
    return getDraws(fonts[drawfont.ID], text, size, position, depth, colour, rotate);
}

}

#else

#include <stdexcept>

struct FontData{};

namespace Resource {
GLFontLoader::~GLFontLoader() {}

void GLFontLoader::UnloadFonts() {}

Font GLFontLoader::LoadFont(std::string file, GLTextureLoader* texLoader) { throw std::runtime_error("Tried to use Font::LoadFont, but NO_FREETYPE is defined"); }

float GLFontLoader::MeasureString(Font font, std::string text, float size) { throw std::runtime_error("Tried to use Font::MesaureString, but NO_FREETYPE is defined"); }

  
std::vector<QuadDraw> GLFontLoader::DrawString(Font drawfont, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate) { throw std::runtime_error("Tried to use Font::DrawString, but NO_FREETYPE is defined"); }
}
#endif
