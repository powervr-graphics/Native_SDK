/*!
\brief Contains the implementation for a common pvr::platform::InternalOS class which will be used for Linux window systems.
\file PVRShell/OS/Linux/InternalOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "InternalOS.h"

#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"

#include <linux/input.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <signal.h>

#if !defined(CONNAME)
#define CONNAME "/dev/tty"
#endif

namespace pvr {
namespace platform {

// When using termios keypresses are reported as their ASCII values directly
// Most keys translate directly to the characters they represent
static Keys ASCIIStandardKeyMap[128] = {
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 0   */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Backspace, Keys::Tab, /* 5   */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Unknown, /* 10  */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 15  */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 20  */
	Keys::Unknown, Keys::Unknown, Keys::Escape, Keys::Unknown, Keys::Unknown, /* 25  */
	Keys::Unknown, Keys::Unknown, Keys::Space, Keys::Key1, Keys::Quote, /* 30  */
	Keys::Backslash, Keys::Key4, Keys::Key5, Keys::Key7, Keys::Quote, /* 35  */
	Keys::Key9, Keys::Key0, Keys::NumMul, Keys::NumAdd, Keys::Comma, /* 40  */
	Keys::Minus, Keys::Period, Keys::Slash, Keys::Key0, Keys::Key1, /* 45  */
	Keys::Key2, Keys::Key3, Keys::Key4, Keys::Key5, Keys::Key6, /* 50  */
	Keys::Key7, Keys::Key8, Keys::Key9, Keys::Semicolon, Keys::Semicolon, /* 55  */
	Keys::Comma, Keys::Equals, Keys::Period, Keys::Slash, Keys::Key2, /* 60  */
	Keys::A, Keys::B, Keys::C, Keys::D, Keys::E, /* upper case */ /* 65  */
	Keys::F, Keys::G, Keys::H, Keys::I, Keys::J, /* 70  */
	Keys::K, Keys::L, Keys::M, Keys::N, Keys::O, /* 75  */
	Keys::P, Keys::Q, Keys::R, Keys::S, Keys::T, /* 80  */
	Keys::U, Keys::V, Keys::W, Keys::X, Keys::Y, /* 85  */
	Keys::Z, Keys::SquareBracketLeft, Keys::Backslash, Keys::SquareBracketRight, Keys::Key6, /* 90  */
	Keys::Minus, Keys::Backquote, Keys::A, Keys::B, Keys::C, /* 95  */
	Keys::D, Keys::E, Keys::F, Keys::G, Keys::H, /* lower case */ /* 100 */
	Keys::I, Keys::J, Keys::K, Keys::L, Keys::M, /* 105 */
	Keys::N, Keys::O, Keys::P, Keys::Q, Keys::R, /* 110 */
	Keys::S, Keys::T, Keys::U, Keys::V, Keys::W, /* 115 */
	Keys::X, Keys::Y, Keys::Z, Keys::SquareBracketLeft, Keys::Backslash, /* 120 */
	Keys::SquareBracketRight, Keys::Backquote, Keys::Backspace /* 125 */
};

// Provides a mapping between a special key code combination with the associated Key
struct SpecialKeyCode
{
	const char* str;
	Keys key;
};

// Some codes for F-keys can differ, depending on whether we are reading a
// /dev/tty from within X or from a text console.
// Some keys (e.g. Home, Delete) have multiple codes one for the standard version and one for the numpad version.
static SpecialKeyCode ASCIISpecialKeyMap[] = { { "[A", Keys::Up }, { "[B", Keys::Down }, { "[C", Keys::Right }, { "[D", Keys::Left },
	{ "[E", Keys::Key5 }, // Numpad 5 has no second function - do this to avoid the code being interpreted as Escape.
	{ "OP", Keys::F1 }, // Within X
	{ "[[A", Keys::F1 }, // Text console
	{ "OQ", Keys::F2 }, // Within X
	{ "[[B", Keys::F2 }, // Text console
	{ "OR", Keys::F3 }, // Within X
	{ "[[C", Keys::F3 }, // Text console
	{ "OS", Keys::F4 }, // Within X
	{ "[[D", Keys::F4 }, // Text console
	{ "[15~", Keys::F5 }, // Within X
	{ "[[E", Keys::F5 }, // Text console
	{ "[17~", Keys::F6 }, { "[18~", Keys::F7 }, { "[19~", Keys::F8 }, { "[20~", Keys::F9 }, { "[21~", Keys::F10 }, { "[23~", Keys::F11 }, { "[24~", Keys::F12 },
	{ "[1~", Keys::Home }, { "OH", Keys::Home }, { "[2~", Keys::Insert }, { "[3~", Keys::Delete }, { "[4~", Keys::End }, { "OF", Keys::End }, { "[5~", Keys::PageUp },
	{ "[6~", Keys::PageDown }, { NULL, Keys::Unknown } };

