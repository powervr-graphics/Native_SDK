/*!
\brief Contains the declaration of the ShellOS class. Most of the functionality is platform-specific, and as such is
delegated to platform-specific ShellOS.cpp files. Do not access or use directly.
\file PVRShell/OS/ShellOS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/ShellIncludes.h"
#include "PVRShell/ShellData.h"
#include "PVRShell/Shell.h"
namespace pvr {
namespace platform {
//Forward declaration of internal implementation.
struct InternalOS;

/// <summary>Implements a lot of the functionality and forwards to the platform from PVRShell. Users don't use directly,
/// instead use the pvr::Shell class.</summary>
class ShellOS
{
public:
	/// <summary>Capabilities that may be different between platforms.</summary>
	struct Capabilities
	{
		types::Capability resizable;
		types::Capability movable;
	};

	ShellData _shellData;
	ShellOS(OSApplication instance, OSDATA osdata);

	virtual ~ShellOS();

	Result init(DisplayAttributes& data); // Accepts a data struct so it can overide default values
	Result initializeWindow(DisplayAttributes& data);
	bool isInitialized();
	void releaseWindow();

	//Getters
	OSApplication getApplication() const;
	OSDisplay getDisplay()  const;
	OSWindow getWindow() const;
	const std::string& getDefaultReadPath() const;
	const std::vector<std::string>& getReadPaths() const;
	const std::string& getWritePath() const;
	const std::string& getApplicationName() const;
	void setApplicationName(const std::string&);
	Result handleOSEvents();

	static const Capabilities&	getCapabilities()
	{
		return _capabilities;
	}

	Stream* getFileStream(const std::string filename);
	Result popUpMessage(const char8* const title, const char8* const message, ...) const;

	Shell* getShell() { return _shell.get(); }


	void updatePointingDeviceLocation();
protected:
	std::auto_ptr<Shell> _shell;
	std::string _AppName;
	std::vector<std::string> _ReadPaths;
	std::string _WritePath;

private:
	OSApplication _instance;
	InternalOS*	_OSImplementation;
	static const Capabilities _capabilities;
};

inline const string& ShellOS::getApplicationName() const { return _AppName; }
inline void ShellOS::setApplicationName(const std::string& appName) { _AppName = appName; }
inline const string& ShellOS::getDefaultReadPath() const
{
	assertion(_ReadPaths.size() != 0);
	return _ReadPaths[0];
}
inline const std::vector<std::string>& ShellOS::getReadPaths() const
{
	return _ReadPaths;
}
inline const string& ShellOS::getWritePath() const
{
	return _WritePath;
}
}
}