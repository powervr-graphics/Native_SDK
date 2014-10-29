#version 300 es

in highp vec3 vNormal;
in highp vec3 vLightDirection;
layout (location = 0) out lowp vec4 oColour;

void main()
{
	highp float inv_lightdist = 1.0 / length(vLightDirection);
	highp float diffuse = max(dot(normalize(vNormal), vLightDirection * inv_lightdist), 0.0);
	oColour = vec4(vec3(diffuse) * inv_lightdist * 10.0, 1.0);
}