// This mapping is taken from input-event-codes.h - see http://www.usb.org/developers/hidpage
static Keys KeyboardKeyMap[] = { Keys::Unknown, Keys::Escape, Keys::Key1, Keys::Key2, Keys::Key3, Keys::Key4, Keys::Key5, Keys::Key6, Keys::Key7, Keys::Key8, Keys::Key9,
	Keys::Key0, Keys::Minus, Keys::Equals, Keys::Backspace, Keys::Tab, Keys::Q, Keys::W, Keys::E, Keys::R, Keys::T, Keys::Y, Keys::U, Keys::I, Keys::O, Keys::P,
	Keys::SquareBracketLeft, Keys::SquareBracketRight, Keys::Return, Keys::Control, Keys::A, Keys::S, Keys::D, Keys::F, Keys::G, Keys::H, Keys::J, Keys::K, Keys::L,
	Keys::Semicolon, Keys::Quote, Keys::Backquote, Keys::Shift, Keys::Backslash, Keys::Z, Keys::X, Keys::C, Keys::V, Keys::B, Keys::N, Keys::M, Keys::Comma, Keys::Period,
	Keys::Slash, Keys::Shift, Keys::NumMul, Keys::Alt, Keys::Space, Keys::CapsLock, Keys::F1, Keys::F2, Keys::F3, Keys::F4, Keys::F5, Keys::F6, Keys::F7, Keys::F8, Keys::F9,
	Keys::F10, Keys::NumLock, Keys::ScrollLock, Keys::Num7, Keys::Num8, Keys::Num9, Keys::NumSub, Keys::Num4, Keys::Num5, Keys::Num6, Keys::NumAdd, Keys::Num1, Keys::Num2,
	Keys::Num3, Keys::Num0, Keys::NumPeriod, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::F11, Keys::F12, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown,
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Control, Keys::NumDiv, Keys::PrintScreen, Keys::Alt, Keys::Unknown, Keys::Home, Keys::Up, Keys::PageUp,
	Keys::Left, Keys::Right, Keys::End, Keys::Down, Keys::PageDown, Keys::Insert, Keys::Delete, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown,
	Keys::Unknown, Keys::Unknown, Keys::Pause, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::SystemKey1, Keys::SystemKey1, Keys::SystemKey2,
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown };

static struct termios OriginalTermio;
static int TtyFileDescriptor;

static struct sigaction OldSIGSEGVAction;
static struct sigaction OldSIGINTAction;
static struct sigaction OldSIGTERMAction;

// Forward declare functions used by the signal handler
static void restoreTtyState();
static void uninstallSignalHandlers();

// Provides a callback for particular signals on Linux platforms. This function is used specifically for resetting modified state
static void signalHandler(int sig, siginfo_t* si, void* ucontext)
{
	// Restore the tty to its original state
	restoreTtyState();

	// Uninstall the signal handlers (sigactions) and revert them back to their original state
	uninstallSignalHandlers();
}

static void restoreTtyState()
{
	// Reopen the tty so that we can restore the state
	if (!TtyFileDescriptor)
	{
		if ((TtyFileDescriptor = open(CONNAME, O_RDWR | O_NONBLOCK)) <= 0) { Log(LogLevel::Warning, "Unable to open '%s' for resetting attributes", CONNAME); }
	}

	// Recover tty state.
	if (tcsetattr(TtyFileDescriptor, TCSAFLUSH, &OriginalTermio) == -1) { Log(LogLevel::Error, "Unable to reset attributes for '%s'. Unable to recover the tty state", CONNAME); }
}

