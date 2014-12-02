uniform sampler2D  sReflectTex;
uniform sampler2D  sNormalMap;

varying mediump vec3  EyeDirection;

varying mediump vec2  TexCoord;

void main()
{
	// Get the normal direction per pixel from the normal map
	// The tNormal vector is defined in surface local coordinates (tangent space).
	mediump vec3 normal = texture2D(sNormalMap, TexCoord).rgb * 2.0 - 1.0;
	
	// reflect(): For the incident vector I and surface orientation N, returns the reflection direction:
	// I - 2 * dot(N, I) * N, N must already be normalized in order to achieve the desired result.
	mediump vec3 reflectDir = reflect(normal, EyeDirection);
	mediump vec2 reflectCoord = (reflectDir.xy) * 0.5 + 0.5;
	
	// Look-up in the 2D texture using the normal map disturbance
	gl_FragColor = texture2D(sReflectTex, reflectCoord);
}