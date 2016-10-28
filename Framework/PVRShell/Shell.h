/*!*********************************************************************************************************************
\file         PVRShell\Shell.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the all-important pvr::Shell class that the user will be inheriting from for his application.
              See bottom of this file or of any Demo file for the newDemo function the user must implement for his application.
***********************************************************************************************************************/
#pragma once
#include "PVRShell/CommandLine.h"

#include <bitset>


/*!*********************************************************************************************************************
\brief       Main namespace for the PowerVR Framework
***********************************************************************************************************************/
namespace pvr {

//!\cond NO_DOXYGEN
struct PointerLocationStore
{
	int16 x; int16 y;
};
//!\endcond

/*!*********************************************************************************************************************
\brief        Mouse pointer coordinates.
***********************************************************************************************************************/
class PointerLocation : public PointerLocationStore
{
public:
	PointerLocation() { }
	PointerLocation(const PointerLocationStore& st) : PointerLocationStore(st) { }
	PointerLocation(int16 x, int16 y) { this->x = x; this->y = y; }
};

/*!*********************************************************************************************************************
\brief        Enumeration representing a simplified, unified input event designed to unify simple actions across different
devices.
***********************************************************************************************************************/
enum class SimplifiedInput
{
	NONE = 0,
	Left = 1,  //!<Left arrow, Swipe left
	Right = 2, //!<Right arrow, Swipe left
	Up = 3,		//!<Up arrow, Swipe left
	Down = 4,	//!<Down arrow, Swipe left
	ActionClose = 5, //!<Esc, Q, Android back, iOS home
	Action1 = 6,	 //!<Space, Enter, Touch screen center
	Action2 = 7,	//!<Key 1, Touch screen left side
	Action3 = 8,	//!<Key 2, Touch screen right side
};

/*!*********************************************************************************************************************
\brief        Enumeration representing a System Event (quit, Gain focus, Lose focus).
***********************************************************************************************************************/
enum class SystemEvent
{
	SystemEvent_Quit, SystemEvent_LoseFocus, SystemEvent_GainFocus
};

/*!*********************************************************************************************************************
\brief        Enumeration representing a Keyboard Key.
***********************************************************************************************************************/
enum class Keys : byte
{
//Whenever possible, keys get ASCII values of their default (non-shifted) values of a default US keyboard.
	Backspace = 0x08,
	Tab = 0x09,
	Return = 0x0D,

	Shift = 0x10, Control = 0x11, Alt = 0x12,

	Pause = 0x13,
	PrintScreen = 0x2C,
	CapsLock = 0x14,
	Escape = 0x1B,
	Space = 0x20,

	PageUp = 0x21, PageDown = 0x22, End = 0x23, Home = 0x24,

	Left = 0x25, Up = 0x26, Right = 0x27, Down = 0x28,

	Insert = 0x2D, Delete = 0x2E,

	//ASCII-Based
	Key0 = 0x30, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,

	A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M,
	N = 0x4E, O, P, Q, R, S, T, U, V, W, X, Y, Z,


	Num0 = 0x60, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, NumPeriod,

	F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	SystemKey1 = 0x5B, SystemKey2 = 0x5D,
	WindowsKey = 0x5B, MenuKey = 0x5D, //ALIASES

	NumMul = 0x6A, NumAdd = 0x6B, NumSub = 0x6D, NumDiv = 0x6E,
	NumLock = 0x90, ScrollLock = 0x91,

	Semicolon = 0xBA, Equals = 0xBB, Minus = 0xBD,

	Slash = 0xBF,

	Comma = 0xBC, Period = 0xBE,

	Backquote = 0xC0,

	SquareBracketLeft = 0xDB, SquareBracketRight = 0xDD, Quote = 0xDE, Backslash = 0xDC,

	MaxNumberOfKeyCodes,
	Unknown = 0xFF
};

class Stream;
/*!*********************************************************************************************************************
\brief       Contains system/platform related classes and namespaces. Main namespace for the PowerVR Shell.
***********************************************************************************************************************/
namespace platform {

/*!****************************************************************************************************************
\brief  Contains all data of a system event.
*******************************************************************************************************************/
struct ShellEvent
{
	enum
	{
		SystemEvent,
		PointingDeviceDown,
		PointingDeviceUp,
		PointingDeviceMove,
		KeyDown,
		KeyUp
	} type;

