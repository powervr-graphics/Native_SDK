#ifndef COMMON_GLSL
#define COMMON_GLSL

#include "random.glsl"

#define RAY_GEN_SHADER_IDX 0
#define CLOSEST_HIT_SHADER_IDX 0
#define MISS_SHADER_IDX 0

struct ShadowRayPayload
{
    float dist;
};

#endif