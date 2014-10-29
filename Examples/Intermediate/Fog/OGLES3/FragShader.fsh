#version 300 es
uniform sampler2D  sTexture;

uniform lowp vec3  FogColor;

in mediump vec2  TexCoord;
in lowp    vec3  DiffuseLight;
in lowp    vec3  FogIntensity;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	// Get color from the texture and modulate with diffuse lighting
    lowp vec3 texColour  = texture(sTexture, TexCoord).rgb;
    lowp vec3 colour = texColour * DiffuseLight;
	
	// interpolate the fog colour with the texture-diffuse colour using the 
	// fog intensity calculated in the vertex shader
	colour = mix(FogColor, colour, FogIntensity);
	oColour = vec4(colour, 1.0);
}
