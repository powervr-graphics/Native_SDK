#version 450

/*********** INPUT DEFINES: THESE MUST BE SELECTED AND DEFINED BY THE PROGRAM ***********
IMAGE_BINDING_INPUT   : Image unit used for the texture that contains the current generation
IMAGE_BINDING_OUTPUT  : Image unit used for the texture that contains the current generation
*********************************** END INPUT DEFINES **********************************/

uniform layout(rgba8, set = 0, binding = 0) readonly highp image2D imageIn;
uniform layout(rgba8, set = 0, binding = 1) writeonly highp image2D imageOut;

layout(local_size_x = 32, local_size_y = 1) in;

void main()
{
	ivec2 image_bounds = ivec2(imageSize(imageIn));

	// We are duplicating the size of the weights just to avoid having to bounds check after every operation.
	// By doing that, we only need to bounds check once per 19 elements
	// These should be hardcoded, if possible, otherwise calculated and uploaded as uniforms.
	lowp const float w[] = float[](
	                         0.016815, 0.023597, 0.031821, 0.041234, 0.051343, 0.061432, 0.07063, 0.078033, 0.082841, 0.084509, 0.082841, 0.078033, 0.07063, 0.061432, 0.051343, 0.041234, 0.031821, 0.023597, 0.016815,
	                         0.016815, 0.023597, 0.031821, 0.041234, 0.051343, 0.061432, 0.07063, 0.078033, 0.082841, 0.084509, 0.082841, 0.078033, 0.07063, 0.061432, 0.051343, 0.041234, 0.031821, 0.023597, 0.016815);

	int fptr = 0; // Counter tracking the last item in our cache
	int x = 0; // Counter (Calculated from fpts) used to find which item in the weights array each colour in the cache corresponds to

	lowp vec3 f[19]; // This is our per-row cache of colours. We use the previous numbers to check what we must load or discard.


	int row = int(gl_GlobalInvocationID.x);

	// PRIME THE CACHE. Load the 10 first pixels.
	f[0] = imageLoad(imageIn, ivec2(0, row)).xyz;  // We load the first item multiple times so that we don't need special code in the loop.
	f[1] = f[0];
	f[2] = f[0];
	f[3] = f[0];
	f[4] = f[0];
	f[5] = f[0];
	f[6] = f[0];
	f[7] = f[0];
	f[8] = f[0];
	f[9] = f[0];
	f[10] = imageLoad(imageIn, ivec2(1, row)).xyz;
	f[11] = imageLoad(imageIn, ivec2(2, row)).xyz;
	f[12] = imageLoad(imageIn, ivec2(3, row)).xyz;
	f[13] = imageLoad(imageIn, ivec2(4, row)).xyz;
	f[14] = imageLoad(imageIn, ivec2(5, row)).xyz;
	f[15] = imageLoad(imageIn, ivec2(6, row)).xyz;
	f[16] = imageLoad(imageIn, ivec2(7, row)).xyz;
	f[17] = imageLoad(imageIn, ivec2(8, row)).xyz;
	f[18] = imageLoad(imageIn, ivec2(9, row)).xyz;

	// Scan the image left-to-right (one invocation must be launched for each row of the image)
	for (int column = 0; column < image_bounds.x; ++column)
	{
		// Average the colour. X tracks the which item in the weight array corresponds to each item in the colour array
		// IMPORTANT NOTE: We do not need bounds checking or expensive modulo operations here, because we have made sure
		// the weight array is doubled.
		vec3 col = f[0] * w[x];
		col += f[1] * w[1 + x];
		col += f[2] * w[2 + x];
		col += f[3] * w[3 + x];
		col += f[4] * w[4 + x];
		col += f[5] * w[5 + x];
		col += f[6] * w[6 + x];
		col += f[7] * w[7 + x];
		col += f[8] * w[8 + x];
		col += f[9] * w[9 + x];
		col += f[10] * w[10 + x];
		col += f[11] * w[11 + x];
		col += f[12] * w[12 + x];
		col += f[13] * w[13 + x];
		col += f[14] * w[14 + x];
		col += f[15] * w[15 + x];
		col += f[16] * w[16 + x];
		col += f[17] * w[17 + x];
		col += f[18] * w[18 + x];

		imageStore(imageOut, ivec2(column, row), vec4(col, 1)); // STORE INTO THE OUTPUT IMAGE.

		int column_to_sample = column + 10; // Select the next texel to load.
		if (column_to_sample >= image_bounds.x) { column_to_sample = image_bounds.x - 1; } //Bounds check to avoid black borders.
		f[fptr++] = imageLoad(imageIn, ivec2(column_to_sample, row)).xyz; //Load the next texel replacing the oldest item in the array.
		x = 19 - fptr; //Calculate the X factor (used to match which item in the weight array we must match to each item in the F array)
		if (fptr == 19) { fptr = 0; }

	}
}