	union
	{
		PointerLocationStore location;
		uint8 buttonIdx;
		pvr::SystemEvent systemEvent;
		Keys key;
	};
};

struct ShellData;
class ShellOS;
/*!****************************************************************************************************************
\brief        The PowerVR Shell (pvr::Shell) is the main class that the user will inherit his application from.
\description  This class abstracts the platform for the user and provides a unified interface to it. The user will
              normally write his application as a class inheriting from the Shell. This way the user can have
			  specific and easy to use places to write his code - Application start, window initialisation, per
			  frame, cleanup. All platform queries and settings can be done on the shell (set the required Graphics
			  API required, window size etc.). Specific callbacks and queries are provided for system events
			  (keyboard, mouse, touch) as well as a unified simplified input interface provided such abstracted
			  input events as "Left", "Right", "Action1", "Quit" across different platforms.
*******************************************************************************************************************/
class Shell : public IPlatformProvider
{
	friend class ShellOS;
	friend class StateMachine;
public:
	/*!****************************************************************************************************************
	\brief Contains a pointer location in normalised coordinates.
	*******************************************************************************************************************/
	struct PointerNormalisedLocation
	{
		PointerNormalisedLocation() {}
		PointerNormalisedLocation(const PointerLocation& location) { x = location.x; y = location.y; }

		float32 x; float32 y;
	};

	/*!****************************************************************************************************************
	\brief Contains the state of a pointing device (mouse, touch screen).
	*******************************************************************************************************************/
	struct PointingDeviceState
	{
	protected:
		PointerLocation pointerLocation;    //!< Location of the pointer
		PointerLocation dragStartLocation;  //!< Location of a drag starting point
		int8 buttons;                       //!< Buttons pressed
	public:
		/*!****************************************************************************************************************
		\brief Constructor.
		*******************************************************************************************************************/
		PointingDeviceState() : pointerLocation(0, 0), dragStartLocation(0, 0), buttons(0) {}
		/*!****************************************************************************************************************
		\return The location of the pointer.
		*******************************************************************************************************************/
		PointerLocation position() { return pointerLocation; }

		/*!****************************************************************************************************************
		\return The location of a drag action's starting point.
		*******************************************************************************************************************/
		PointerLocation dragStartPosition() { return dragStartLocation; }

		/*!****************************************************************************************************************
		\brief  Queries if a specific button is pressed.
		\param  buttonIndex The index of the button (0 up to 6).
		\return True if the button exists and is pressed. False otherwise.
		*******************************************************************************************************************/
		bool isPressed(int8 buttonIndex)
		{
			return (buttons & (1 << buttonIndex)) != 0;
		}
		/*!****************************************************************************************************************
		\return  True if during a dragging action.
		*******************************************************************************************************************/
		bool isDragging()
		{
			return (buttons & 0x80) != 0;
		}

	};

	/*!****************************************************************************************************************
	\brief  Internal class of the pvr::Shell.
	*******************************************************************************************************************/
	struct PrivatePointerState : public PointingDeviceState
	{
		void startDragging() { buttons |= 0x80; dragStartLocation = pointerLocation; }
		void endDragging() { buttons &= 0x7F; }

		void setButton(int8 buttonIndex, bool pressed)
		{
			buttons = (buttons & ~(1 << buttonIndex)) | (pressed << buttonIndex);
		}
		void setPointerLocation(PointerLocation pointerLocation)
		{
			this->pointerLocation = pointerLocation;
		}

	};

protected:

	/*!****************************************************************************************************************
	\brief        IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents application start.
	\description  This function must be implemented in the user's application class. It will be fired once, on start,
	              before any other callback and before  Graphics Context aquisition. It is suitable to do per-run
				  initialisation, load assets files and similar tasks.
				  A context does not exist yet, hence if the user tries to create API objects, they will
				  fail and the behaviour is undefined.
	\return       When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	              returned , the Shell will detect that, clean up, and exit.
	*******************************************************************************************************************/
	virtual Result initApplication() = 0;

	/*!****************************************************************************************************************
	\brief        IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents successful context aquisition.
	\description  This function must be implemented in the user's application class. It will be fired once after every
	              time the main Graphics Context (the one the Application Window is using) is initialized. This is
				  usually once per application run, but in some cases (context lost) it may be called more than once.
				  If the context is lost, the releaseView() callback will be fired, and if it is reaquired this function
				  will be called again.
	              This callback is suitable to do all do-once tasks that require a graphics context, such as creating
				  an On-Screen FBO, and for simple applications creating the graphics objects.
	\return       When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	              returned , the Shell will detect that, clean up, and exit.
	*******************************************************************************************************************/
	virtual Result initView() = 0;

	/*!****************************************************************************************************************
	\brief        IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents every frame.
	\description  This function must be implemented in the user's application class. It will be fired once every frame.
	              The user should use this callback as his main callback to start rendering and per-frame code.
	              This callback is suitable to do all per-frame task. In multithreaded environments, it should be used
				  to mark the start and signal the end of frames.
	\return       When implementing, return a suitable error code to signify failure. Return pvr::Success to signify
	              success and allow the Shell to do all actions necessary to render the frame (swap buffers etc.).
	              If pvr::Result::Success is not returned, the Shell will detect that, clean up, and exit.
				  Return pvr::Result::ExitRenderFrame to signify a clean, non-error exit for the application.
				  Any other error code will be logged.
	*******************************************************************************************************************/
	virtual Result renderFrame() = 0;

