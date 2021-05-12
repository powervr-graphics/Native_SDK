#version 460

layout(location = 0) in mediump vec3 RayDir;
layout(location = 0) out mediump vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvVPMatrix;
	highp vec4 vCameraPosition;
};

layout(set = 1, binding = 0) uniform samplerCube skyboxImage;

void main()
{
	outColor = vec4(texture(skyboxImage, RayDir).rgb, 1.0);
}
