#ifndef RESOURCES_H
#define RESOURCES_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <cstring>

#include "stb_image.h"

namespace Resource {

  struct Texture
  {
  	Texture()
  	{
  		path = "";
  		ID = 0;
  		dim = glm::vec2(1, 1);
  	}
  	Texture(unsigned int ID, glm::vec2 dimentions, std::string path)
  	{
  		this->path = path;
  		this->ID = ID;
  		this->dim = dimentions;
  	}
  	std::string path;
  	unsigned int ID = 0;
  	glm::vec2 dim = glm::vec2(0, 0);
  };

}

#endif