static void uninstallSignalHandlers()
{
	struct sigaction currentSignalAction;

	// SIGSEGV
	if (sigaction(SIGSEGV, nullptr, &currentSignalAction) == 0)
	{
		if (currentSignalAction.sa_sigaction == &signalHandler)
		{
			sigaction(SIGSEGV, &OldSIGSEGVAction, nullptr);
			memset(&OldSIGSEGVAction, 0, sizeof(OldSIGSEGVAction));
		}
	}

	// SIGINT
	if (sigaction(SIGINT, nullptr, &currentSignalAction) == 0)
	{
		if (currentSignalAction.sa_sigaction == &signalHandler)
		{
			sigaction(SIGINT, &OldSIGINTAction, nullptr);
			memset(&OldSIGINTAction, 0, sizeof(OldSIGINTAction));
		}
	}

	// SIGTERM
	if (sigaction(SIGTERM, nullptr, &currentSignalAction) == 0)
	{
		if (currentSignalAction.sa_sigaction == &signalHandler)
		{
			sigaction(SIGTERM, &OldSIGTERMAction, nullptr);
			memset(&OldSIGTERMAction, 0, sizeof(OldSIGTERMAction));
		}
	}
}

static void installSignalHandler()
{
	struct sigaction signalAction;
	struct sigaction currentSignalAction;

	memset(&signalAction, 0, sizeof(signalAction));

	signalAction.sa_sigaction = &signalHandler;
	signalAction.sa_flags = SA_RESETHAND;

	// SIGSEGV
	if (sigaction(SIGSEGV, nullptr, &currentSignalAction) != 0 || currentSignalAction.sa_sigaction != &signalHandler)
	{
		signalAction.sa_sigaction = &signalHandler;
		sigaction(SIGSEGV, &signalAction, &OldSIGSEGVAction);
	}

	// SIGINT
	if (sigaction(SIGINT, nullptr, &currentSignalAction) != 0 || currentSignalAction.sa_sigaction != &signalHandler)
	{
		sigdelset(&signalAction.sa_mask, SIGINT);
		signalAction.sa_sigaction = &signalHandler;
		sigaction(SIGINT, &signalAction, &OldSIGINTAction);
	}

	// SIGTERM
	if (sigaction(SIGTERM, nullptr, &currentSignalAction) != 0 || currentSignalAction.sa_sigaction != &signalHandler)
	{
		sigdelset(&signalAction.sa_mask, SIGTERM);
		signalAction.sa_sigaction = &signalHandler;
		sigaction(SIGTERM, &signalAction, &OldSIGTERMAction);
	}
}

