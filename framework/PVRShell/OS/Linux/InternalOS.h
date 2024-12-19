/*!
\brief Contains a common implementation for pvr::platform::InternalOS specifically for Linux platforms.
\file PVRShell/OS/Linux/InternalOS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/OS/ShellOS.h"

namespace pvr {
namespace platform {

class InternalOS
{
public:
	InternalOS(ShellOS* shellOS);
	virtual ~InternalOS();

	void setIsInitialized(bool isInitialized) { _isInitialized = isInitialized; }

	ShellOS* getShellOS() { return _shellOS; }

	const ShellOS* getShellOS() const { return _shellOS; }

	bool isInitialized() { return _isInitialized; }

	virtual bool handleOSEvents(std::unique_ptr<Shell>& shell);

	Keys getKeyFromAscii(unsigned char initialKey);
	Keys getKeyFromEVCode(uint32_t keycode);

private:
	bool _isInitialized;
	ShellOS* _shellOS;

	Keys getSpecialKey(unsigned char firstCharacter) const;
};
} // namespace platform
} // namespace pvr
