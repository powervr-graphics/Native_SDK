/*!*********************************************************************************************************************
\File			MatrixMultiplication.cpp
\Title			Headless Vulkan Compute Matrix Multiply
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			A command line tool which is a headless/surfaceless (ie without a window) vulkan application which
				calculates the multiplication of some matrices in a compute shader on the gpu with the goal of
				perfoming a SGEMM (Single precision GEneral Matrix Multiply) benchmark
***********************************************************************************************************************/
#include <PVRCore/PVRCore.h>
#include "MatrixMultiplicationGPU.h"
#include "MatrixMultiplication.h"

/// <summary>
/// Prints the supported command line paramaters.
/// </summary>
void printHelp()
{
	std::cout << std::endl << "SGEMM benchmark supported command line options:" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-h"
			  << "Display this mesage." << std::endl;
	std::cout << std::left << std::setw(20) << "\t-i"
			  << "Displays information about what this benchmark does." << std::endl;
	std::cout << std::left << std::setw(20) << "\t-va"
			  << "Produces a niave CPU multiplication and stores the result to check against the GPU results to validate their correctness.";
	std::cout << " Will take an order of magnitude longer than the GPU." << std::endl;
	std::cout << std::left << std::setw(20) << "\t-shaders=[names]";
	std::cout << "Will run the specified shaders by name, shader names are specificed as a list of comma seperated values. If left empty this will run all tests in a demo mode"
			  << std::endl;
	std::cout << std::left << std::setw(27) << " "
			  << "Here is a list of recognised shader names:" << std::endl;

	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_naive_AT";
	std::cout << "Each invocation calulates one cell in the product, no optimisations. Matrix A is sent transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_naive_BT";
	std::cout << "No optimisations. Matric B is sent transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_naive_CT";
	std::cout << "No optimisations. Product matrix is stored transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_naive_ATCT";
	std::cout << "No optimisations Product Matrix is stored transposed and Matrix A is sent transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_naive_BTCT";
	std::cout << "No optimisations Product Matrix is stored transposed and Matrix B is sent transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_linearwg_AT";
	std::cout << "Work groups represent horizontal lines of the product matrix. A is sent transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_linearwg_BT";
	std::cout << "Work groups represent horizontal lines of the product matrix. B is sent transposed" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_linearwg_vec4";
	std::cout << "The matrices are stored as an array of vectors to reduce the numbe of reads. B is transposed." << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_linearwg_vec4_local";
	std::cout << "The matrices are stored as an array of vectors. local memory is used to store entire column of B, will fail for large N" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_tile";
	std::cout << "Workgroups are square tiles in the product matrix, local memory stores required tiles " << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_tile_vec4";
	std::cout << "Workgroups are square tiles in the product matrix, matrices are represented as arrays of vectors" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_tile_WF";
	std::cout << "Work groups are rectanular, but the tiles remain square, this is each invocation calculates WF number of  cells in the product" << std::endl;
	std::cout << std::left << std::setw(27) << " " << std::left << std::setw(30) << "mat_mul_rect";
	std::cout << "A generalisation of the square tiling, each invocation produces one cell in the product" << std::endl;

	std::cout << std::left << std::setw(20) << "\t-M=";
	std::cout << "Sets the height of matrices A and C" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-N=";
	std::cout << "Sets the width of matrix A and the hieght of Matrix B" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-P=";
	std::cout << "Sets the width of matrices B and C" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-epsilon=";
	std::cout << "Sets the percision that matrix validation is performed at" << std::endl;

	std::cout << std::left << std::setw(20) << "\t-naive_wg_width=";
	std::cout << "Sets the work group width for the naive implementations" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-naive_wg_height=";
	std::cout << "Sets the work group height for the naive implementations" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-linear_wg=";
	std::cout << "Sets the size of the segmentations of the linear workgroups" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-tile_square_wg=";
	std::cout << "Sets the square tile width and height, directly corrosponds to workgroup size for some examples" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-tile_square_wf=";
	std::cout << "Sets the work factor, the number of cells per invocation calculated in the product" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-tile_m=";
	std::cout << "Sets the tile size in the M dimension for the rectangular example" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-tile_n=";
	std::cout << "Sets the tile size in the N dimension for the rectangular example" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-tile_p=";
	std::cout << "Sets the tile size in the P dimension for the rectangular example" << std::endl;
	std::cout << std::left << std::setw(20) << "\t-default";
	std::cout << "Displays the default settings for these variables" << std::endl;
}

