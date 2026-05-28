/*!
\brief Implementation of function to load shader files
\file PVRSuperResolution/FileIO.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
	#include <Windows.h>
#endif

#if defined(__ANDROID__) // On android, external files are packaged in the .apk as assets
	#include <android/asset_manager.h>
	#include <android/log.h>
	#include <android_native_app_glue.h>
#endif

#include "Log.h"
#include "FileIO.h"

namespace pvr {

void loadFile(const std::string& fileName, std::vector<unsigned char>& pointerData, void* application)
{
#if defined(_WIN32)
	HRSRC hR = FindResource(GetModuleHandle(NULL), fileName.c_str(), RT_RCDATA);

	if (!hR)
	{
		logMessage(std::string("File could not be found:" + fileName).c_str());
		throw;
	}

	HGLOBAL hG = LoadResource(NULL, hR);

	if (!hG)
	{
		logMessage(std::string("File could not be loaded:" + fileName).c_str());
		throw;
	}
	// Get the data pointer itself. NB: Does not actually lock anything.
	void* pointerDataTemporal = LockResource(hG);
	size_t bufferSize = SizeofResource(NULL, hR);

	pointerData.resize(bufferSize);
	memcpy(pointerData.data(), pointerDataTemporal, bufferSize);

	UnlockResource(hG);
#endif
#if defined(__ANDROID__)
	struct android_app* app = static_cast<android_app*>(application);

	try
	{
		AAsset* _asset = AAssetManager_open(app->activity->assetManager, fileName.c_str(), AASSET_MODE_RANDOM);

		if (_asset == nullptr)
		{
			logMessage(std::string("Could not open file " + fileName).c_str());
			throw;
		}

		uint64_t assetSize = static_cast<uint64_t>(AAsset_getLength(_asset));
		pointerData.resize(assetSize);
		size_t dataRead = (size_t)AAsset_read(_asset, pointerData.data(), assetSize);

		if (dataRead == 0)
		{
			logMessage("Error reading shader file with size 0");
			throw;
		}
		else if (dataRead < 0)
		{
			logMessage("Error reading shader file with size < 0");
			throw;
		}

		AAsset_close(_asset);
	}
	catch (const std::exception& e)
	{
		logMessage("Exception reading file");
	}
#endif
#if defined(__linux__) && !defined(__ANDROID__)
	FILE* file = fopen(fileName.c_str(), "rb");

	if (!file)
	{
		logMessage("No file found");
		throw;
	}

	fseek(file, 0L, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	pointerData.resize(fileSize);

	size_t dataRead = fread(pointerData.data(), 1, fileSize, file);

	if (dataRead != fileSize)
	{
		if (feof(file) != 0)
		{
			logMessage("Error reading file feof(file) != 0");
			throw;
		}
		else
		{
			logMessage("Other error reading file");
			throw;
		}
	}
	fclose(file);
#endif
}

} // namespace pvr
