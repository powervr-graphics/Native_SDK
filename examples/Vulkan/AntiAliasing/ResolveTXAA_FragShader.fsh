#version 320 es

layout (binding = 0) uniform mediump sampler2D screenTexture;
layout (binding = 1) uniform mediump sampler2D historyTexture;
layout (binding = 2) uniform mediump sampler2D velocityTexture;
		
layout(location = 0) in highp vec2 TexCoords;
layout(location = 0) out mediump vec4 FragColor;

void main()
{
	mediump float screenWidth = float(textureSize(screenTexture, 0).x);
	mediump float screenHeight = float(textureSize(screenTexture, 0).y);

	mediump float unit = 1.0f;
	mediump float widthStepSize  = unit / screenWidth;
	mediump float heightStepSize = unit / screenHeight;	

	highp vec2 velocitySamplingPos = TexCoords;
	highp vec2 velocity = texture(velocityTexture, velocitySamplingPos).xy;
	mediump vec2 previousPixelPos =	TexCoords - velocity;

	// We fetch current and last frame information according to velocity buffer 
	mediump vec3 currentColor = texture(screenTexture, TexCoords).xyz;
	mediump vec3 historyColor = texture(historyTexture, previousPixelPos).xyz;

	// Neighbor values are needed to create a bounding box
	mediump vec3 nearColor0 = texture(screenTexture, vec2(TexCoords) + vec2(widthStepSize, 0.0f)).xyz;
	mediump vec3 nearColor1 = texture(screenTexture, vec2(TexCoords) - vec2(widthStepSize, 0.0f)).xyz;
	mediump vec3 nearColor2 = texture(screenTexture, vec2(TexCoords) + vec2(0.0f, heightStepSize)).xyz;
	mediump vec3 nearColor3 = texture(screenTexture, vec2(TexCoords) - vec2(0.0f, heightStepSize)).xyz;

	mediump vec3 minColor = min(currentColor, min(nearColor0, min(nearColor1, min(nearColor2, nearColor3))));
	mediump vec3 maxColor = max(currentColor, max(nearColor0, max(nearColor1, max(nearColor2, nearColor3))));
	
	historyColor = clamp(historyColor, minColor, maxColor);

	mediump float modFactor = 0.9f;
	mediump vec3 color = mix(currentColor, historyColor, modFactor);
	
	FragColor = vec4(color, 1.0f);
}