InternalOS::InternalOS(ShellOS* shellOS) : _isInitialized(false), _shellOS(shellOS)
{
	OriginalTermio = { 0 };
	TtyFileDescriptor = 0;

	// Attempt to open the tty (the terminal connected to standard input) as read/write
	// Note that because O_NONBLOCK has been used termio.c_cc[VTIME] will be ignored so is not set
	if ((TtyFileDescriptor = open(CONNAME, O_RDWR | O_NONBLOCK)) <= 0) { Log(LogLevel::Warning, "Unable to open '%s'", CONNAME); }
	else
	{
		// Reads the current set of terminal attributes to the struct
		tcgetattr(TtyFileDescriptor, &OriginalTermio);

		// Ensure that on-exit the terminal state is restored as per the original termios structure retrieved above
		atexit(restoreTtyState);

		// Used as a modified set of termios attriutes
		struct termios termio;

		// Take a copy of the original termios structure.
		// The original set attributes will be modified and used as the source termios structure in a tcsetattr call to update the terminal attributes
		termio = OriginalTermio;

		// Enables raw mode (input is available character per character, echoing is disabled and special processing of terminal input and output characters is disabled)
		cfmakeraw(&termio);

		// Re-enable the use of Ctrl-C for sending a SIGINT signal to the current process causing it to terminate
		// Also re-enables Ctrl-Z which can be used to send a SIGTSTP signal.
		termio.c_lflag |= ISIG;

		// re-enable NL -> CR-NL expansion on output
		termio.c_oflag |= OPOST | ONLCR;

		// Set the minimum number of characters to read in bytes
		termio.c_cc[VMIN] = 1;

		// Update the attributes of the current terminal
		if (tcsetattr(TtyFileDescriptor, TCSANOW, &termio) == -1) { Log(LogLevel::Error, "Unable to set attributes for '%s'", CONNAME); }

		Log(LogLevel::Information, "Opened '%s' for input", CONNAME);
	}

	// Restore the terminal console on SIGINT, SIGSEGV and SIGABRT
	installSignalHandler();

	{
		// Construct our read paths and write path
		std::string selfProc = "/proc/self/exe";

		ssize_t exePathLength(PATH_MAX);
		std::string exePath(exePathLength, 0);
		exePathLength = readlink(selfProc.c_str(), &exePath[0], exePathLength - 1);
		if (exePathLength == -1) { Log(LogLevel::Warning, "Readlink %s failed. The application name, read path and write path have not been set", selfProc.c_str()); }
		else
		{
			// Resize the executable path based on the result of readlink
			exePath.resize(exePathLength);

			Log(LogLevel::Debug, "Found executable path: '%s'", exePath.c_str());

			FilePath filepath(exePath);
			_shellOS->setApplicationName(filepath.getFilenameNoExtension());
			_shellOS->setWritePath(filepath.getDirectory() + FilePath::getDirectorySeparator());
			_shellOS->clearReadPaths();
			_shellOS->addReadPath(filepath.getDirectory() + FilePath::getDirectorySeparator());
			_shellOS->addReadPath(std::string(".") + FilePath::getDirectorySeparator());
			_shellOS->addReadPath(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
			_shellOS->addReadPath(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets_" + filepath.getFilenameNoExtension() + FilePath::getDirectorySeparator());
		}
	}
}

InternalOS::~InternalOS()
{
	if (TtyFileDescriptor)
	{
		close(TtyFileDescriptor);
		TtyFileDescriptor = 0;
	}

	uninstallSignalHandlers();
}

Keys InternalOS::getSpecialKey(unsigned char firstCharacter) const
{
	uint32_t numCharacters = 0;
	unsigned char currentKey = 0;
	// Escape keys will input up to 4 characters including the first character to the terminal i.e. 3 additonal character bytes
	// rfc3629 states that the maximum number of bytes per character is 4: https://tools.ietf.org/html/rfc3629
	const unsigned int maxNumExtraCharacterBytes = 3;
	// Leave room at the end of the buffer for a null terminator
	unsigned char buf[maxNumExtraCharacterBytes + 1];

	// Read until we have the full key.
	while ((read(TtyFileDescriptor, &currentKey, 1) == 1) && numCharacters < maxNumExtraCharacterBytes) { buf[numCharacters++] = currentKey; }
	buf[numCharacters] = '\0';

	uint32_t pos = 0;

	// Find the matching special key from the map.
	while (ASCIISpecialKeyMap[pos].str != NULL)
	{
		if (!strcmp(ASCIISpecialKeyMap[pos].str, (const char*)buf)) { return ASCIISpecialKeyMap[pos].key; }
		pos++;
	}

	// If additional character bytes have been retrieved and a matching key has not been found there has been an unrecognised special key read.
	if (numCharacters > 0) { return Keys::Unknown; }
	else
	// No additional character bytes have been retrieved meaning it must just be the first character - a key corresponding to an escape key
	{
		return ASCIIStandardKeyMap[firstCharacter];
	}
}

Keys InternalOS::getKeyFromAscii(unsigned char initialKey)
{
	Keys key = Keys::Unknown;

	if (initialKey < ARRAY_SIZE(KeyboardKeyMap))
	{
		// Check for special multi-character keys
		// CHeck for escape sequences (they start with a '27' byte which matches the escape key)
		if ((ASCIIStandardKeyMap[initialKey] == Keys::Escape) || (ASCIIStandardKeyMap[initialKey] == Keys::F7)) { key = getSpecialKey(initialKey); }
		else
		{
			key = ASCIIStandardKeyMap[initialKey];
		}
	}

	return key;
}

Keys InternalOS::getKeyFromEVCode(uint32_t keycode)
{
	if (keycode >= ARRAY_SIZE(KeyboardKeyMap)) { return Keys::Unknown; }
	return KeyboardKeyMap[keycode];
}

bool InternalOS::handleOSEvents(std::unique_ptr<Shell>& shell)
{
	// Check user input from the available input devices.

	// Check input from tty
	if (TtyFileDescriptor > 0)
	{
		unsigned char initialKey = 0;

		// Read a single byte using the TTY file descriptor
		int bytesRead = read(TtyFileDescriptor, &initialKey, 1);
		Keys key = Keys::Unknown;

		// If the number of bytes is not 0 and the initial key has been set then determine the corresponding key
		if ((bytesRead > 0) && initialKey) { key = getKeyFromAscii(initialKey); }
		shell->onKeyDown(key);
		shell->onKeyUp(key);
	}

	return true;
}
} // namespace platform
} // namespace pvr
//!\endcond
