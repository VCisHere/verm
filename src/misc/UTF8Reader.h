#ifndef _UTF8_READER_H_
#define _UTF8_READER_H_

#include <string>

class CUTF8Reader
{
public:
    CUTF8Reader(const std::string& str);
    ~CUTF8Reader();

    bool GetNext(char32_t& code);

private:
    std::string m_str;
    size_t m_cur;
};

#endif // _UTF8_READER_H_
