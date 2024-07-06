/*!*********************************************************************************************************************
\File			MatrixMultiplicationGPU.cpp
\Title			Vulkan Compute implementation of the different multiplication strategies
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Covers the GPU and vulkan side of implementing the benchmark
***********************************************************************************************************************/
#include "MatrixMultiplicationGPU.h"

// platform specific resource loaders for file reading on windows
#if defined(_WIN32)
#include "PVRCore/Windows/WindowsResourceStream.h"
#endif

// All the vulkan resources used in this program
std::unique_ptr<DeviceResources> _resources;

// Strings to represent the shader source code and the the filepaths to the shaders
std::string TemplateShaderSource = "";
std::string TemplateShaderFilePath = "MatMulTemplate.csh";
std::string ShaderFilePaths[13] = { "mat_mul_naive_AT.csh", "mat_mul_naive_BT.csh", "mat_mul_naive_CT.csh", "mat_mul_naive_ATCT.csh", "mat_mul_naive_BTCT.csh",
	"mat_mul_linearwg_AT.csh", "mat_mul_linearwg_BT.csh", "mat_mul_linearwg_vec4.csh", "mat_mul_linearwg_vec4_local.csh", "mat_mul_tile.csh", "mat_mul_tile_vec4.csh",
	"mat_mul_tile_WF.csh", "mat_mul_rect.csh" };

// dimensions of the matrices
uint32_t Mat_M = 0;
uint32_t Mat_N = 0;
uint32_t Mat_P = 0;

pvr::FilePath pathToExe = pvr::FilePath("temp");

void initiateVulkan(char* pathToExecutable)
{
	// store the executable path for loading the shaders later
	pathToExe = pvr::FilePath(pathToExecutable);

	_resources = std::make_unique<DeviceResources>();

	// Create a Vulkan 1.0 instance and retrieve compatible physical devices
	pvr::utils::VulkanVersion VulkanVersion(1, 0, 0);
	_resources->instance = pvr::utils::createInstance("VulkanMatrixMultiplication", VulkanVersion, pvr::utils::InstanceExtensions(VulkanVersion));

	if (_resources->instance->getNumPhysicalDevices() == 0)
	{
		std::cout << std::endl << "There are no vulkan enabled devices connected" << std::endl;
		exit(-1);
	}

	// create debug callbacks
	_resources->debugUtilCallbacks = pvr::utils::createDebugUtilsCallbacks(_resources->instance);

	// create the compute command queue and device
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_COMPUTE_BIT };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	// create the device and retrieve the caommand queue from it
	_resources->device = pvr::utils::createDeviceAndQueues(_resources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);
	_resources->commandQueue = _resources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_resources->commandQueue->setObjectName("ComputeQueue");

	// create the memory allocator
	_resources->vma = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_resources->device));

	// create and allocate the command pool from the command queue
	_resources->commandPool = _resources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(_resources->commandQueue->getFamilyIndex()));

	// allocate the command buffers out of the command pool
	_resources->primaryCommandBuffer = _resources->commandPool->allocateCommandBuffer();
	_resources->primaryCommandBuffer->setObjectName("PrimaryCommandBuffer");

	// create the descriptor pool where descriptor sets will be allocated from
	pvrvk::DescriptorPoolCreateInfo descPoolCreateInfo;
	descPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, 8);
	descPoolCreateInfo.setMaxDescriptorSets(1);
	_resources->descriptorPool = _resources->device->createDescriptorPool(descPoolCreateInfo);
	_resources->descriptorPool->setObjectName("DescriptorPool");
}

