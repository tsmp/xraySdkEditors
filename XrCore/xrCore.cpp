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

void PrintBuildId()
{
	constexpr int MonthsCount = 12;

	static const char* MonthId[MonthsCount] =
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	static const int DaysInMonth[MonthsCount] =
	{
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	static const int start_day = 31;	// 31
	static const int start_month = 1;	// January
	static const int start_year = 1999;	// 1999

	const char* BuildDate = __DATE__;

	int days;
	int months = 0;
	int years;

	string16 month;
	string256 buffer;
	strcpy_s(buffer, BuildDate);
	sscanf(buffer, "%s %d %d", month, &days, &years);
	month[15] = '\0';

	for (int i = 0; i < MonthsCount; i++)
	{
		if (_stricmp(MonthId[i], month))
			continue;

		months = i;
		break;
	}

	u32 build_id = (years - start_year) * 365 + days - start_day;

	for (int i = 0; i < months; ++i)
		build_id += DaysInMonth[i];

	for (int i = 0; i < start_month - 1; ++i)
		build_id -= DaysInMonth[i];

	Msg("'xrCore' build %d, %s\n", build_id, BuildDate);
}

namespace CPU
{
	extern void Detect();
};

void xrCore::InitCore(const char* AppName, LogCallback cb)
{
	xr_strcpy(ApplicationName, AppName);

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

	xr_FS = xr_new<ELocatorAPI>();
	xr_EFS = xr_new<EFS_Utils>();

	u32 flags = 0;

	if (strstr(Params, "-build"))
		flags |= ELocatorAPI::flBuildCopy;

	if (strstr(Params, "-ebuild"))
		flags |= ELocatorAPI::flBuildCopy | ELocatorAPI::flEBuildCopy;

#ifdef DEBUG
	if (strstr(Params, "-cache"))
		flags |= ELocatorAPI::flCacheFiles;
	else
		flags &= ~ELocatorAPI::flCacheFiles;
#endif // DEBUG

	flags |= ELocatorAPI::flScanAppRoot;

	if (0 != strstr(Params, "-file_activity"))
		flags |= ELocatorAPI::flDumpFileActivity;

	FS.InitFS(flags);
	PrintBuildId();
	EFS._initialize();

#ifdef DEBUG
	Msg("CRT heap 0x%08x", _get_heap_handle());
	Msg("Process heap 0x%08x", GetProcessHeap());
#endif // DEBUG

	SetLogCB(cb);
}

void xrCore::DestroyCore()
{
	FS.DestroyFS();
	EFS._destroy();
	xr_delete(xr_FS);
	xr_delete(xr_EFS);
	Memory._destroy();
}

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
