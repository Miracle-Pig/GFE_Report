#include <QApplication>

#include "DlgReport.h"

#ifdef _WIN32
#include <windows.h>
#include <Dbghelp.h>
#endif

#include <spdlog/spdlog-inl.h>

#pragma comment(lib, "dbghelp.lib")

#ifdef _WIN32
long WINAPI UE_callback(_EXCEPTION_POINTERS* excp)
{
#if !defined GFE_Release
    HANDLE lhDumpFile = CreateFileW(L"GFE_Report.dmp", GENERIC_WRITE, 0, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
    MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
    loExceptionInfo.ExceptionPointers = excp;
    loExceptionInfo.ThreadId = GetCurrentThreadId();
    loExceptionInfo.ClientPointers = TRUE;
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
    CloseHandle(lhDumpFile);
#endif
    spdlog::default_logger()->flush();
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif


int main(int argc, char *argv[])
{

#if defined _WIN32
    SetUnhandledExceptionFilter(UE_callback);      // capture unhandled exception
#endif

    QApplication a(argc, argv);
    auto dlgReport = new DlgReport();
    dlgReport->ShowReportType();
    return a.exec();
}
