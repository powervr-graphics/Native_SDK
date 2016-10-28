/*!*********************************************************************************************************************
\file         PVRShell\OS\ShellOS.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains the declaration of the ShellOS class. Most of the functionality is platform-specific, and as such is
              delegated to platform-specific ShellOS.cpp files. Do not access or use directly.
***********************************************************************************************************************/
#pragma once
#include "PVRShell/ShellIncludes.h"
#include "PVRShell/ShellData.h"
#include "PVRShell/Shell.h"
namespace pvr {
namespace platform {
//Forward declaration of internal implementation.
struct InternalOS;

/*!****************************************************************************************************************
\brief Implements a lot of the functionality and forwards to the platform from PVRShell. Users don't use directly,
       instead use the pvr::Shell class.
*******************************************************************************************************************/
class ShellOS
{
public:
	/*!****************************************************************************************************************
	\brief Capabilities that may be different between platforms.
	*******************************************************************************************************************/
	struct Capabilities
	{
		types::Capability resizable;
		types::Capability movable;
	};

	ShellData m_shellData;
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
		return m_capabilities;
	}

	Stream* getFileStream(const std::string filename);
	Result popUpMessage(const tchar* const title, const tchar* const message, ...) const;

	Shell* getShell() { return m_shell.get(); }


	void updatePointingDeviceLocation();
protected:
	std::auto_ptr<Shell> m_shell;
	std::string m_AppName;
	std::vector<std::string> m_ReadPaths;
	std::string m_WritePath;

private:
	OSApplication m_instance;
	InternalOS*	m_OSImplementation;
	static const Capabilities m_capabilities;
};

inline const string& ShellOS::getApplicationName() const { return m_AppName; }
inline void ShellOS::setApplicationName(const std::string& appName) { m_AppName = appName; }
inline const string& ShellOS::getDefaultReadPath() const
{
	assertion(m_ReadPaths.size() != 0);
	return m_ReadPaths[0];
}
inline const std::vector<std::string>& ShellOS::getReadPaths() const
{
	return m_ReadPaths;
}
inline const string& ShellOS::getWritePath() const
{
	return m_WritePath;
}
}
}
