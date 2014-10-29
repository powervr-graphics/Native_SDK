uniform sampler2D  sTexture;

uniform lowp vec3  FogColor;

varying mediump vec2  TexCoord;
varying lowp    vec3  DiffuseLight;
varying lowp    vec3  FogIntensity;

void main()
{
	// Get color from the texture and modulate with diffuse lighting
    lowp vec3 texColor  = texture2D(sTexture, TexCoord).rgb;
    lowp vec3 color = texColor * DiffuseLight;
	
	// interpolate the fog color with the texture-diffuse color using the 
	// fog intensity calculated in the vertex shader
	color = mix(FogColor, color, FogIntensity);
	gl_FragColor = vec4(color, 1.0);
}
