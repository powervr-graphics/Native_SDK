#version 310 es

uniform int uCameraID0;
uniform int uCameraID1;
uniform highp sampler2D sCamera0;
uniform highp sampler2D sCamera1;

in highp vec3 outPosition;
in highp vec2 outTexture;
in mediump vec2 outCameraWeights;
layout (location = 0) out highp vec4 oColor;

#define MAX_CAMERAS 4
layout (std140, binding = 0) uniform CamerasUBO
{
	highp mat4 ViewMatrix[MAX_CAMERAS];
	highp vec3 K[MAX_CAMERAS];
	highp vec2 P[MAX_CAMERAS];
	highp vec2 sensorSize[MAX_CAMERAS];
	highp vec2 sensorCentre[MAX_CAMERAS];
} cameras;

highp vec2 Undistort(highp vec2 pt, highp vec3 k, highp vec2 p)
{
	highp float r2 = dot(pt, pt);
	highp float barrel = (
		1.0 +
		k[0] * r2 +
		k[1] * r2*r2 + 
		k[2] * r2*r2*r2 );
		
	highp float tanx = (2.0*p[0]*pt.x*pt.y) + p[1]*(r2 + 2.0*pt.x*pt.x);
	highp float tany = (2.0*p[1]*pt.x*pt.y) + p[0]*(r2 + 2.0*pt.y*pt.y);
		
	pt = pt * barrel + vec2(tanx, tany);
	
	return pt;
}
highp vec2 WorldToCameraUV(int cameraId, highp vec3 P)
{
	highp vec4 viewPos = cameras.ViewMatrix[cameraId] * vec4(P, 1.0);
	if(viewPos.z>=0.0) return vec2(0.0);
	
	highp vec2 c = (viewPos.xy/-viewPos.z);
	
	c = Undistort(c, cameras.K[cameraId], cameras.P[cameraId]);

	c = c*cameras.sensorSize[cameraId].xy + cameras.sensorCentre[cameraId].xy;
	c /= 1024.0;
	
	c.y = 1.0-c.y;
	return c;
}

mediump vec3 SampleCameraAtPosition(int cameraId, highp vec3 P, sampler2D sampler)
{
	highp vec2 c = WorldToCameraUV(cameraId, P);
	return texture(sampler, c).rgb;
}

void main()
{
	mediump vec3 v = SampleCameraAtPosition(uCameraID0, outPosition, sCamera0)*outCameraWeights.r;
	v = max(v, SampleCameraAtPosition(uCameraID1, outPosition, sCamera1)*outCameraWeights.g);
	oColor.rgb = outPosition;
	oColor.a = 1.0; 
}
