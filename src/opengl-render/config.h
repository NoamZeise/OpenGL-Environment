#ifndef OPENGL_RENDERER_CONFIG_H
#define OPENGL_RENDERER_CONFIG_H


//#define NDEBUG
//#define NO_ASSIMP

namespace settings
{

const bool FIXED_RATIO = true;
const bool USE_TARGET_RESOLUTION = true;
const int TARGET_WIDTH = 1600;
const int TARGET_HEIGHT = 900;

#ifndef NDEBUG
const bool ERROR_ONLY = true;
#endif
}


#endif
