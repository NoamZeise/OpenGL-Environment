#version 330 core

in vec2 TexCoords;
out vec4 colour;

uniform sampler2D image;
uniform vec4 spriteColour;
uniform bool enableTex;

void main()
{
    if(enableTex)
        colour = texture(image, TexCoords) * spriteColour;
    else
        colour = spriteColour;
}