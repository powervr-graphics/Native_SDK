#version 300 es

uniform sampler2D s2DMap;
uniform samplerCube sCubeMap;

uniform bool bCubeReflection;

in mediump vec3  ReflectDir;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	// select whether to use cube map reflection or 2d reflection	
	if(bCubeReflection)
	{
		oColour = texture(sCubeMap, ReflectDir);
	}
	else 
	{
		oColour = texture(s2DMap, ReflectDir.xy * 0.5 + 0.5);
	}
}