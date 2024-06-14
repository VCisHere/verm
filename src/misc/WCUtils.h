#ifndef _WC_UTILS_H_
#define _WC_UTILS_H_

#include <string>

class CWCUtils
{
public:
    static std::u32string UTF8ToUnicode(const std::string& str);
    static uint32_t GetWideCharWidth(char32_t code);
};

#endif // _WC_UTILS_H_
