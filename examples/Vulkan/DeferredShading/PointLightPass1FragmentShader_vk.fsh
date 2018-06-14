#version 450

layout(set = 0, binding = 0) uniform StaticPerScene
{
	highp float farClipDistance;
};

layout(set = 1, binding = 0) uniform StaticsPerPointLight
{
	highp float vLightIntensity;
	highp vec4 vLightColor;
	highp vec4 vLightSourceColor;
};

layout(location = 0) out lowp vec4 oColorFbo;

void main()
{
	oColorFbo = vec4(0.0);
}
