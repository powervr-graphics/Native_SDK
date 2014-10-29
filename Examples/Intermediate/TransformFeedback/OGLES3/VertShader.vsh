#version 300 es

#define POSITION_ARRAY		0
#define VELOCITY_ARRAY		1
#define ATTRIBUTES_ARRAY	2

layout (location = POSITION_ARRAY)   in highp   vec3 inPosition;
layout (location = VELOCITY_ARRAY)   in highp   vec3 inVelocity;
layout (location = ATTRIBUTES_ARRAY) in highp   vec3 inAttributes;

uniform highp mat4 ViewProjMatrix;

out highp float varTimeToLive;

void main() 
{ 		
	gl_Position = ViewProjMatrix * vec4(inPosition, 1.0);
	gl_PointSize = gl_Position.z * 2.0;
	varTimeToLive = inAttributes.x;	
} 