int main(int argc, char** argv)
{
	// All the defaults for the demo are set inside of MatrixMultiplication.h, then any edits made by the command line are changed
	// which demos to run, initially set to none, but if the commandline doesn't specify a specific benchmark then a demo of all will run
	bool testToRun[TestVariables::numberOfTotalTests] = { false, false, false, false, false, false, false, false, false, false, false, false, false };

	// create the command line parser
	pvr::platform::CommandLineParser parser(argc - 1, argv + 1);
	const pvr::CommandLine& cmdLine = parser.getParsedCommandLine();

	std::cout << "Single Precision General Matrix Multiplication (SGEMM) benchmarking test." << std::endl;

	// Control if the program should perform a cpu validation
	bool validate = false;
	cmdLine.getBoolOptionSetTrueIfPresent("-validate", validate);
	cmdLine.getBoolOptionSetTrueIfPresent("-va", validate);

	// print help
	if (cmdLine.hasOption("-help") || cmdLine.hasOption("-h"))
	{
		printHelp();
		exit(0);
	}

	// print information about this benchmark
	if (cmdLine.hasOption("-info") || cmdLine.hasOption("-i"))
	{
		std::cout << std::endl << "SGMM benchmark is a perfomance benchmark. Multiplication of large matrices with floating point elements." << std::endl;
		std::cout << "The benchmark is trying to test which type of shader performs best under different circumstances" << std::endl;
		exit(0);
	}

	// print the default variables
	if (cmdLine.hasOption("-default"))
	{
		std::cout << "These are the default options for the demo version of the benchmark." << std::endl;
		std::cout << "You may want to consider changing them to match your device's max workgroup size" << std::endl;

		std::cout << std::left << std::setw(20) << "\t-M= " << TestVariables::M << std::endl;
		std::cout << std::left << std::setw(20) << "\t-N= " << TestVariables::N << std::endl;
		std::cout << std::left << std::setw(20) << "\t-P= " << TestVariables::P << std::endl;
		std::cout << std::left << std::setw(20) << "\t-epsilon " << TestVariables::epsilon << std::endl;

		std::cout << std::left << std::setw(20) << "\t-naive_wg_width= " << TestVariables::naive_wg_width << std::endl;
		std::cout << std::left << std::setw(20) << "\t-naive_wg_height= " << TestVariables::naive_wg_height << std::endl;
		std::cout << std::left << std::setw(20) << "\t-linear_wg=" << TestVariables::linear_wg_size << std::endl;
		std::cout << std::left << std::setw(20) << "\t-tile_square_wg=" << TestVariables::tile_square_wg_size << std::endl;
		std::cout << std::left << std::setw(20) << "\t-tile_square_wf=" << TestVariables::tile_square_WF << std::endl;
		std::cout << std::left << std::setw(20) << "\t-tile_m=" << TestVariables::m_tile_size << std::endl;
		std::cout << std::left << std::setw(20) << "\t-tile_n=" << TestVariables::n_tile_size << std::endl;
		std::cout << std::left << std::setw(20) << "\t-tile_p=" << TestVariables::p_tile_size << std::endl;
		exit(0);
	}

	// Now take the the user input for which shader to run
	std::vector<std::string> shaderNames;
	if (cmdLine.getStringOptionList("-shaders", shaderNames))
	{
		for (size_t i = 0; i < shaderNames.size(); i++)
		{
			bool shaderFound = false;
			// check if this is recognised as a shader
			for (size_t j = 0; j < TestVariables::numberOfTotalTests; j++)
			{
				if (shaderNames[i] == TestVariables::Names[j])
				{
					testToRun[j] = true;
					shaderFound = true;
					break;
				}
			}
			if (!shaderFound) { std::cout << "\nCould not find a shader with the name : " << shaderNames[i] << std::endl; }
		}
	}
	else
	{
		// Run the demo version so turn on all shaders
		for (size_t i = 0; i < TestVariables::numberOfTotalTests; i++) { testToRun[i] = true; }
		std::cout << "Running demo version" << std::endl;
	}

	// now take the user input for the matrix dimensions
	cmdLine.getIntOption("-M", TestVariables::M);
	cmdLine.getIntOption("-m", TestVariables::M);
	cmdLine.getIntOption("-N", TestVariables::N);
	cmdLine.getIntOption("-n", TestVariables::N);
	cmdLine.getIntOption("-P", TestVariables::P);
	cmdLine.getIntOption("-p", TestVariables::P);

	// now take the user input for the wg dimensions
	cmdLine.getIntOption("-naive_wg_width", TestVariables::naive_wg_width);
	cmdLine.getIntOption("-naive_wg_height", TestVariables::naive_wg_height);
	cmdLine.getIntOption("-linear_wg", TestVariables::linear_wg_size);
	cmdLine.getIntOption("-tile_square_wg", TestVariables::tile_square_wg_size);
	cmdLine.getIntOption("-tile_square_wf", TestVariables::tile_square_WF);
	cmdLine.getIntOption("-tile_m", TestVariables::m_tile_size);
	cmdLine.getIntOption("-tile_n", TestVariables::n_tile_size);
	cmdLine.getIntOption("-tile_p", TestVariables::p_tile_size);

	// take the user input for percision of validation calculations
	cmdLine.getFloatOption("-epsilon", TestVariables::epsilon);

	// check that the user has inputted acceptable variables
	TestVariables::validateUserData();
	TestVariables::updateWorkgroupsToLaunch();

	std::cout << "M  " << TestVariables::M << "\t\tN  " << TestVariables::N << "\t\tP  " << TestVariables::P << std::endl;
	std::cout << "A (" << TestVariables::M << "x" << TestVariables::N << ") \tB (" << TestVariables::N << "x" << TestVariables::P << ")"
			  << "\tC (" << TestVariables::M << "x" << TestVariables::P << ")\n\n";

	// Produce random matrix data
	std::cout << std::left << std::setw(55) << "==Producing Matrix data";
	std::srand((unsigned int)timer.getElapsedNanoSecs());
	TestVariables::A = Matrix::RandomMat(TestVariables::M, TestVariables::N);
	TestVariables::B = Matrix::RandomMat(TestVariables::N, TestVariables::P);
	std::cout << "Done! " << std::left << std::setw(5) << timer.getElapsedMilliSecs() << " (ms)" << std::endl;
	timer.Reset();

	// run the appropriate list of benchmarks
	runBenchmarksWithList(testToRun, validate, argv[0]);
}

