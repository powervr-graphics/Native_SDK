/*!
\brief Used in the timeline semaphore example
\file TimelineData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#ifndef POWERVR_SDK_TIMELINEDATA_H
#define POWERVR_SDK_TIMELINEDATA_H

struct NoiseComputePushConstant{
	float scale{};
	int isFirstImage{false}; // Instead of bool for better compatybility
	glm::vec2 offset;

	NoiseComputePushConstant(float scale, int isFirstImage, const glm::vec2& offset) : scale(scale), isFirstImage(isFirstImage), offset(offset) {}
};
#endif // POWERVR_SDK_TIMELINEDATA_H
