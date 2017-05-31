/*!
\brief Implementations of methods of the NativeLibrary class.
\file PVRCore/Base/NativeLibrary.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#import <Foundation/Foundation.h>
#include <dlfcn.h>

static const char* g_pszEnvVar = "PVRTRACE_LIB_PATH";

void* OpenFramework(const char * pszPath)
{
	@autoreleasepool
	{
		void* lib = NULL;
		
		// --- Set a global environment variable to point to this path (for VFrame usage)
		const char* slash = strrchr(pszPath, '/');
		if(slash)
		{
			char szPath[FILENAME_MAX];
			memset(szPath, 0, sizeof(szPath));
			strncpy(szPath, pszPath, slash-pszPath);
			setenv(g_pszEnvVar, szPath, 1);
		}
		else
		{
			// Use the current bundle path
			NSString* framework = [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] bundlePath], @"Contents/Frameworks/"];
			setenv(g_pszEnvVar, [framework UTF8String], 1);
		}
		
		// --- Make a temp symlink
		char szTempFile[FILENAME_MAX];
		memset(szTempFile, 0, sizeof(szTempFile));
		strcat(szTempFile, [NSTemporaryDirectory() UTF8String]);
		strcat(szTempFile, "tmp.XXXXXX");
		
		if(mkstemp(szTempFile))
		{
			if(symlink(pszPath, szTempFile) == 0)
			{
				lib = dlopen(szTempFile, RTLD_LAZY | RTLD_GLOBAL);
				remove(szTempFile);
			}
		}
		
		// --- Can't find the lib? Check the application framework folder instead.
		if(!lib)
		{
			NSString* framework = [NSString stringWithFormat:@"%@/%@/%s", [[NSBundle mainBundle] bundlePath], @"Contents/Frameworks/", pszPath];
			lib = dlopen([framework UTF8String], RTLD_LAZY | RTLD_GLOBAL);
			
			if(!lib)
			{
				const char * err = dlerror();
				if(err)
				{
					NSLog(@"dlopen failed with error: %s => %@", err, framework);
				}
			}
		}

		return lib;
	}
}
//!\endcond