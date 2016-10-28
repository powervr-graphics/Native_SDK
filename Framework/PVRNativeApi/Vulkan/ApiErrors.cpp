#include "PVRNativeApi/ApiErrors.h"

namespace pvr {
namespace api {

/* Vulkan has a different error model and does not return errors asynchronously.*/
int checkApiError(string* errOutStr) { return 0; }

/* Vulkan has a different error model and does not return errors asynchronously.*/
bool logApiError(const char* note, Logger::Severity severity) { return true; }

/* Vulkan has a different error model and does not return errors asynchronously.*/
bool succeeded(Result res)
{
	if (res == Result::Success)
	{
		return true;
	}
	Log(Log.Error, "%s", Log.getResultCodeString(res));
	return false;
}
}
}