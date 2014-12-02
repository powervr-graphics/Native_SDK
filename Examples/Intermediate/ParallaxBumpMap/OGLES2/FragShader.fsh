uniform sampler2D	basemap;
uniform sampler2D	normalmap;
uniform sampler2D	heightmap;

varying lowp vec3	lightDir;
varying lowp vec3	viewDir;
varying lowp vec2	texCoord;

void main (void)
{
	// Normalise the directions in tangent space
	lowp vec3 vLightDir = normalize(lightDir);
	
	// Initial texture read
	// Calculate how far we're shifting by (using parallax scale).
	lowp float fDepth = texture2D(heightmap, texCoord).x;
	
	// Set the UV Coord appropriately
	lowp vec2 vTexCoord = texCoord + (fDepth * viewDir.xy);
	
	// Base map Lookup
	lowp vec3 texColour = texture2D(basemap, vTexCoord).rgb;
	
	// Now do everything else, diffuse, ambient etc.
	lowp vec3 vNormal = (texture2D(normalmap, vTexCoord).rbg)*2.0-1.0;
		
	// diffuse lighting
	lowp float diffIntensity = max(dot(vLightDir, vNormal), 0.0);	
	
	// calculate actual colour
	lowp vec3 colour = vec3(diffIntensity) * texColour;

	gl_FragColor = vec4(colour, 1.0);
}