#include "font_loader.h"

#include <resource_loader/font_loader.h>
#include <graphics/glm_helper.h>
#include <graphics/logger.h>

namespace Resource {
  
  GLFontLoader::GLFontLoader(Resource::ResourcePool pool) {
      this->pool = pool;
  }
  
  GLFontLoader::~GLFontLoader() {
      UnloadStaged();
      UnloadGpu();
  }

  void GLFontLoader::loadToGPU() {
      UnloadGpu();
      inGpu = staged;
      staged.clear();
  }

  void fontsUnload(std::vector<FontData*> &fonts) {
      for(int i = 0; i < fonts.size(); i++)
	  delete fonts[i];
      fonts.clear();
  }

  void GLFontLoader::UnloadStaged() {
      fontsUnload(staged);
  }

  void GLFontLoader::UnloadGpu() {
      fontsUnload(inGpu);
  }

  Font GLFontLoader::LoadFont(std::string file, TextureLoaderGL *texLoader) {
      LOG("loading font: " << file << " ID: " << staged.size());
      FontData* d = loadFont(file, FONT_LOAD_SIZE);
      Resource::Texture t = texLoader->LoadTexture(d->textureData,
						   d->width,
						   d->height,
						   d->nrChannels);
      for(auto &c: d->chars)
	  c.second.tex = t;
      d->textureData = nullptr;
      staged.push_back(d);
      return Font((unsigned int)(staged.size() - 1), pool);
  }
  
  float GLFontLoader::MeasureString(Font font, std::string text, float size) {
      if (font.ID >= inGpu.size()) {
	  LOG_ERROR("font is out of range, in MeasureString, returning 0");
	  return 0.0f;
      }
      return measureString(inGpu[font.ID], text, size);
  }


  std::vector<QuadDraw> GLFontLoader::DrawString(Font drawfont,
                                                 std::string text,
                                                 glm::vec2 position, float size,
                                                 float depth, glm::vec4 colour,
                                                 float rotate) {
    if (drawfont.ID >= inGpu.size()) {
	LOG_ERROR("font is out of range in DrawString, returing no draws!");
	return {};
    }
    return getDraws(inGpu[drawfont.ID], text, size, position, depth, colour, rotate);
}

}
