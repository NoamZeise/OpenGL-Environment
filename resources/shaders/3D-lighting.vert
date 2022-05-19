#version 410 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec3 outFragPos;
layout (location = 2) out vec3 outNormal;

uniform mat4 model;
uniform mat4 normalMat;
uniform mat4 projection;
uniform mat4 view;

void main()
{
  outTexCoord = inTexCoord;

  vec4 fragPos = view * model * vec4(inPos, 1.0);
  outNormal = mat3(normalMat) * inNormal;

  gl_Position = projection * fragPos;
  outFragPos = vec3(fragPos) / fragPos.w;
}
