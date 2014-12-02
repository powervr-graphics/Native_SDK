#version 300 es

#define POSITION_ARRAY		0
#define VELOCITY_ARRAY		1
#define ATTRIBUTES_ARRAY	2

layout (location = POSITION_ARRAY)   in highp  vec3 inPosition;
layout (location = VELOCITY_ARRAY)   in highp  vec3 inVelocity;
layout (location = ATTRIBUTES_ARRAY) in highp  vec3 inAttributes; // x = curTimeToLive, y = Damping, z = initialTimeToLive

uniform highp vec3  EmitDirection;
uniform highp vec3  Force;
uniform highp float TimeDelta;

out highp vec3  oPosition;
out highp vec3  oVelocity;
out highp vec3  oAttributes;

void main() 
{ 	
	gl_Position = vec4(inPosition, 1.0);
	oAttributes.x = inAttributes.x - TimeDelta;
	oAttributes.y = inAttributes.y;
	oAttributes.z = inAttributes.z;
	
	// Spawn at origin if it died
	if (oAttributes.x < 0.0)
	{
		oPosition = vec3(0.0);
		oVelocity = EmitDirection * inAttributes.y;
		oAttributes.x = inAttributes.z;
	}
	else
	{
		// not realistic, but works for demo purposes
		oVelocity = inVelocity + Force * TimeDelta;
		oPosition = inPosition + oVelocity * TimeDelta;
	}
} 
