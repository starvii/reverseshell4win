#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <tchar.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

inline static BOOL file_exists(PCTSTR path) {
    //HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL ret = FALSE;
    const auto hFile = CreateFile(
        path,
        GENERIC_READ,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hFile != INVALID_HANDLE_VALUE) {
        ret = TRUE;
        CloseHandle(hFile);
    }
    return ret;
}

static SIZE_T $strlen(const TCHAR* str) {
    BYTE* p = (BYTE*)str;
    SIZE_T i = 0;
    while (*p != 0) {
        i++;
        p++;
    }
    return i;
}

static int $is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

static int $strtrim(PCTSTR str, LPSTR out) {
    if (str == nullptr || out == nullptr) return -ERROR_INVALID_ADDRESS;

    // jump left spaces
    const char* start = str;
    while (*start != '\0' && $is_whitespace(*start)) {
        start++;
    }

    // all spaces or null string
    if (*start == '\0') {
        *out = _T('\0');
        return 0;
    }

    // jump right spaces
    const char* end = str + strlen(str) - 1;
    while (end > start && $is_whitespace(*end)) {
        end--;
    }

    // length of new string
    size_t length = end - start + 1;

    // malloc new 
    //char* result = malloc(length + 1); // +1 用于 '\0'
    //if (result == NULL) return NULL;

    // copy one by one
    for (size_t i = 0; i < length; i++) {
        out[i] = start[i];
    }
    out[length] = '\0';

    return 0;
}

static int $print(_In_ PCTSTR text) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD charsWritten = 0;
    if (hConsole == INVALID_HANDLE_VALUE) {
        return (INT32)hConsole;
    }
    return WriteConsole(hConsole, text, $strlen(text), &charsWritten, NULL) | FlushConsoleInputBuffer(hConsole);
}

static void $memcpy(void* dst, const void* src, size_t len) {
    BYTE* pd = (BYTE*)dst;
    const BYTE* ps = (const BYTE*)src;
    for (size_t i = 0; i < len; i++) {
        pd[i] = ps[i];
    }
}

static void _lower(LPSTR str) {
    for (auto i = 0; i < $strlen(str); i++) {
        auto ch = str[i];
        if (ch >= _T('A') && ch <= _T('Z')) {
            str[i] = ch + (_T('a') - _T('A'));
        }
    }
}