	/*!****************************************************************************************************************
	\brief        IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents graphics context released.
	\description  This function must be implemented in the user's application class. It will be fired once before the
	              main Graphics Context is lost.
				  The user should use this callback as his main callback to release all API objects as they will be
				  invalid afterwards. In simple applications where all objects are created in initView, it should
				  release all objects acquired in initView.
				  This callback will be called when the application is exiting, but not only then - losing (and later
				  re-acquiring) the Graphics Context will lead to this callback being fired, followed by an initView
				  callback, renderFrame etc.
	\return       When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	              returned, the Shell will detect that, clean up, and exit. If the shell was exiting, this will happen
				  anyway.
	*******************************************************************************************************************/
	virtual Result releaseView() = 0;

	/*!****************************************************************************************************************
	\brief        IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents application exit.
	\description  This function must be implemented in the user's application class. It will be fired once before the
	              application exits, after the Graphics Context is torn down.
				  The user should use this callback as his main callback to release all objects that need to. The
				  application will exit shortly after this callback is fired.
				  In effect, the user should release all objects that were acquired during initApplication.
				  Do NOT use this to release API objects - these should already have been released during releaseView.
	\return       When implementing, return a suitable error code to signify a failure that will be logged.
	*******************************************************************************************************************/
	virtual Result quitApplication() = 0;

