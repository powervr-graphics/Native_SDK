/*!*********************************************************************************************************************
\File         MainWindows.cpp
\Title        Main Windows
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a Windows platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"


#ifdef VK_USE_PLATFORM_WIN32_KHR

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		return 0;
	case WM_SIZE:
		return 0;
	default:
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void createWin32WIndowSurface(VulkanHelloAPI& VulkanExample)
{
	VulkanExample.surfaceData.width  = 800.0f;
	VulkanExample.surfaceData.height = 600.0f;

	WNDCLASSEX win_class;
	VulkanExample.surfaceData.connection = GetModuleHandle(NULL);

	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = VulkanExample.surfaceData.connection;
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = "Vulkan Hello API Sample";
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&win_class))
	{
		Log(true, "Unexpected Error - WIN32 Window Class creation Failed \n");
		exit(1);
	}

	RECT WndRect = { 0, 0, (LONG)VulkanExample.surfaceData.width, (LONG)VulkanExample.surfaceData.height };
	AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, FALSE);
	VulkanExample.surfaceData.window = CreateWindowEx(0, win_class.lpszClassName, win_class.lpszClassName, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
	                                   100, 100, WndRect.right - WndRect.left, WndRect.bottom - WndRect.top,
	                                   NULL, NULL, VulkanExample.surfaceData.connection, NULL);
	if (!VulkanExample.surfaceData.window)
	{
		Log(true, "Unexpected Error - WIN32 Window creation Failed \n");
		exit(1);
	}
}

static void destroyWin32WindowSurface(VulkanHelloAPI& VulkanExample)
{
	DestroyWindow(VulkanExample.surfaceData.window);
	PostQuitMessage(0);
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	VulkanHelloAPI VulkanExample;
	createWin32WIndowSurface(VulkanExample);
	VulkanExample.initialize();
	VulkanExample.recordCommandBuffer();

	MSG msg;
	BOOL msgRes;

	while ((msgRes = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (msgRes != -1)
		{
			VulkanExample.drawFrame();

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;

	destroyWin32WindowSurface(VulkanExample);

	return 0;
}

#endif