void runBenchmarksWithList(bool benchmarksToRun[], bool validate, char* pathToExecutable)
{
	if (validate)
	{
		// Do a cpu validation for the results
		std::cout << std::left << std::setw(55) << "==Calculating CPU validation";
		timer.Reset();
		TestVariables::C = Matrix::matMul(TestVariables::A, TestVariables::B);
		std::cout << "Done! " << std::left << std::setw(5) << timer.getElapsedMilliSecs() << " (ms)" << std::endl;
	}

	// first set up vulkan
	std::cout << std::left << std::setw(55) << "==Initiating Vulkan";
	timer.Reset();
	initiateVulkan(pathToExecutable);
	makeDescriptors();
	makePipelineLayout();
	makeBuffers(TestVariables::M, TestVariables::N, TestVariables::P);
	std::cout << "Done! " << std::left << std::setw(5) << timer.getElapsedMilliSecs() << " (ms)" << std::endl;

	// Compile all the shaders and prepare the compute pipelines for each test specified
	std::cout << std::left << std::setw(55) << "==Running tests\n";
	bool inputBufferFilled = false;
	timer.Reset();
	for (size_t i = 0; i < TestVariables::numberOfTotalTests; i++)
	{
		if (benchmarksToRun[i])
		{
			// create the pipeline for this event
			std::cout << "\tCompiling shader";
			if (i < 12) { makePipeline((int)i, TestVariables::XWorkgroupSize[i], TestVariables::YWorkgroupSize[i]); }
			else
			{
				// make the pipelines that use a rectangular setting
				makePipeline((int)i, TestVariables::XWorkgroupSize[i], TestVariables::YWorkgroupSize[i], TestVariables::n_tile_size);
			}

			// shader is compiled so we can either fill the input buffers with data or empty the results buffer
			if (!inputBufferFilled)
			{
				// fill the input buffers
				updateBuffers(TestVariables::A, TestVariables::B);
				inputBufferFilled = true;
			}
			else
			{
				// empty results buffers so that previous tests don't interfere
				emptyResultBuffers();
			}

			// The shader is compiled and we can now run the specific test
			std::cout << "\r";
			std::cout << "\t" << std::left << std::setw(47) << TestVariables::Names[i];
			// do the compute work twice, this is to to warm up the memory before timing begins
			doComputeWork(TestVariables::XWorkgroupsToLaunch[i], TestVariables::YWorkgroupsToLaunch[i]);

			// correctly timed compute work
			timer.Reset();
			doComputeWork(TestVariables::XWorkgroupsToLaunch[i], TestVariables::YWorkgroupsToLaunch[i]);
			std::cout << "Done! " << std::left << std::setw(5) << timer.getElapsedMilliSecs() << " (ms)";

			// if we are validating then check the output matrix against our cpu version.
			if (validate)
			{
				if (Matrix::validate(TestVariables::C, fetchResult(TestVariables::Transposed[i]), TestVariables::epsilon)) { std::cout << "  (SUCCESS)"; }
				else
				{
					std::cout << "  (FAILURE)";
				}
			}
			std::cout << std::endl;
		}
	}
}
