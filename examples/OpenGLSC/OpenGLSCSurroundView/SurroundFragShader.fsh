#version 310 es

uniform int uCameraID0;
uniform int uCameraID1;
uniform highp sampler2D sCamera0;
uniform highp sampler2D sCamera1;

#define MAX_CAMERAS 4
layout(std140, binding = 0) uniform GlobalUBO
{
	highp mat4 ViewMatrix[MAX_CAMERAS];
	highp vec3 K[MAX_CAMERAS];
	highp vec2 P[MAX_CAMERAS];
	highp vec2 sensorSize[MAX_CAMERAS];
	highp vec2 sensorCentre[MAX_CAMERAS];
	highp vec2 cameraImageResolution;
};

in highp vec3 vPosition;
in highp vec2 vTexture;
in mediump vec2 vCameraWeights;

layout (location = 0) out highp vec4 oColor;

highp vec2 Undistort(highp vec2 pt, highp vec3 k, highp vec2 p)
{
	highp float r2 = dot(pt, pt);
	highp float barrel = (
		1.0 +
		k.x * r2 +
		k.y * r2*r2 + 
		k.z * r2*r2*r2 );
		
	highp float tanx = (2.0*p.x*pt.x*pt.y) + p.y*(r2 + 2.0*pt.x*pt.x);
	highp float tany = (2.0*p.y*pt.x*pt.y) + p.x*(r2 + 2.0*pt.y*pt.y);
		
	pt = pt * barrel + vec2(tanx, tany);
	
	return pt;
}


highp vec2 WorldToCameraUV(int cameraId, highp vec3 PParam)
{
	highp vec4 viewPos = ViewMatrix[cameraId] * vec4(PParam, 1.0);
	if(viewPos.z >= 0.0)
	{
		return vec2(0.0);
	}
	
	highp vec2 c = (viewPos.xy / -viewPos.z);
	
	c = Undistort(c, K[cameraId], P[cameraId]);

	c = c * sensorSize[cameraId].xy + sensorCentre[cameraId].xy;
	c /= cameraImageResolution;
	
	c.y = 1.0 - c.y;
	return c;
}

mediump vec3 SampleCameraAtPosition(int cameraId, highp vec3 P, sampler2D sampler)
{
	highp vec2 c = WorldToCameraUV(cameraId, P);
	return texture(sampler, c).rgb;
}

highp vec3 reinhardTonemapping(highp vec3 color)
{
	highp float gamma = 1.5;
	highp float exposure = 1.5;
	color *= exposure/(1. + color / exposure);
	color = pow(color, vec3(1. / gamma));
	return color;
}

void main()
{
	mediump vec3 v = SampleCameraAtPosition(uCameraID0, vPosition, sCamera0)*vCameraWeights.r;
	v = max(v, SampleCameraAtPosition(uCameraID1, vPosition, sCamera1)*vCameraWeights.g);
	oColor = vec4(reinhardTonemapping(v), 1.0);
}
