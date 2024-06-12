#include "ConPty.h"

#include <cassert>
#include <codecvt>
#include <iostream>
#include <thread>

std::string UTF8ToGBK(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring tmp_wstr = conv.from_bytes(str);
    // GBK locale name in windows
    const char* GBK_LOCALE_NAME = ".936";
    std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> convert(
        new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    return convert.to_bytes(tmp_wstr);
}

int main()
{
    CPty* pPty = new CConPty();

    pPty->Create("C:\\windows\\system32\\cmd.exe", "", "", "", 80, 25);

    Sleep(1000);

    int64_t cnt = pPty->Write("echo hello\r\n");

    Sleep(1000);

    std::string strBuffer = pPty->ReadAll();

    // std::cout << strBuffer << std::endl;
    std::cout << UTF8ToGBK(strBuffer) << std::endl;

    pPty->Destroy();
}
