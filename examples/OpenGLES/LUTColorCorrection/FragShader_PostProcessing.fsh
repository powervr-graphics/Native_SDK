#version 310 es

layout(location = 0) in mediump vec2 vTexCoords;
layout(location = 0) out mediump vec4 oColor;

mediump uniform sampler2D sSceneTexture;
mediump uniform sampler3D sLUTexture;

void main()
{
	mediump vec3 originalColor = texture(sSceneTexture, vTexCoords).rgb;

	// right half of the screen shows LUT result
    // left half of the screen shows original image
	if(vTexCoords.x > 0.5)
	{
		// use the original RGB value as 3D coordinates
        // to look up the color graded result in the LUT
		mediump vec3 modifColor = texture(sLUTexture, originalColor).rgb;
		oColor = vec4(modifColor, 1.0);
	}
	else
	{
		oColor = vec4(originalColor, 1.0);
	}
}
