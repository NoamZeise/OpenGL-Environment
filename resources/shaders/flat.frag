#version 330 core

in vec2 TexCoords;
out vec4 colour;

uniform sampler2D image;
uniform vec4 spriteColour;
uniform vec4 texOffset;
uniform bool enableTex;

void main()
{
  vec2 coord = TexCoords;
  coord.x *= texOffset.z;
  coord.y *= texOffset.w;
  coord.x += texOffset.x;
  coord.y += texOffset.y;
  if(enableTex)
      colour = texture(image, coord) * spriteColour;
  else
      colour = spriteColour;

  if(colour.w == 0)
    discard;
}