void makeDescriptors()
{
	// create the descriptor layout
	pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo;

	// The three matrix ssbos
	layoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	layoutCreateInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	layoutCreateInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	// The transposed matrix ssbos
	layoutCreateInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	layoutCreateInfo.setBinding(4, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	layoutCreateInfo.setBinding(5, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	// The vec4 matrix ssbos
	layoutCreateInfo.setBinding(6, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	layoutCreateInfo.setBinding(7, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_resources->descriptorSetLayout = _resources->device->createDescriptorSetLayout(layoutCreateInfo);

	// allocate the descriptors out of the descriptor pool using the layout
	_resources->descriptorSet = _resources->descriptorPool->allocateDescriptorSet(_resources->descriptorSetLayout);
	_resources->descriptorSet->setObjectName("DescriptorSet");
}

void makeSingleMatrixBuffer(int bufferIndex, int numOfElements)
{
	_resources->matrixBufferSSBOs[bufferIndex] =
		pvr::utils::createBuffer(_resources->device, pvrvk::BufferCreateInfo(sizeof(float) * numOfElements, pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _resources->vma, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_resources->matrixBufferSSBOs[bufferIndex]->setObjectName("MatrixBufferSBO");
}

pvrvk::WriteDescriptorSet makeSingleMatrixDescSet(int bufferIndex, int numOfElements)
{
	// the buffer index in the array stored on cpu side matches the binding points defined in the shaders
	pvrvk::WriteDescriptorSet toWrite(pvrvk::DescriptorType::e_STORAGE_BUFFER, _resources->descriptorSet, bufferIndex, 0);
	toWrite.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_resources->matrixBufferSSBOs[bufferIndex], 0, sizeof(float) * numOfElements));
	return toWrite;
}

void makeBuffers(uint32_t M, uint32_t N, uint32_t P)
{
	// A is a (MxN) B is a (NxP) and C is therefore (MxP)
	// Store the dimensions of these matrices locally so that they can be later passed to the shaders during runtime compilation
	Mat_M = M;
	Mat_N = N;
	Mat_P = P;

	// Now allocate the buffer space for the different matrices
	// A(MxN)
	makeSingleMatrixBuffer(0, M * N);
	// B(NxP)
	makeSingleMatrixBuffer(1, N * P);
	// C(MxP)
	makeSingleMatrixBuffer(2, M * P);

	// AT(NxM)
	makeSingleMatrixBuffer(3, M * N);
	// BT(PxN)
	makeSingleMatrixBuffer(4, N * P);
	// CT(PxM)
	makeSingleMatrixBuffer(5, M * P);

	// Vec4A
	makeSingleMatrixBuffer(6, M * N);
	// Vec4BT
	makeSingleMatrixBuffer(7, N * P);

	// Associate all of the buffers to their biffer views
	for (uint32_t i = 0; i < _resources->matrixBufferCount; i++)
	{
		_resources->matrixBufferViews[i].pointToMappedMemory(_resources->matrixBufferSSBOs[i]->getDeviceMemory()->getMappedData());
	}
	// update the descriptor sets using a writer to update them all in one go
	std::vector<pvrvk::WriteDescriptorSet> descSetWriter;

	descSetWriter.push_back(makeSingleMatrixDescSet(0, M * N));
	descSetWriter.push_back(makeSingleMatrixDescSet(1, N * P));
	descSetWriter.push_back(makeSingleMatrixDescSet(2, M * P));

	descSetWriter.push_back(makeSingleMatrixDescSet(3, M * N));
	descSetWriter.push_back(makeSingleMatrixDescSet(4, N * P));
	descSetWriter.push_back(makeSingleMatrixDescSet(5, M * P));

	descSetWriter.push_back(makeSingleMatrixDescSet(6, M * N));
	descSetWriter.push_back(makeSingleMatrixDescSet(7, N * P));

	// update the descriptor sets
	_resources->device->updateDescriptorSets(descSetWriter.data(), (uint32_t)descSetWriter.size(), nullptr, 0);
}

// Usually an asset stream would be gathered using a method belonging to the pvr::Shell
// Since this is a headless app there is no access to the shell methods, reproduce its functionality here
auto readShaderSource(std::string& filePath)
{
	// first check the relative path, this will most likely work best in debugging mode
	auto fs = pvr::FileStream::createFileStream(filePath, "rb", false);
	if (fs->isReadable()) { return fs; }
	fs.reset(0);

	// Next try the absolute file path, MacOS packages the file inside an .app which has places resources differently and then
	// launches a unix file from in inside the contents of the app. this means depending on the platform a different file path
	// is needed. On Android becuase this is a command line example it is not packaged into an .apk like usual and is essentially
	// a linux example as far as file reading is concerned.

#if defined(__APPLE__)
	std::string OSFilePath = pathToExe.getDirectory() + pvr::FilePath::getDirectorySeparator() + ".." + pvr::FilePath::getDirectorySeparator() + "Resources" +
		pvr::FilePath::getDirectorySeparator() + filePath;
#else
	std::string OSFilePath = pathToExe.getDirectory() + pvr::FilePath::getDirectorySeparator() + "Assets_VulkanMatrixMultiplication" + pvr::FilePath::getDirectorySeparator() + filePath;
#endif

	// Load that file into the file file stream
	fs = pvr::FileStream::createFileStream(OSFilePath, "rb", false);
	// Now test if this file is now readable
	if (fs->isReadable()) { return fs; }
	fs.reset(0);

// Now now try a special window specific loading tactic, this is to use our packaged resources.rc file.
#if defined(_WIN32)
	try
	{
		fs = std::make_unique<pvr::WindowsResourceStream>(filePath);
	}
	catch (pvr::FileNotFoundError err)
	{
		std::cout << "\nFile not found with windows specific method\n" << err.what() << std::endl;
	}
	if (fs->isReadable()) { return fs; }
	fs.reset(0);
#endif

	// Now if all these methods have failed then exit the program because an essential file has failed to be loaded
	std::cout << "\nFile loading for : " << filePath << " has failed unexpectedly, now exiting \n";
	exit(-1);
}

void makePipelineLayout()
{
	// create the layout for the pipeline using the descriptor set layout
	pvrvk::PipelineLayoutCreateInfo layoutCreateInfo;
	layoutCreateInfo.addDescSetLayout(_resources->descriptorSetLayout);

	_resources->pipelineLayout = _resources->device->createPipelineLayout(layoutCreateInfo);

	// at this stage load in the template shader source code
	// auto fs = pvr::FileStream::createFileStream(TemplateShaderFilePath, "rb", false);
	auto fs = readShaderSource(TemplateShaderFilePath);

	fs->readIntoString(TemplateShaderSource);
	// check the shader isn't empty
	if (TemplateShaderSource.empty())
	{
		std::cout << "Template shader contained no data, file path is most likely incorrect";
		exit(-1);
	}
}

void makePipeline(int shaderIndex, int xWorkgroupSize, int yWorkgroupSize, int nTileSize)
{
	// Construct the header string with all the defines used for this shader
	std::stringstream shaderSourceCode;
	shaderSourceCode << "#"
					 << "version 320 es";
	shaderSourceCode << "\n#define M " << Mat_M;
	shaderSourceCode << "\n#define N " << Mat_N;
	shaderSourceCode << "\n#define P " << Mat_P;
	shaderSourceCode << "\n#define WG_X_SIZE " << xWorkgroupSize;
	shaderSourceCode << "\n#define WG_Y_SIZE " << yWorkgroupSize;

	// Almost all properties for the shader can be reconstructed from the workgroup sizes
	// However if the nTile is set to be not the default then the shader we are constructing is
	// for a rectangular example and therefore needs the extra information.
	if (nTileSize) { shaderSourceCode << "\n#define N_TILE " << nTileSize; }

	// Read in the specific shader source for this module
	// pass this header onto the pipeline creation
	auto fs = readShaderSource(ShaderFilePaths[shaderIndex]);
	std::string individualShaderSourceCode;
	fs->readIntoString(individualShaderSourceCode);

	// combine the header code, the template and then the individual shader code
	shaderSourceCode << "\n" << TemplateShaderSource << "\n" << individualShaderSourceCode;
	std::string CompleteSourceCode = shaderSourceCode.str();
	pvrvk::ShaderModule shader;

	// create a shader module
	try
	{
		shader = pvr::utils::createShaderModule(_resources->device, CompleteSourceCode, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	}
	catch (pvrvk::Error& err)
	{
		std::cout << "Error compiling shader for pipeline index" << shaderIndex << std::endl;
		std::cout << err.what() << "\n\nDumping shader source code:\n" << shaderSourceCode.str() << std::endl;
		exit(-1);
	}

	// using the shader module and the pipeline layout to create the pipeline create info
	pvrvk::ComputePipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.computeShader = shader;
	pipelineCreateInfo.pipelineLayout = _resources->pipelineLayout;

	// finally create the pipeline, sometimes even if the shader source code is valid the creation of the pipeline will fail
	try
	{
		_resources->computePipeline = _resources->device->createComputePipeline(pipelineCreateInfo);
		_resources->computePipeline->setObjectName("ComputePipeline");
	}
	catch (pvrvk::ErrorOutOfHostMemory const& err)
	{
		std::cout << "\nError Ran out of host memory while trying to create pipeline for shader " << ShaderFilePaths[shaderIndex] << std::endl;
		std::cout << "This can happen happen because shared memory used for this shader is too large for the device, try altering the tile sizes, use the option '-h' for more "
					 "information"
				  << std::endl;
		std::cout << err.what() << std::endl;
		exit(-1);
	}
	catch (pvrvk::Error const& err)
	{
		std::cout << "\nUnexpected error occured when creating the compute pipeline for the shader" << ShaderFilePaths[shaderIndex] << std::endl;
		std::cout << "\n" << err.what() << std::endl;
		exit(-1);
	}
}

void updateBuffers(Matrix LHS, Matrix RHS)
{
	// update the contents of the buffer
	// A is (MxN) B is (NxP)
	pvr::utils::updateHostVisibleBuffer(_resources->matrixBufferSSBOs[0], LHS.data(), 0, sizeof(float) * Mat_M * Mat_N);
	pvr::utils::updateHostVisibleBuffer(_resources->matrixBufferSSBOs[1], RHS.data(), 0, sizeof(float) * Mat_N * Mat_P);
	// transposed matrices
	pvr::utils::updateHostVisibleBuffer(_resources->matrixBufferSSBOs[3], Matrix::transpose(LHS).data(), 0, sizeof(float) * Mat_M * Mat_N);
	pvr::utils::updateHostVisibleBuffer(_resources->matrixBufferSSBOs[4], Matrix::transpose(RHS).data(), 0, sizeof(float) * Mat_N * Mat_P);
	// the vec4 representations
	pvr::utils::updateHostVisibleBuffer(_resources->matrixBufferSSBOs[6], LHS.data(), 0, sizeof(float) * Mat_M * Mat_N);
	pvr::utils::updateHostVisibleBuffer(_resources->matrixBufferSSBOs[7], Matrix::transpose(RHS).data(), 0, sizeof(float) * Mat_N * Mat_P);

	// If the device doesn't have coherant memory, it has to be flushed.
	if (static_cast<uint32_t>(_resources->matrixBufferSSBOs[0]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		// Flush the entire range of all of the SSBOS
		for (size_t i = 0; i < 8; i++) { _resources->matrixBufferSSBOs[i]->getDeviceMemory()->flushRange(0, _resources->matrixBufferViews[i].getSize()); }
	}
}

Matrix fetchResult(bool transposed)
{
	// If the device doesn't have coherant memory, it has to be flushed.
	if (static_cast<uint32_t>(_resources->matrixBufferSSBOs[0]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		// Flush the entire range of all of the SSBOS
		for (size_t i = 0; i < 8; i++) { _resources->matrixBufferSSBOs[i]->getDeviceMemory()->flushRange(0, _resources->matrixBufferViews[i].getSize()); }
	}
	// Get the correct result based on if the product was transposed or not
	if (!transposed)
	{
		float* m = (float*)_resources->matrixBufferViews[2].getMappedMemory();
		// M is a (MxP)
		Matrix Prod(Mat_M, Mat_P, m);
		return Prod;
	}
	else
	{
		// float* m = (float*)_resources->bufferViewProdMatrix[1].getMappedMemory();
		float* m = (float*)_resources->matrixBufferViews[5].getMappedMemory();
		// MT is a (PxM) Matrix
		Matrix ProdT(Mat_P, Mat_M, m);
		return Matrix::transpose(ProdT);
	}
}

void emptyResultBuffers()
{
	float* productBuffer1 = (float*)_resources->matrixBufferViews[2].getMappedMemory();
	float* productBuffer2 = (float*)_resources->matrixBufferViews[5].getMappedMemory();
	for (uint32_t i = 0; i < Mat_M * Mat_P; i++)
	{
		productBuffer1[i] = 0;
		productBuffer2[i] = 0;
	}
	// If the device doesn't have coherant memory, it has to be flushed.
	if (static_cast<uint32_t>(_resources->matrixBufferSSBOs[0]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		// Flush the entire range of all of the SSBOS
		for (size_t i = 0; i < 8; i++) { _resources->matrixBufferSSBOs[i]->getDeviceMemory()->flushRange(0, _resources->matrixBufferViews[i].getSize()); }
	}
}

void doComputeWork(int xWorkgroupNumber, int yWorkgroupNumber)
{
	// Fill the command buffer
	_resources->commandPool->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);
	_resources->primaryCommandBuffer->begin();
	pvr::utils::beginCommandBufferDebugLabel(_resources->primaryCommandBuffer, pvrvk::DebugUtilsLabel("MainComputePass"));
	_resources->primaryCommandBuffer->bindPipeline(_resources->computePipeline);
	_resources->primaryCommandBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _resources->pipelineLayout, 0, _resources->descriptorSet);
	_resources->primaryCommandBuffer->dispatch(xWorkgroupNumber, yWorkgroupNumber, 1);
	pvr::utils::endCommandBufferDebugLabel(_resources->primaryCommandBuffer);
	_resources->primaryCommandBuffer->end();

	// create the submit command queue
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_resources->primaryCommandBuffer;
	submitInfo.numCommandBuffers = 1;
	_resources->commandQueue->submit(submitInfo);

	// attempt to do the compute work
	try
	{
		_resources->commandQueue->waitIdle();
		_resources->device->waitIdle();
	}
	catch (pvrvk::ErrorDeviceLost& err)
	{
		std::cout << "\nError Device lost, computation was perhaps too large for this test" << std::endl;
		std::cout << "Try changing the matrix settings, use -h for more info \n\n" << err.what() << std::endl;
		exit(-1);
	}
	catch (pvrvk::Error& err)
	{
		std::cout << "unplanned error\n\n" << err.what() << std::endl;
		exit(-1);
	}
}
