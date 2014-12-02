uniform samplerCube sCubeMap;

varying mediump vec3 vEyeDir;

void main()
{
	gl_FragColor = textureCube(sCubeMap, vEyeDir);
}