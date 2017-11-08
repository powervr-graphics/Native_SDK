/*!
\brief A StackTrace class implementation for windows.
\file PVRCore/Windows/StackTraceWindows.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#ifdef DEBUG
#include "PVRCore/Base/StackTrace.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <Psapi.h>
#include <algorithm>
#include <iterator>


// Adapted from http://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app/6207030
// Thank you Jerry Coffin.

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack( push, before_imagehlp, 8 )
#include <imagehlp.h>
#pragma pack( pop, before_imagehlp )

namespace pvr {
namespace impl {
struct module_data
{
	std::string image_name;
	std::string module_name;
	void* base_address;
	DWORD load_size;
};

class symbol
{
	typedef IMAGEHLP_SYMBOL64 sym_type;
	enum { MAX_LENGTH = 1024 };
	union
	{
		sym_type sym;
		char _padding[sizeof(sym_type) + MAX_LENGTH];
	};

public:
	symbol(HANDLE process, DWORD64 address)
	{
		memset(&sym, '\0', sizeof(sym) + MAX_LENGTH);
		sym.SizeOfStruct = sizeof(sym);
		sym.MaxNameLength = MAX_LENGTH;
		DWORD64 displacement;

		SymGetSymFromAddr64(process, address, &displacement, &sym);
	}

	std::string name() { return std::string(sym.Name); }
	std::string undecorated_name()
	{
		if (*sym.Name == '\0')
		{
			return "<couldn't map PC to fn name>";
		}
		std::vector<char> und_name(MAX_LENGTH);
		UnDecorateSymbolName(sym.Name, &und_name[0], MAX_LENGTH, UNDNAME_COMPLETE);
		return std::string(&und_name[0], strlen(&und_name[0]));
	}
};


class get_mod_info
{
	HANDLE process;
	static const int buffer_length = 4096;
public:
	get_mod_info(HANDLE h) : process(h) {}

	module_data operator()(HMODULE module)
	{
		module_data ret;
		char temp[buffer_length];
		MODULEINFO mi;

		GetModuleInformation(process, module, &mi, sizeof(mi));
		ret.base_address = mi.lpBaseOfDll;
		ret.load_size = mi.SizeOfImage;

		GetModuleFileNameEx(process, module, temp, sizeof(temp));
		ret.image_name = temp;
		GetModuleBaseName(process, module, temp, sizeof(temp));
		ret.module_name = temp;
		std::vector<char> img(ret.image_name.begin(), ret.image_name.end());
		std::vector<char> mod(ret.module_name.begin(), ret.module_name.end());
		SymLoadModule64(process, 0, &img[0], &mod[0], (DWORD64)ret.base_address, ret.load_size);
		return ret;
	}
};
}

std::string getStackTraceInfo(int skipFrames)
{
#ifdef DEBUG
	static bool first_run = true;
	static HANDLE process = GetCurrentProcess();
	static HANDLE hThread = GetCurrentThread();
	static void* base = 0;
	static DWORD image_type = 0;

	DWORD offset_from_symbol = 0;
	IMAGEHLP_LINE64 line = { 0 };

	if (first_run)
	{
		static std::vector<impl::module_data> modules;
		static std::vector<HMODULE> module_handles(1);
		// Load the symbols:
		if (!SymInitialize(process, NULL, false))
		{
			Log(LogLevel::Debug, "Unable to initialize debug symbol handler."
			    " It will be impossible to properly trace call stacks in case of API errors in command buffers.");
		}
		else
		{
			DWORD symOptions = SymGetOptions();
			symOptions |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME;
			SymSetOptions(symOptions);
		}
		DWORD cbNeeded = 0;
		EnumProcessModules(process, &module_handles[0], (DWORD)module_handles.size() * sizeof(HMODULE), &cbNeeded);
		module_handles.resize(cbNeeded / sizeof(HMODULE));
		EnumProcessModules(process, &module_handles[0], (DWORD)module_handles.size() * sizeof(HMODULE), &cbNeeded);
		std::transform(module_handles.begin(), module_handles.end(), std::back_inserter(modules), impl::get_mod_info(process));
		base = modules[0].base_address;
		IMAGE_NT_HEADERS* h = ImageNtHeader(base);
		image_type = h->FileHeader.Machine;
		first_run = false;
	}


	CONTEXT context;
	RtlCaptureContext(&context);
	// Setup stuff:
#ifdef _M_X64
	STACKFRAME64 frame;
	frame.AddrPC.Offset = context.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
#else
	STACKFRAME64 frame;
	frame.AddrPC.Offset = context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
#endif
	line.SizeOfStruct = sizeof(line);
	int n = 0;

	// Build the std::string:
	std::ostringstream builder;
	do
	{
		if (!skipFrames)
		{
			if (frame.AddrPC.Offset != 0)
			{
				std::string fnName = impl::symbol(process, frame.AddrPC.Offset).undecorated_name();
				builder << fnName << " ";
				if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset_from_symbol, &line))
				{
					builder << "  " << line.FileName << "(" << line.LineNumber << ") ";
				}
				builder << "\n";
				if (fnName == "main" || fnName == "WinMain")
				{
					break;
				}
				if (fnName == "RaiseException")
				{
					// This is what we get when compiled in Release mode:
					return  "Your program has crashed. A 64-bit debug build is necessary to get the information of the failing calls.\n";
					;
				}
			}
			else
			{
				builder << "(No Symbols: Program Counter == 0)";
			}
		}
		else
		{
			skipFrames--;
		}
		if (!StackWalk64(image_type, process, hThread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) { break; }
		if (++n > 20) { break; }
	}
	while (frame.AddrReturn.Offset != 0);
	//return EXCEPTION_EXECUTE_HANDLER;
	//SymCleanup(process); NO CLEANUP

	// Display the std::string:
	return builder.str();
#else
	return std::string("----- Cannot get stacktrace in Release builds. ------");
#endif
}
}
#endif
//!\endcond