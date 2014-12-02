uniform sampler2D s2DMap;
uniform samplerCube sCubeMap;

uniform bool bCubeReflection;

varying mediump vec3  ReflectDir;

void main()
{
	// select whether to use cube map reflection or 2d reflection	
	if(bCubeReflection)
	{
		gl_FragColor = textureCube(sCubeMap, ReflectDir);
	}
	else 
	{
		gl_FragColor = texture2D(s2DMap, ReflectDir.xy * 0.5 + 0.5);
	}
}