static int extract(_In_ PCTSTR cmdline, _Out_ PTSTR szCmd, _Out_ PTSTR host, _Out_ PTSTR szPort, _Out_ UINT16& port) {
    int retCode = ERROR_SUCCESS;
    const auto len = $strlen(cmdline);

    const auto buffer_length = ((len + 1) / 8 + 1) * 8;
    const auto buffer = static_cast<TCHAR*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buffer_length << 1u));
    if (nullptr == buffer) {
        retCode = ERROR_NOT_ENOUGH_MEMORY;
        goto __ERROR__;
    }
    {
        const auto tmp_buf = buffer + buffer_length;

        $strtrim(cmdline, tmp_buf);

        const auto l1 = tmp_buf[0];
        const auto r1 = tmp_buf[$strlen(tmp_buf) - 1];
        auto start_index = l1 == _T('"') || l1 == _T('\'') ? 1 : 0;
        auto end_index = r1 == _T('"') || r1 == _T('\'') ? $strlen(tmp_buf) - 1 : $strlen(tmp_buf);

        $memcpy(buffer, tmp_buf + start_index, end_index - start_index);
    }
    {
        // read file check exist
        const auto bRet = file_exists(buffer);
        if (!bRet) {
            retCode = ERROR_FILE_NOT_FOUND;
            goto __ERROR__;
        }
    }
    {
        // remove ".exe"
        const auto bl = $strlen(buffer);
        _lower(buffer);
        if (buffer[bl - 4] == _T('.') &&
            buffer[bl - 3] == _T('e') &&
            buffer[bl - 2] == _T('x') &&
            buffer[bl - 1] == _T('e')) {
            buffer[bl - 4] = _T('\0');
        }
        else {
            retCode = ERROR_DATA_NOT_ACCEPTED;
            goto __ERROR__;
        }
    }
    // extract port:
    {
        SSIZE_T hyphens[2]{ -1, -1 };
        const auto bl = $strlen(buffer);
        size_t idx = 0;
        for (auto i = 0; i < bl; i++) {
            const auto j = bl - 1 - i;
            const auto ch = buffer[j];
            if (ch == _T('-')) {
                hyphens[idx] = j;
                idx += 1;
                if (idx >= 2) {
                    break;
                }
            }
        }
        if (hyphens[0] < 7 || hyphens[1] < 0 || hyphens[0] <= hyphens[1]) {
            return ERROR_DATA_NOT_ACCEPTED;
        }
        UINT64 p = 0;
        for (auto i = hyphens[0] + 1; i < bl; i++) {
            const auto ch = buffer[i];
            if (ch < _T('0') || ch > _T('9')) {
                return ERROR_DATA_NOT_ACCEPTED;
            }
            p = p * 10 + ch - _T('0');
        }
        if (p <= 0 || p > 65535) {
            return ERROR_DATA_NOT_ACCEPTED;
        }
        port = static_cast<UINT16>(p);
        $memcpy(szPort, buffer + hyphens[0] + 1, bl - hyphens[0]);
        $memcpy(host, buffer + hyphens[1] + 1, hyphens[0] - hyphens[1] - 1);

        const auto j = hyphens[1];
        if (j >= 10 &&
            buffer[j - 10] == _T('p') &&
            buffer[j - 9] == _T('o') &&
            buffer[j - 8] == _T('w') &&
            buffer[j - 7] == _T('e') &&
            buffer[j - 6] == _T('r') &&
            buffer[j - 5] == _T('s') &&
            buffer[j - 4] == _T('h') &&
            buffer[j - 3] == _T('e') &&
            buffer[j - 2] == _T('l') &&
            buffer[j - 1] == _T('l')
            ) {
            szCmd[0] = _T('p');
            szCmd[1] = _T('o');
            szCmd[2] = _T('w');
            szCmd[3] = _T('e');
            szCmd[4] = _T('r');
            szCmd[5] = _T('s');
            szCmd[6] = _T('h');
            szCmd[7] = _T('e');
            szCmd[8] = _T('l');
            szCmd[9] = _T('l');
            szCmd[10] = _T('.');
            szCmd[11] = _T('e');
            szCmd[12] = _T('x');
            szCmd[13] = _T('e');
            szCmd[14] = _T('\0');
        }
        else {
            szCmd[0] = _T('c');
            szCmd[1] = _T('m');
            szCmd[2] = _T('d');
            szCmd[3] = _T('.');
            szCmd[4] = _T('e');
            szCmd[5] = _T('x');
            szCmd[6] = _T('e');
            szCmd[7] = _T('\0');
        }
    }
    

    goto __FREE__;
__ERROR__:
    do {} while (0);
__FREE__:
    if (nullptr != buffer) {
        HeapFree(GetProcessHeap(), 0, buffer);
    }
    return retCode;
}

int main() {
    const auto cmdline = GetCommandLine();
    printf("cmdline = %s\n", cmdline);
    auto len = $strlen(cmdline);
    printf("len = %lu, last_ascii = %hhu\n", len, cmdline[len - 1]);
 
    auto buffer = static_cast<LPSTR>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) << 2u));
    if (nullptr == buffer) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    LPSTR szHost = buffer;
    LPSTR szPort = buffer + len + 1;
    LPSTR szCmd = buffer + (len + 1) * 2;
    UINT16 u16Port = 0;

    printf("cmdline = %s\n", cmdline);
    int retCode = extract(cmdline, szCmd, szHost, szPort, u16Port);
    printf("retCode = %d\n", retCode);
    printf("cmd = %s, host = %s, port = %s %hu", szCmd, szHost, szPort, u16Port);


    //retCode |= reverse(szCmd, szHost, szPort);
    auto r1 = HeapFree(GetProcessHeap(), 0, buffer);
    retCode |= r1 ? 0 : ~0;
    return retCode;

    return 0;
}