	/*!****************************************************************************************************************
	\brief       Override in your class to handle the "Click" or "Touch" event of the main input device (mouse
	             or touchscreen).
	\description This event will be fired on releasing the button, when the mouse pointer has not moved more than a
	              few pixels since the button was pressed (otherwise a drag will register instead of a click).
	\param buttonIdx The index of the button (LMB:0, RMB:1, MMB:2, Touch:0)
	\param location The location of the click.
	******************************************************************************************************************/
	virtual void eventClick(int buttonIdx, PointerLocation location) { (void)buttonIdx; (void)location; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle the finish of a "Drag" event of the main input device (mouse, touchscreen).
	\description This event will be fired on releasing the button, and the mouse pointer has not moved more than a
	few pixels since the button was pressed.
	\param location The location of the click.
	******************************************************************************************************************/
	virtual void eventDragFinished(PointerLocation location) { (void)location; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle the start of a "Drag" event of the main input device (mouse, touchscreen).
	\description This event will be fired after a movement of more than a few pixels is detected with a button down.
	\param buttonIdx The index of the button (LMB:0, RMB:1, MMB:2, Touch:0).
	\param location The location of the click.
	******************************************************************************************************************/
	virtual void eventDragStart(int buttonIdx, PointerLocation location) { (void)location; (void)buttonIdx; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle the initial press (down) of the main input device (mouse, touchscreen).
	\description This event will be fired on pressing any button.
	\param buttonIdx The index of the button (LMB:0, RMB:1, MMB:2, Touch:0)
	******************************************************************************************************************/
	virtual void eventButtonDown(int buttonIdx) { (void)buttonIdx; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle the release (up) of the main input device (mouse, touchscreen).
	\description This event will be fired on releasing any button.
	\param buttonIdx The index of the button (LMB:0, RMB:1, MMB:2, Touch:0)
	******************************************************************************************************************/
	virtual void eventButtonUp(int buttonIdx) { (void)buttonIdx; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle the press of a key of the keyboard.
	\description This event will be fired on pressing any key.
	\param key The key pressed
	******************************************************************************************************************/
	virtual void eventKeyDown(Keys key) { (void)key; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle a keystroke of the keyboard.
	\description This event will normally be fired multiple times during a key press, as controlled by the key repeat
	             of the operating system.
	\param key The key pressed
	******************************************************************************************************************/
	virtual void eventKeyStroke(Keys key) { (void)key; }

	/*!****************************************************************************************************************
	\brief Override in your class to handle the release (up) of a key of of the keyboard.
	\description This event will be fired once, when releasing a key.
	\param key The key released
	******************************************************************************************************************/
	virtual void eventKeyUp(Keys key) { (void)key; }

	/*!****************************************************************************************************************
	\brief       Override in your class to handle a unified interface for input across different platforms and devices.
	\description This event abstracts, maps and unifies several input devices, in a way with a mind to unify several
	             platforms and input devices.
				 The Left/Right/Up/Down keyboard key, Swipe Left/Right/Up/Down both cause Left/Right/Up/Down events.
				 Left Click at Center, Space key, Enter key, Touch at Center cause Action1.
				 Left Click at Left, Right Click, One Key, Touch at the Left cause Action2.
				 Left Click at Right, Middle Click, Two Key, Touch at the Right cause Action3.
				 Escape, Q key, Back button cause Quit.
				 Default behaviour is Quit action calls exitShell. In order to retain Quit button functionality,
				 this behaviour should be mirrored (exitShell called on ActionClose).
	\param key The Simplified Unified Event
	******************************************************************************************************************/
	virtual void eventMappedInput(SimplifiedInput key)
	{
		switch (key)
		{
		case SimplifiedInput::ActionClose:
			exitShell();
			break;
		default: break;
		}
	}

public:
	/*!****************************************************************************************************************
	\brief       Used externally to signify events to the Shell. Do not use.
	******************************************************************************************************************/
	void onKeyDown(Keys key)
	{
		ShellEvent evt;
		evt.type = ShellEvent::KeyDown;
		evt.key = key;
		eventQueue.push(evt);
	}

	/*!****************************************************************************************************************
	\brief       Used externally to signify events to the Shell. Do not use.
	******************************************************************************************************************/
	void onKeyUp(Keys key)
	{
		ShellEvent evt;
		evt.type = ShellEvent::KeyUp;
		evt.key = key;
		eventQueue.push(evt);
	}

	/*!****************************************************************************************************************
	\brief       Used externally to signify events to the Shell. Do not use.
	******************************************************************************************************************/
	void onPointingDeviceDown(uint8 buttonIdx)
	{
		ShellEvent evt;
		evt.type = ShellEvent::PointingDeviceDown;
		evt.buttonIdx = buttonIdx;
		eventQueue.push(evt);
	}

	/*!****************************************************************************************************************
	\brief       Used externally to signify events to the Shell. Do not use.
	******************************************************************************************************************/
	void onPointingDeviceUp(uint8 buttonIdx)
	{
		ShellEvent evt;
		evt.type = ShellEvent::PointingDeviceUp;
		evt.buttonIdx = buttonIdx;
		eventQueue.push(evt);
	}

	/*!****************************************************************************************************************
	\brief       Used externally to signify events to the Shell. Do not use.
	******************************************************************************************************************/
	void onSystemEvent(SystemEvent systemEvent)
	{
		ShellEvent evt;
		evt.type = ShellEvent::SystemEvent;
		evt.systemEvent = systemEvent;
		eventQueue.push(evt);
	}

private:
	/*!****************************************************************************************************************
	\brief       Used externally to signify events to the Shell. Do not use.
	******************************************************************************************************************/
	void updatePointerPosition(PointerLocation location);
	void implKeyDown(Keys key);
	void implKeyUp(Keys key);
	void implPointingDeviceDown(uint8 buttonIdx);
	void implPointingDeviceUp(uint8 buttonIdx);
	void implSystemEvent(SystemEvent systemEvent);

	std::queue<ShellEvent> eventQueue;

	void processShellEvents();

public:
	/*!****************************************************************************************************************
	\brief       Default constructor. Do not instantiate a Shell class directly - extend as your application and then
	             provide the newDemo() function returning your application instance. See bottom of this file.
	******************************************************************************************************************/
	Shell();

	/*!****************************************************************************************************************
	\brief       Called at the appropriate time by the state machine.
	******************************************************************************************************************/
	Result init(ShellData* data);

	/*!****************************************************************************************************************
	\brief       Destructor.
	******************************************************************************************************************/
	virtual ~Shell();

	/*!****************************************************************************************************************
	\return       The display attributes (width, height, bpp, AA, etc) of this pvr::Shell (OSManager interface
	              implementation).
	******************************************************************************************************************/
	DisplayAttributes& getDisplayAttributes();

	/*!****************************************************************************************************************
	\return       The underlying Display object of this shell (OSManager interface implementation).
	******************************************************************************************************************/
	OSDisplay getDisplay();

	/*!****************************************************************************************************************
	\return       The underlying Window object of this shell (OSManager interface implementation).
	******************************************************************************************************************/
	OSWindow getWindow();
	/* END IMPLEMENT OSManager*/


private:
	/* called by our friend the State Machine */
	Result shellInitApplication();
	Result shellQuitApplication();
	Result shellInitView();
	Result shellReleaseView();
	Result shellRenderFrame();

public:
	/*!****************************************************************************************************************
	\brief     Query if a key is pressed.
	\param     key The key to check
	\return    True if a keyboard exists and the key is pressed
	******************************************************************************************************************/
	bool isKeyPressed(Keys key) { return isKeyPressedVal(key); }

	/*!****************************************************************************************************************
	\brief     Query if a key is pressed.
	\param     buttonIndex The number of the button to check (LMB:0, RMB:1, MMB:2)
	\return    True if a mouse/touchscreen exists and the button with this is index pressed. Simple touch is 0.
	******************************************************************************************************************/
	bool isButtonPressed(int8 buttonIndex) { return buttonIndex > 7 ? false : m_pointerState.isPressed(buttonIndex); }

	/*!****************************************************************************************************************
	\brief     Query the pointer location in pixels.
	\return    The location of the pointer in pixels.
	******************************************************************************************************************/
	PointerLocation getPointerAbsolutePosition() { return m_pointerState.position(); }

	/*!****************************************************************************************************************
	\brief     Query the pointer location in normalised coordinates (0..1).
	\return    The location of the pointer in normalised coordinates (0..1).
	******************************************************************************************************************/
	PointerNormalisedLocation getPointerNormalisedPosition()
	{
		PointerNormalisedLocation pos;
		pos.x = m_pointerState.position().x / (float)getWidth();
		pos.y = m_pointerState.position().y / (float)getHeight();
		return pos;
	}

	/*!****************************************************************************************************************
	\brief     Query the state of the pointing device (Mouse, Touchscreend).
	\return    A PointingDeviceState struct containing the state of the pointing device.
	******************************************************************************************************************/
	PointingDeviceState& getPointingDeviceState() { return m_pointerState; }
	/* End Input Handling :  Queried */

	/*!****************************************************************************************************************
	\return    The total time from an arbitrary starting point, in milliseconds.
	******************************************************************************************************************/
	uint64 getTime();

	/*!****************************************************************************************************************
	\return    The time of the current frame, in milliseconds.
	******************************************************************************************************************/
	uint64 getFrameTime();

	/*!****************************************************************************************************************
	\return    The time at init application, in milliseconds.
	******************************************************************************************************************/
	uint64 getTimeAtInitApplication() const;

	/*!****************************************************************************************************************
	\return    A pvr::platform::CommandLine::Options object containing the command line options passed at app launch.
	******************************************************************************************************************/
	const platform::CommandLineParser::ParsedCommandLine& getCommandLine() const;

	/*!****************************************************************************************************************
	\brief    ONLY EFFECTIVE AT INIT APPLICATION. Set the application to run at full screen mode. Not all
	          platforms support this option.
	******************************************************************************************************************/
	void setFullscreen(const bool fullscreen);
	/*!****************************************************************************************************************
	\return   True if the application is running in full screen. False otherwise.
	******************************************************************************************************************/
	bool isFullScreen() const;

	/*!****************************************************************************************************************
	\return   The width of the application area (window width or screen width in full screen).
	******************************************************************************************************************/
	uint32 getWidth() const;
	/*!****************************************************************************************************************
	\return   The height of the application area (window height or screen height in full screen).
	******************************************************************************************************************/
	uint32 getHeight() const;

	/*!****************************************************************************************************************
	\brief    ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the window size.
	\param    w The width of the window
	\param    h The height of the window
	\return   pvr::Result::Success if successful. pvr::Result::UnsupportedRequest if unsuccessful.
	******************************************************************************************************************/
	Result setDimensions(uint32 w, uint32 h);

	/*!****************************************************************************************************************
	\return   The window position X coordinate.
	******************************************************************************************************************/
	uint32 getPositionX() const;

	/*!****************************************************************************************************************
	\return   The window position Y coordinate.
	******************************************************************************************************************/
	uint32 getPositionY() const;

	/*!****************************************************************************************************************
	\brief    ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the window position. Not supported in all
	          platforms.
	\return   pvr::Result::Success if successful. pvr::Result::UnsupportedRequest if unsuccessful.
	******************************************************************************************************************/
	Result setPosition(uint32 x, uint32 y);

	/*!****************************************************************************************************************
	\return   The frame after which the application is set to quit.
	******************************************************************************************************************/
	int32 getQuitAfterFrame() const;
	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set a frame after which the application will quit.
	******************************************************************************************************************/
	void setQuitAfterFrame(uint32 value);

	/*!****************************************************************************************************************
	\return   The time after which the application will quit.
	******************************************************************************************************************/
	float32 getQuitAfterTime() const;
	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set a time after which the application will quit.
	******************************************************************************************************************/
	void setQuitAfterTime(float32 value);

	/*!****************************************************************************************************************
	\return   The vertical synchronisation swap interval. 0 is immediate.
	******************************************************************************************************************/
	VsyncMode getVsyncMode() const;

	/*!****************************************************************************************************************
	\return   The number of logical swap chain images. This number is always one greater than the max number returned by
	getSwapChainIndex().
	******************************************************************************************************************/
	uint32 getSwapChainLength() const;

	/*!****************************************************************************************************************
	\return   The logical backbuffer image that the application currently owns and should render to. It is undefined
			  to use an Off Screen FBO that points to any swap image other than the one with index this number.
	******************************************************************************************************************/
	uint32 getSwapChainIndex() const;

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the swap interval (vertical sync).
	\param   mode The swap interval. 0 is no interval (no vsync). 1 is default.
	******************************************************************************************************************/
	void setVsyncMode(VsyncMode mode);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set a desired number of swap images (number of framebuffers).
	This number will be clamped between the minimum and the maximum number supported by the platform, so that if a small
	(0-1) or large (8+) number is requested, the minimum/maximum of the platform will always be providedd
	\param   swapChainLength The desired number of swap images. Default is 3.
	******************************************************************************************************************/
	void setPreferredSwapChainLength(uint32 swapChainLength);

	/*!****************************************************************************************************************
	\brief   EFFECTIVE IF CALLED during RenderFrame. Force the shell to ReleaseView and then InitView again after this
			frame.
	******************************************************************************************************************/
	void forceReinitView();

	/*!****************************************************************************************************************
	\return   The number of anti-aliasing samples.
	******************************************************************************************************************/
	uint32 getAASamples() const;
	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the Anti-Aliasing samples.
	\param   value The anti-aliasing samples.
	******************************************************************************************************************/
	void setAASamples(uint32 value);

	/*!****************************************************************************************************************
	\return   The number of total color bits per pixel.
	******************************************************************************************************************/
	uint32 getColorBitsPerPixel() const;

	/*!****************************************************************************************************************
	\return   The number of depth bits per pixel.
	******************************************************************************************************************/
	uint32 getDepthBitsPerPixel() const;

	/*!****************************************************************************************************************
	\return   The number of stencil bits per pixel.
	******************************************************************************************************************/
	uint32 getStencilBitsPerPixel() const;

	/*!****************************************************************************************************************
	\return   The Colorspace of the main window backbuffer (linear RGB or sRGB).
	******************************************************************************************************************/
	types::ColorSpace getBackBufferColorspace();

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Specify the colorspace of the backbuffer. Default is a
	(linear) RGB BackBuffer. Use this to specifically request an sRGB backbuffer. Since the support of backbuffer
	colorspace is an extension in many implementations, if you use this function, you must call getBackBufferColorspace
	after initApplication (in initView) to determine the actual backBuffer colorspace that was obtained.
	******************************************************************************************************************/
	void setBackBufferColorspace(types::ColorSpace colorSpace);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the number of color bits per pixel.
	\param   r The Red channel color bits
	\param   g The Green channel color bits
	\param   b The Blue channel color bits
	\param   a The Alpha channel color bits
	******************************************************************************************************************/
	void setColorBitsPerPixel(uint32 r, uint32 g, uint32 b, uint32 a);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the number of Depth bits per pixel.
	\param   value The Depth channel bits.
	******************************************************************************************************************/
	void setDepthBitsPerPixel(uint32 value);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the number of Stencil bits per pixel.
	\param   value The Stencil channel bits.
	******************************************************************************************************************/
	void setStencilBitsPerPixel(uint32 value);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Forces frame time to always be reported as 1/60th
	         of a second.
	\param   value True to force frame time, false to use real frame time.
	******************************************************************************************************************/
	void setForceFrameTime(const bool value);
	/*!****************************************************************************************************************
	\return  True if forcing frame time.
	******************************************************************************************************************/
	bool isForcingFrameTime();

	/*!****************************************************************************************************************
	\return  True if screen is Landscape (height > width).
	******************************************************************************************************************/
	bool isScreenRotated()const;

	/*!****************************************************************************************************************
	\brief	return true if present backbuffer is enabled
	\return	true if presenting backbuffer
	*******************************************************************************************************************/
	bool isPresentingBackBuffer();

	/*!****************************************************************************************************************
	\brief	enable or disable presenting back-buffer
	\param	value
	*******************************************************************************************************************/
	void setPresentBackBuffer(const bool value);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets a specific Graphics API type (version) that the user
	wants to use. The context creation will fail if this precise version cannot be created.
	\param   contextType The context type requested.
	******************************************************************************************************************/
	void setApiTypeRequired(Api contextType);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets the minimum Graphics API type (version) that the user
	intends to use. The context creation will fail if at least this context version cannot be created. The latest
	context version supported will be created.
	\param   contextType The context type requested.
	******************************************************************************************************************/
	void setMinApiType(Api contextType);

	/*!****************************************************************************************************************
	\brief  Gets the minimum Graphics API type (version) that the user has set. See setMinApiType.
	\return  The api type and version that the user has set as the minimum acceptable.
	******************************************************************************************************************/
	Api getMinApiTypeRequired();

	/*!****************************************************************************************************************
	\brief   Gets the maximum supported Graphics API type.
	\return  The api type and maximum version that the implementation can provide.
	******************************************************************************************************************/
	Api getMaxApiLevel();

	/*!****************************************************************************************************************
	\brief   Queries if a particular Graphics API type/version is supported.
	\param   api The context type queried.
	\return   True if api is supported
	******************************************************************************************************************/
	bool isApiSupported(Api api);

	/*!****************************************************************************************************************
	\return   The context type that will be requested from PVRApi.
	******************************************************************************************************************/
	Api  getApiTypeRequired();

	/*!****************************************************************************************************************
	\return   Return the actual API level of the existing Graphics Context. If called before initView, results are
	          undefined.
	******************************************************************************************************************/
	Api  getApiType();

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets the Device Queue types that the user may
	         require (Graphics, Compute etc.).
	\param   queueType The DeviceQueueType requested.
	******************************************************************************************************************/
	void setDeviceQueueTypesRequired(DeviceQueueType queueType);

	/*!****************************************************************************************************************
	\return   The DeviceQueueTypes that have been set as required.
	******************************************************************************************************************/
	DeviceQueueType getDeviceQueueTypesRequired();

	/*!****************************************************************************************************************
	\brief   Prints out general information about this Shell (application name, sdk version, cmd line etc.
	******************************************************************************************************************/
	void showOutputInfo();

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Captures the frames between start and stop and
	         saves them as TGA screenshots.
	\param   start First frame to be captured
	\param   stop Last frame to be captured
	******************************************************************************************************************/
	void setCaptureFrames(uint32 start, uint32 stop);

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets the upscale factor of the screenshots.
	         Upscaling only.
	\param   value Upscaling of the screenshots
	******************************************************************************************************************/
	void setCaptureFrameScale(uint32 value);

	/*!****************************************************************************************************************
	\return   If capturing frames, the first frame to be captured.
	******************************************************************************************************************/
	uint32 getCaptureFrameStart() const;

	/*!****************************************************************************************************************
	\return   If capturing frames, the last frame to be captured.
	******************************************************************************************************************/
	uint32 getCaptureFrameStop() const;

	/*!****************************************************************************************************************
	\brief		Get the requested context priority.0=Low,1=Medium, 2+ = High. Initial value: High.
	\return		If supported, the priority of the main Graphics Context used by the application.
	******************************************************************************************************************/
	uint32 getContextPriority() const;

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. If supported, sets a ContextPriority that the
	         shell will attempt to use when creating the main Graphics Context used for the window. Initial value:High.
	\param   value The context priority requested. 0=Low, 1=Medium, 2+=High.
	******************************************************************************************************************/
	void setContextPriority(uint32 value);

	/*!****************************************************************************************************************
	\return  If setDesiredConfig was required, returns the desired ConfigID.
	******************************************************************************************************************/
	uint32 getDesiredConfig() const;

	/*!****************************************************************************************************************
	\brief   ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. If supported by the platform/API, sets a specific
	         Context Configuration ID to be used.
	\param   value The ConfigID that will be requested.
	******************************************************************************************************************/
	void setDesiredConfig(uint32 value);

	/*!****************************************************************************************************************
	\returns The artificial frame time that has been set. 0 means unset.
	******************************************************************************************************************/
	uint32 getFakeFrameTime() const;

	/*!****************************************************************************************************************
	\brief   Sets a time delta that will be used as frame time to increment the application clock instead of real time.
	         This number will be returned as the frame time.
	\param   value The number of milliseconds of the frame.
	******************************************************************************************************************/
	void setFakeFrameTime(uint32 value);

	/*!****************************************************************************************************************
	\return  True if FPS are being printed out.
	******************************************************************************************************************/
	bool isShowingFPS() const;

	/*!****************************************************************************************************************
	\brief  Sets if the Frames Per Second are to be output periodically.
	\param 	showFPS Set to true to output fps, false otherwise.
	******************************************************************************************************************/
	void setShowFPS(bool showFPS);

	/*!****************************************************************************************************************
	\return  An Frames-Per-Second value calculated periodically by the application.
	******************************************************************************************************************/
	float getFPS() const;

	/*!****************************************************************************************************************
	\return  The current version of the PowerVR SDK.
	******************************************************************************************************************/
	static const char8* getSDKVersion() { return PVRSDK_VERSION; }

	/*!****************************************************************************************************************
	\brief  Set a message to be displayed on application exit. Normally used to display critical error messages that
	        might be missed if displayed as just logs.
	\param  format A printf-style format string
	\param  ... Printf-style variable arguments
	******************************************************************************************************************/
	void setExitMessage(const tchar* const format, ...);

	/*!****************************************************************************************************************
	\brief  Sets the application name.
	******************************************************************************************************************/
	void setApplicationName(const tchar* const format, ...);

	/*!****************************************************************************************************************
	\brief  Sets the window title. Will only be actually displayed If used on or before initApplication.
	******************************************************************************************************************/
	void setTitle(const tchar* const format, ...);

	/*!****************************************************************************************************************
	\return  The exit message set by the user.
	******************************************************************************************************************/
	const string& getExitMessage() const;

	/*!****************************************************************************************************************
	\return  The window title.
	******************************************************************************************************************/
	const string& getTitle() const;

	/*!****************************************************************************************************************
	\return  The application name.
	******************************************************************************************************************/
	const string& getApplicationName() const;

	/*!****************************************************************************************************************
	\return  The first (default) read path. Normally, current directory.
	******************************************************************************************************************/
	const string& getDefaultReadPath() const;

	/*!****************************************************************************************************************
	\return  The a list of all the read paths that will successively be tried when looking to read a file.
	******************************************************************************************************************/
	const std::vector<string>& getReadPaths() const;

	/*!****************************************************************************************************************
	\return  The path where any files saved (screenshots, logs) will be output to.
	******************************************************************************************************************/
	const string& getWritePath() const;

	/*!****************************************************************************************************************
	\brief  Signifies the application to clean up and exit. Will go through the normal StateMachine cycle and exit
	        cleanly, exactly like returning ExitRenderFrame from RenderFrame. Will skip the next RenderFrame execution.
	******************************************************************************************************************/
	void exitShell();

	/*!****************************************************************************************************************
	\brief  Create and return a Stream object for a specific filename. Uses platform dependent lookup rules to create
	        the	stream from the filesystem or a platform-specific store (Windows resources, Android .apk assets) etc.
			Will first try the filesystem (if available) and then the built-in stores, in order to allow the user to
			easily override built-in assets.
	******************************************************************************************************************/
	Stream::ptr_type getAssetStream(const string& filename, bool logFileNotFound = true);

	/*!****************************************************************************************************************
	\return  Gets the ShellOS object owned by this shell.
	******************************************************************************************************************/
	ShellOS& getOS() const;

	/*!****************************************************************************************************************
	\return  Gets the ShellOS object owned by this shell.
	******************************************************************************************************************/
	GraphicsContext& getGraphicsContext();

	/*!****************************************************************************************************************
	\return  Gets the ShellOS object owned by this shell.
	******************************************************************************************************************/
	const GraphicsContext& getGraphicsContext() const;

	/*!****************************************************************************************************************
	\return  Gets the ShellOS object owned by this shell.
	******************************************************************************************************************/
	GraphicsContext& context() { return getGraphicsContext(); }

	/*!****************************************************************************************************************
	\return  Gets the ShellOS object owned by this shell.
	******************************************************************************************************************/
	const GraphicsContext& context() const { return getGraphicsContext(); }

	/*!****************************************************************************************************************
	\return  Gets the Platform Context class used by this shell.
	******************************************************************************************************************/
	IPlatformContext& getPlatformContext();

	/*!****************************************************************************************************************
	\return  Gets the Platform Context class used by this shell.
	******************************************************************************************************************/
	const IPlatformContext& getPlatformContext() const;

	/*!****************************************************************************************************************
	\brief  Saves a screenshot of the current display.
	******************************************************************************************************************/
	void takeScreenshot() const;

private:
	bool m_dragging;
	std::bitset<256> m_keystate;
	PrivatePointerState m_pointerState;
	ShellData* m_data;

	SimplifiedInput MapKeyToMainInput(Keys key)
	{
		switch (key)
		{
		case Keys::Space: case Keys::Return: return SimplifiedInput::Action1;  break;
		case Keys::Escape: case Keys::Q: return SimplifiedInput::ActionClose; break;
		case Keys::Key1: return SimplifiedInput::Action2; break;
		case Keys::Key2: return SimplifiedInput::Action3; break;

		case Keys::Left: return SimplifiedInput::Left; break;
		case Keys::Right: return SimplifiedInput::Right; break;
		case Keys::Up: return SimplifiedInput::Up; break;
		case Keys::Down: return SimplifiedInput::Down; break;
		default: return SimplifiedInput::NONE;
		}
	}
	bool setKeyPressedVal(Keys key, char val)
	{
		bool temp = m_keystate[(uint32)key];
		m_keystate[(uint32)key] = (val != 0);
		return temp;
	}
	bool isKeyPressedVal(Keys key)
	{
		return m_keystate[(uint32)key];
	}
	SimplifiedInput MapPointingDeviceButtonToSimpleInput(int buttonIdx)
	{
		switch (buttonIdx)
		{
		case 0: return SimplifiedInput::Action1;
		case 1: return SimplifiedInput::Action2;
		case 2: return SimplifiedInput::Action3;
		default: return SimplifiedInput::NONE;
		}
	}

};

}
using pvr::platform::Shell;
/*!****************************************************************************************************************
\brief         ---IMPLEMENT THIS FUNCTION IN YOUR MAIN CODE FILE TO POWER YOUR APPLICATION---
\description   You must return an std::auto_ptr to pvr::Shell, that will be wrapping a pointer to an instance of
               your Application class. The implementation is usually trivial.
\returns       The implementation of this function is typically a single, simple line:
               return std::auto_ptr<pvr::Shell>(new MyApplicationClass());
******************************************************************************************************************/
std::auto_ptr<pvr::Shell> newDemo();
}
