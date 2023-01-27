#include "stdafx.h"
#include <mmsystem.h>
#include <objbase.h>
#include "xrCore.h"

#ifdef DEBUG
#include <malloc.h>
#endif // DEBUG

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxerr.lib")

#ifndef _WIN64
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

XRCORE_API xrCore Core;
XRCORE_API u32 build_id;
XRCORE_API LPCSTR build_date;

namespace CPU
{
	extern void Detect();
};

void xrCore::InitCore(const char* AppName, LogCallback cb)
{
	LPCSTR fs_fname = "fs.ltx";
	xr_strcpy(ApplicationName, AppName);

#ifdef XRCORE_STATIC
	_clear87();
	_control87(_PC_53, MCW_PC);
	_control87(_RC_CHOP, MCW_RC);
	_control87(_RC_NEAR, MCW_RC);
	_control87(_MCW_EM, MCW_EM);
#endif

	// Init COM so we can use CoCreateInstance
	if (!strstr(GetCommandLine(), "-editor"))
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

	xr_strcpy(Params, sizeof(Params), GetCommandLine());
	_strlwr_s(Params, sizeof(Params));

	string_path fn, dr, di;

	// application path
	GetModuleFileName(GetModuleHandle(MODULE_NAME), fn, sizeof(fn));
	_splitpath(fn, dr, di, nullptr, nullptr);
	strconcat(sizeof(ApplicationPath), ApplicationPath, dr, di);

	GetCurrentDirectory(sizeof(WorkingPath), WorkingPath);

	// User/Comp Name
	DWORD sz_user = sizeof(UserName);
	GetUserName(UserName, &sz_user);

	DWORD sz_comp = sizeof(CompName);
	GetComputerName(CompName, &sz_comp);

	// Mathematics & PSI detection
	CPU::Detect();

	Memory._initialize(strstr(Params, "-mem_debug") ? TRUE : FALSE);
	DUMP_PHASE;

	InitLog();
	_initialize_cpu();
	rtc_initialize();

	xr_FS = xr_new<ELocatorAPI>();
	xr_EFS = xr_new<EFS_Utils>();

	u32 flags = 0;

	if (strstr(Params, "-build"))
		flags |= CLocatorAPI::flBuildCopy;

	if (strstr(Params, "-ebuild"))
		flags |= CLocatorAPI::flBuildCopy | CLocatorAPI::flEBuildCopy;

#ifdef DEBUG
	if (strstr(Params, "-cache"))
		flags |= CLocatorAPI::flCacheFiles;
	else
		flags &= ~CLocatorAPI::flCacheFiles;
#endif // DEBUG

	flags |= CLocatorAPI::flScanAppRoot;

#ifndef ELocatorAPIH
	if (0 != strstr(Params, "-file_activity"))
		flags |= CLocatorAPI::flDumpFileActivity;
#endif

	FS._initialize(flags, fs_fname);
	Msg("'%s' build %d, %s\n", "xrCore", build_id, build_date);
	EFS._initialize();

#ifdef DEBUG
	Msg("CRT heap 0x%08x", _get_heap_handle());
	Msg("Process heap 0x%08x", GetProcessHeap());
#endif // DEBUG

	SetLogCB(cb);
}

#include "compression_ppmd_stream.h"
extern compression::ppmd::stream *trained_model;

void xrCore::DestroyCore()
{
	FS._destroy();
	EFS._destroy();
	xr_delete(xr_FS);
	xr_delete(xr_EFS);

	if (trained_model)
	{
		void* buffer = trained_model->buffer();
		xr_free(buffer);
		xr_delete(trained_model);
	}

	Memory._destroy();
}

#ifndef XRCORE_STATIC

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_THREAD_ATTACH:
		timeBeginPeriod(1);
		break;

	case DLL_PROCESS_DETACH:
#ifdef USE_MEMORY_MONITOR
		memory_monitor::flush_each_time(true);
#endif // USE_MEMORY_MONITOR
		break;
	}
	return TRUE;
}
#endif // XRCORE_STATIC
