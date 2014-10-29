#version 100

uniform samplerCube sSkybox;

varying mediump vec3 RayDir;

void main()
{
	// Sample skybox cube map
	gl_FragColor = textureCube(sSkybox, RayDir);
}
