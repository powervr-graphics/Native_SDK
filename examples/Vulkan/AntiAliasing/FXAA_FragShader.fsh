#version 320 es

layout(binding = 0) uniform sampler2D screenTexture;

layout(location = 0) in highp vec2 TexCoords;
layout(location = 0) out mediump vec4 FragColor;

layout(constant_id=0) const mediump float lumaThreshold = 0.5f;
layout(constant_id=1) const mediump float reduceMul = 1.0f / 8.0f;
layout(constant_id=2) const mediump float reduceMin = 1.0f / 128.0f;
layout(constant_id=3) const mediump float spanMax = 8.0f;

void main()
{
	ivec2 textureSize = textureSize(screenTexture, 0);
	mediump float widthStepSize = 1.0f / float(textureSize.x);
	mediump float heightStepSize = 1.0f / float(textureSize.y);	
	mediump vec2 texelStep = vec2(widthStepSize, heightStepSize);

	mediump vec3 colorCenter = texture(screenTexture, TexCoords).rgb;
	mediump vec3 colorUpperLeft = texture(screenTexture, vec2(TexCoords.x - widthStepSize, TexCoords.y + heightStepSize)).rgb;
    mediump vec3 colorUpperRight = texture(screenTexture, vec2(TexCoords.x + widthStepSize, TexCoords.y + heightStepSize)).rgb;
    mediump vec3 colorBottomLeft = texture(screenTexture, vec2(TexCoords.x - widthStepSize, TexCoords.y - heightStepSize)).rgb;
    mediump vec3 colorBottomRight = texture(screenTexture, vec2(TexCoords.x + widthStepSize, TexCoords.y - heightStepSize)).rgb;

	mediump vec3 toLuma = vec3(0.299, 0.587, 0.114);

	mediump float lumaNW = dot(colorUpperLeft, toLuma);
	mediump float lumaNE = dot(colorUpperRight, toLuma);
	mediump float lumaSW = dot(colorBottomLeft, toLuma);
	mediump float lumaSE = dot(colorBottomRight, toLuma);
	mediump float lumaM  = dot(colorCenter, toLuma);

	// Gather minimum and maximum luma.
	mediump float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	mediump float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	if (lumaMax - lumaMin <= lumaMax * lumaThreshold)
	{
		FragColor = vec4(colorCenter, 1.0f);
		return;
	}

	mediump vec2 samplingDirection;
	samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	samplingDirection.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	mediump float samplingDirectionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * reduceMul, reduceMin);
	mediump float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);

	samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-spanMax), vec2(spanMax)) * texelStep;

	mediump vec3 rgbSampleNeg = texture(screenTexture, TexCoords + samplingDirection * (1.0/3.0 - 0.5)).rgb;
	mediump vec3 rgbSamplePos = texture(screenTexture, TexCoords + samplingDirection * (2.0/3.0 - 0.5)).rgb;
	mediump vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5;  

	mediump vec3 rgbSampleNegOuter = texture(screenTexture, TexCoords + samplingDirection * (0.0/3.0 - 0.5)).rgb;
	mediump vec3 rgbSamplePosOuter = texture(screenTexture, TexCoords + samplingDirection * (3.0/3.0 - 0.5)).rgb;
	mediump vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25 + rgbTwoTab * 0.5;   
	
	mediump float lumaFourTab = dot(rgbFourTab, toLuma);

	if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
	{
		FragColor = vec4(rgbTwoTab, 1.0); 
	}
	else
	{
		FragColor = vec4(rgbFourTab, 1.0);
	}

}