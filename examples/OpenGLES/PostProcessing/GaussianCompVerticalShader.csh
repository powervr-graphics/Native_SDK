#version 310 es

/*********** INPUT defines: THESE MUST BE SELECTED AND DEFINED BY THE PROGRAM ***********
IMAGE_BINDING_INPUT   : Image unit used for the texture that contains the current generation
IMAGE_BINDING_OUTPUT  : Image unit used for the texture that contains the current generation
*********************************** END INPUT defines **********************************/

uniform layout(rgba16f, binding = 0) readonly mediump image2D imageIn;
uniform layout(rgba16f, binding = 1) writeonly mediump image2D imageOut;

const int MaxGaussianKernel = 51;
const int MaxWeightArraySize = MaxGaussianKernel * 2;

layout(std430, binding = 2) readonly buffer SsboBlurConfig
{
	mediump vec4 blurConfig;
	mediump float gWeights[MaxWeightArraySize];
};

layout(local_size_x = 32, local_size_y = 1) in;

void main()
{
	ivec2 image_bounds = ivec2(imageSize(imageIn));

	int fptr = 0; // Counter tracking the last item in our cache
	int x = 0; // Counter (Calculated from fpts) used to find which item in the weights array each color in the cache corresponds to

	mediump float f[MaxGaussianKernel]; // This is our per-row cache of colors. We use the previous numbers to check what we must load or discard.

	int col = int(gl_GlobalInvocationID.x);

	int halfKernelIndex = int((blurConfig.z - 1.0) * 0.5);

	int offset = 0;
	for (int i = halfKernelIndex; i < int(blurConfig.z); i++)
	{
		f[i] = imageLoad(imageIn, ivec2(col, offset)).r;
		offset++;
	}

	for (int i = 0; i < halfKernelIndex; i++)
	{
		f[i] = f[halfKernelIndex];
	}

	mediump float row = 0.0;

	// Scan the image top-to-bottom (one invocation must be launched for each column of the image)
	for (int rowIndex = 0; rowIndex < image_bounds.y; ++rowIndex)
	{
		// Average the color. X tracks the which item in the weight array corresponds to each item in the color array
		// IMPORTANT NOTE: We do not need bounds checking or expensive modulo operations here, because we have made sure
		// the weight array is doubled.
		row = 0.0;

		for (int i = 0; i < int(blurConfig.z); i++)
		{
			row += f[i] * gWeights[i + x];
		}

		imageStore(imageOut, ivec2(col, rowIndex), vec4(row, 0.0, 0.0, 0.0)); // STORE INTO THE OUTPUT IMAGE.

		int row_to_sample = rowIndex + halfKernelIndex + 1; // Select the next texel to load.
		row_to_sample = min(row_to_sample, image_bounds.y - 1);
		f[fptr++] = imageLoad(imageIn, ivec2(col, row_to_sample)).r; //Load the next texel replacing the oldest item in the array.
		x = int(blurConfig.z) - fptr; //Calculate the X factor (used to match which item in the weight array we must match to each item in the F array)
		if (fptr == int(blurConfig.z)) { fptr = 0; }
	}
}