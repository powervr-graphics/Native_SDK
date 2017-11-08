#version 310 es

in mediump vec2 TexCoord;
uniform sampler2D sOriginalTexture;
uniform sampler2D sTexture;

uniform highp float WindowWidth;

out highp vec4 fragColor;

void main()
{
	highp float imageCoordX = gl_FragCoord.x - 0.5;
	highp float xPosition = imageCoordX / WindowWidth;

	lowp vec3 col = vec3(0.0);

	ivec2 image_bounds = textureSize(sTexture, 0);

	if(xPosition < 0.5)
	{
		col = texture(sOriginalTexture, TexCoord).rgb;
	}
	else if(xPosition == 0.5)
	{
		col = vec3(1.0);
	}
	else
	{
		//Sigma = 5;
		lowp const float w[] = float[](0.016815, 0.023597, 0.031821, 0.041234, 0.051343, 0.061432, 0.07063, 0.078033, 0.082841, 0.084509, 0.082841, 0.078033, 0.07063, 0.061432, 0.051343, 0.041234, 0.031821, 0.023597, 0.016815);

		//combined_weight -- ILLUSTRATION PURPOSES - THESE CALCULATIONS CAN BE DONE CPU-SIDE AND LOADED AS UNIFORMS
		lowp const float c_w[] = float[]
								 (w[0] + w[1], w[2] + w[3], w[4] + w[5], w[6] + w[7], w[8] + w[9] * .5, w[9] * .5 + w[10], w[11] + w[12], w[13] + w[14], w[15] + w[16], w[17] + w[18]);



		mediump float texelHeight = 1. / float(image_bounds.y - 1);

		// offset  -- ILLUSTRATION PURPOSES - THESE CALCULATIONS CAN BE DONE CPU-SIDE AND LOADED AS UNIFORMS
		lowp float o_[] = float[](-texelHeight * 9., -texelHeight * 8., -texelHeight * 7., -texelHeight * 6., -texelHeight * 5., -texelHeight * 4., -texelHeight * 3., -texelHeight * 2., -texelHeight, 0., texelHeight, texelHeight * 2., texelHeight * 3., texelHeight * 4., texelHeight * 5., texelHeight * 6., texelHeight * 7., texelHeight * 8., texelHeight * 9.);
		lowp float o[] = float[](
						   (o_[0] * w[0] + o_[1] * w[1]) / c_w[0],
						   (o_[2] * w[2] + o_[3] * w[3]) / c_w[1],
						   (o_[4] * w[4] + o_[5] * w[5]) / c_w[2],
						   (o_[6] * w[6] + o_[7] * w[7]) / c_w[3],
						   (o_[8] * w[8] + o_[9] * w[9] * .5) / c_w[4],
						   (o_[10] * w[10] + o_[9] * w[9] * .5) / c_w[5],
						   (o_[12] * w[12] + o_[11] * w[11]) / c_w[6],
						   (o_[14] * w[14] + o_[13] * w[13]) / c_w[7],
						   (o_[16] * w[16] + o_[15] * w[15]) / c_w[8],
						   (o_[18] * w[18] + o_[17] * w[17]) / c_w[9]
						 );

		col =
		  texture(sTexture, TexCoord + vec2(0, o[0])).xyz * c_w[0] +
		  texture(sTexture, TexCoord + vec2(0, o[1])).xyz * c_w[1] +
		  texture(sTexture, TexCoord + vec2(0, o[2])).xyz * c_w[2] +
		  texture(sTexture, TexCoord + vec2(0, o[3])).xyz * c_w[3] +
		  texture(sTexture, TexCoord + vec2(0, o[5])).xyz * c_w[4] +
		  texture(sTexture, TexCoord + vec2(0, o[4])).xyz * c_w[5] +
		  texture(sTexture, TexCoord + vec2(0, o[6])).xyz * c_w[6] +
		  texture(sTexture, TexCoord + vec2(0, o[7])).xyz * c_w[7] +
		  texture(sTexture, TexCoord + vec2(0, o[8])).xyz * c_w[8] +
		  texture(sTexture, TexCoord + vec2(0, o[9])).xyz * c_w[9];
	}

	fragColor = vec4(col , 1);
}

