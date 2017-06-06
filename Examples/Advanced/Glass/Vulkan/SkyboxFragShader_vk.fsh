#version 450

layout(set = 0, binding = 0) uniform samplerCube sSkybox;
layout(location = 0) in mediump vec3 RayDir;
layout(location = 0) out vec4 outColor;

void main()
{
	// Sample skybox cube map
	outColor = texture(sSkybox, RayDir);
}
