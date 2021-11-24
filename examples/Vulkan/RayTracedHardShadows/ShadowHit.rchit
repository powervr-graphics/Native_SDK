#version 460
#extension GL_EXT_ray_tracing : require

struct shadowPayload
{
	uint miss;
};
layout(location = 0) rayPayloadInEXT shadowPayload payload;

void main() { payload.miss = 0u; }
