#version 430 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in ivec4 inBoneIDs;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec3 outNormal;


const int MAX_3D_INSTANCE = 10000;

layout (std430, binding = 2) buffer perInstanceModelData
{
  mat4 model[MAX_3D_INSTANCE];
};

layout (std430, binding = 3) buffer perInstanceNormalData
{
  mat4 normalMat[MAX_3D_INSTANCE];
};

const int MAX_BONES = 50;
uniform mat4 bones[MAX_BONES];


uniform mat4 view;
uniform mat4 proj;

void main()
{
    outTexCoord = inTexCoord;

    mat4 skin = mat4(0.0f);
    for(int i = 0; i < 4; i++)
    {
      if(inBoneIDs[i] == -1 || inBoneIDs[i] >= MAX_BONES)
          break;
      skin += inWeights[i] * bones[inBoneIDs[i]];
    }

    vec4 fragPos = model[gl_InstanceID] * skin * vec4(inPos, 1.0f);
    outNormal = (pid.data[gl_InstanceID].normalMat * skin * vec4(inNormal, 1.0f)).xyz;

    gl_Position = projection * view * fragPos;
    outFragPos = vec3(fragPos) / fragPos.w;
}
