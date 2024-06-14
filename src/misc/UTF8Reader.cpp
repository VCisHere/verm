#include "UTF8Reader.h"

#include <cassert>

CUTF8Reader::CUTF8Reader(const std::string& str) : m_str(str), m_cur(0)
{
}

CUTF8Reader::~CUTF8Reader()
{
}

#include <iostream>

bool CUTF8Reader::GetNext(char32_t& code)
{
    if (m_cur == m_str.size())
    {
        return false;
    }

    assert((m_str[m_cur] & 0xF8) <= 0xF0);
    int next = 1;
    unsigned char c = m_str[m_cur];
    if ((c & 0x80) == 0x00)
    {
        code = c;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        code = char32_t(c) << 8;
        unsigned char b = m_str[m_cur + 1];
        code |= char32_t(b);
        next = 2;
    }
    else if ((m_str[m_cur] & 0xF0) == 0xE0)
    {
        uint64_t a = uint8_t(m_str[m_cur]);
        char32_t b = char32_t(m_str[m_cur + 1]) << 8;
        code = uint8_t(m_str[m_cur]);
        code = (code << 8);
        code |= uint8_t(m_str[m_cur + 1]);
        code = (code << 8) | uint8_t(m_str[m_cur + 2]);
        // code = char32_t(m_str[m_cur]) << 16 | char32_t(m_str[m_cur + 1]) << 8 | m_str[m_cur + 2];
        next = 3;
    }
    else if ((m_str[m_cur] & 0xF8) == 0xF0)
    {
        code = char32_t(m_str[m_cur]) << 24 | char32_t(m_str[m_cur + 1]) << 16 | char32_t(m_str[m_cur + 2]) << 8 |
               m_str[m_cur + 3];
        next = 4;
    }
    m_cur += next;

    return true;
}
