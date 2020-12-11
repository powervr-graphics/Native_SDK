/*!*********************************************************************************************************************
\File			MatrixMultiplication.h
\Title			Header file which provides support for MatrixMultiplication.cpp
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Support and backend variables to run the individual tests in the benchmark
***********************************************************************************************************************/

#pragma once
#include "Matrix.h"

/// <summary>
/// Runs the shaders which have been selected by the list of booleans
/// </summary>
/// <param name="benchmarksToRun">The list of benchmarks to run by their index</param>
/// <param name="validate">If a CPU Matrix multiplication should be performed to verify the correctness of shader calculations</param>
/// <param name="pathToExecutable">Used for reading in the shaders by their absolute path if regular readin fails</param>
void runBenchmarksWithList(bool benchmarksToRun[], bool validate, char* pathToExecutable);

// Used for timing shader execution
pvr::Time timer;

namespace TestVariables {
const uint32_t numberOfTotalTests = 13;
int32_t M = 512;
int32_t N = 1536;
int32_t P = 1024;

Matrix A(0, 0, nullptr);
Matrix B(0, 0, nullptr);
// To hold the validation matrix
Matrix C(0, 0, nullptr);

// variables to hold the properties of the tests being done, like workgroups sizes
// with their default values before the user changes
// androids tyoically have a smaller max workgroup size, specifically the chromebook has 512
#ifdef __ANDROID__
int32_t naive_wg_width = 16;
int32_t naive_wg_height = 16;
int32_t linear_wg_size = 32;
int32_t tile_square_wg_size = 16;
int32_t tile_square_WF = 8;

// for the rectangular tiling
int32_t m_tile_size = 32;
int32_t n_tile_size = 64;
int32_t p_tile_size = 16;

// The precision that matricies are confirmed to in validation
float epsilon = 0.1f;
#else
int32_t naive_wg_width = 32;
int32_t naive_wg_height = 32;
int32_t linear_wg_size = 32;
int32_t tile_square_wg_size = 32;
int32_t tile_square_WF = 8;

// for the rectangular tiling
int32_t m_tile_size = 64;
int32_t n_tile_size = 64;
int32_t p_tile_size = 16;

// The precision that matricies are confirmed to in validation
float epsilon = 0.01f;
#endif

std::string Names[numberOfTotalTests] = { "mat_mul_naive_AT", "mat_mul_naive_BT", "mat_mul_naive_CT", "mat_mul_naive_ATCT", "mat_mul_naive_BTCT", "mat_mul_linearwg_AT",
	"mat_mul_linearwg_BT", "mat_mul_linearwg_vec4", "mat_mul_linearwg_vec4_local", "mat_mul_tile", "mat_mul_tile_vec4", "mat_mul_tile_WF", "mat_mul_rect" };

// holds the list of work group sizes which are defined at the pipeline creation stage
int XWorkgroupSize[numberOfTotalTests];
int YWorkgroupSize[numberOfTotalTests];
// holds the number of work groups despatched, for the compute work stage
int XWorkgroupsToLaunch[numberOfTotalTests];
int YWorkgroupsToLaunch[numberOfTotalTests];

// Whether the result of the product is stored inside CT
bool Transposed[numberOfTotalTests] = { false, false, true, true, true, false, false, false, false, false, false, false, false };

void validateUserData()
{
	if (N % 4)
	{
		std::cout << "N must be divisble by 4 for the vec4 examples" << std::endl;
		exit(0);
	}
	if (P % naive_wg_width)
	{
		std::cout << "P must be divisible by the naive work group width" << std::endl;
		exit(0);
	}
	if (M % naive_wg_height)
	{
		std::cout << "M must be divisible by the naive work group height" << std::endl;
		exit(0);
	}
	if (M % linear_wg_size)
	{
		std::cout << "M must be divisble by the linear work group size" << std::endl;
		exit(0);
	}
	if ((M % tile_square_wg_size) + (N % tile_square_wg_size) + (P % tile_square_wg_size))
	{
		std::cout << "M, N and P must all be divisible by the square tile size for the square tile test to work" << std::endl;
		exit(0);
	}
	if (tile_square_wg_size % tile_square_WF)
	{
		std::cout << "The work per thread factor must divide the tile size" << std::endl;
		exit(0);
	}
	if (tile_square_wg_size % 4)
	{
		std::cout << "The tile size must be divisible by 4 for the vec4 examples" << std::endl;
		exit(0);
	}
	if (M % m_tile_size || N % n_tile_size || P % p_tile_size)
	{
		std::cout << "The rectangular tile sizes must divide their respective dimensions" << std::endl;
		exit(0);
	}
}

/// <summary>
/// Since the number of work groups launched are relative to N,M,P this is fired after N,M,P have been set
/// </summary>
void updateWorkgroupsToLaunch()
{
	// XWorkgroup size * Number of Xworkgroups = P (width  of product matrix)
	// YWorkgroup size * Number of Yworkgroups = M (height of product matrix)
	// niave AT
	XWorkgroupSize[0] = naive_wg_width;
	YWorkgroupSize[0] = naive_wg_height;
	XWorkgroupsToLaunch[0] = P / naive_wg_width;
	YWorkgroupsToLaunch[0] = M / naive_wg_height;

	// niave BT
	XWorkgroupSize[1] = naive_wg_width;
	YWorkgroupSize[1] = naive_wg_height;
	XWorkgroupsToLaunch[1] = P / naive_wg_width;
	YWorkgroupsToLaunch[1] = M / naive_wg_height;

	// niave CT
	XWorkgroupSize[2] = naive_wg_width;
	YWorkgroupSize[2] = naive_wg_height;
	XWorkgroupsToLaunch[2] = P / naive_wg_width;
	YWorkgroupsToLaunch[2] = M / naive_wg_height;

	// niave AT CT
	XWorkgroupSize[3] = naive_wg_width;
	YWorkgroupSize[3] = naive_wg_height;
	XWorkgroupsToLaunch[3] = P / naive_wg_width;
	YWorkgroupsToLaunch[3] = M / naive_wg_height;

	// niave BT CT
	XWorkgroupSize[4] = naive_wg_width;
	YWorkgroupSize[4] = naive_wg_height;
	XWorkgroupsToLaunch[4] = P / naive_wg_width;
	YWorkgroupsToLaunch[4] = M / naive_wg_height;

	// linear workgroups
	// Linear workgroup AT
	XWorkgroupSize[5] = 1;
	YWorkgroupSize[5] = linear_wg_size;
	XWorkgroupsToLaunch[5] = P;
	YWorkgroupsToLaunch[5] = M / linear_wg_size;

	// Linear workgroup BT
	XWorkgroupSize[6] = 1;
	YWorkgroupSize[6] = linear_wg_size;
	XWorkgroupsToLaunch[6] = P;
	YWorkgroupsToLaunch[6] = M / linear_wg_size;

	// Linear workgroup vec4 as a result BT
	XWorkgroupSize[7] = 1;
	YWorkgroupSize[7] = linear_wg_size;
	XWorkgroupsToLaunch[7] = P;
	YWorkgroupsToLaunch[7] = M / linear_wg_size;

	// Linear workgroup vec4 as a result BT
	XWorkgroupSize[8] = 1;
	YWorkgroupSize[8] = linear_wg_size;
	XWorkgroupsToLaunch[8] = P;
	YWorkgroupsToLaunch[8] = M / linear_wg_size;

	// Tiled groups
	// square tile
	XWorkgroupSize[9] = tile_square_wg_size;
	YWorkgroupSize[9] = tile_square_wg_size;
	XWorkgroupsToLaunch[9] = P / tile_square_wg_size;
	YWorkgroupsToLaunch[9] = M / tile_square_wg_size;

	// squared tiles except its vec4
	XWorkgroupSize[10] = tile_square_wg_size;
	YWorkgroupSize[10] = tile_square_wg_size;
	XWorkgroupsToLaunch[10] = P / tile_square_wg_size;
	YWorkgroupsToLaunch[10] = M / tile_square_wg_size;

	// we are using the same number of tiles, but per cell in the tile more horizontal work is done
	XWorkgroupSize[11] = tile_square_wg_size / tile_square_WF;
	// XWorkgroupSize[10] = tile_square_wg_size;
	YWorkgroupSize[11] = tile_square_wg_size;
	XWorkgroupsToLaunch[11] = (P / tile_square_wg_size);
	YWorkgroupsToLaunch[11] = M / tile_square_wg_size;

	// This is the rectangular tiles one so extra information needs to be passed into
	Names[12] += " (" + std::to_string(m_tile_size) + "x" + std::to_string(n_tile_size) + "x" + std::to_string(p_tile_size) + ")";
	XWorkgroupSize[12] = p_tile_size;
	YWorkgroupSize[12] = m_tile_size;
	XWorkgroupsToLaunch[12] = P / p_tile_size;
	YWorkgroupsToLaunch[12] = M / m_tile_size;
}

}; // namespace TestVariables
