#version 300 es

uniform sampler2D	basemap;
uniform sampler2D	normalmap;
uniform sampler2D	heightmap;

in lowp vec3 lightDir;
in lowp vec3 viewDir;
in lowp vec2 texCoord;

layout (location = 0) out lowp vec4 oColour;

void main (void)
{
	// Normalise the directions in tangent space
	lowp vec3 vLightDir = normalize(lightDir);
	
	// Initial texture read
	// Calculate how far we're shifting by (using parallax scale).
	lowp float fDepth = texture(heightmap, texCoord).x;
	
	// Set the UV Coord appropriately
	lowp vec2 vTexCoord = texCoord + (fDepth * viewDir.xy);
	
	// Base map Lookup
	lowp vec3 texColour = texture(basemap, vTexCoord).rgb;
	
	// Now do everything else, diffuse, ambient etc.
	lowp vec3 vNormal = (texture(normalmap, vTexCoord).rbg)*2.0-1.0;
		
	// diffuse lighting
	lowp float diffIntensity = max(dot(vLightDir, vNormal), 0.0);	
	
	// calculate actual colour
	lowp vec3 colour = vec3(diffIntensity) * texColour;

	oColour = vec4(colour, 1.0);
}