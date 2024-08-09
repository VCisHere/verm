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
    if (m_cur >= m_str.size())
    {
        return false;
    }

    assert((m_str[m_cur] & 0xF8) <= 0xF0);

    if ((m_str[m_cur] & 0x80) == 0x00)
    {
        code = uint8_t(m_str[m_cur]);
        m_cur += 1;
    }
    else if ((m_str[m_cur] & 0xE0) == 0xC0)
    {
        code = uint8_t(m_str[m_cur]);
        code <<= 8;
        code |= uint8_t(m_str[m_cur + 1]);
        m_cur += 2;
    }
    else if ((m_str[m_cur] & 0xF0) == 0xE0)
    {
        code = uint8_t(m_str[m_cur]);
        code <<= 8;
        code |= uint8_t(m_str[m_cur + 1]);
        code <<= 8;
        code |= uint8_t(m_str[m_cur + 2]);
        m_cur += 3;
    }
    else if ((m_str[m_cur] & 0xF8) == 0xF0)
    {
        code = uint8_t(m_str[m_cur]);
        code <<= 8;
        code |= uint8_t(m_str[m_cur + 1]);
        code <<= 8;
        code |= uint8_t(m_str[m_cur + 2]);
        code <<= 8;
        code |= uint8_t(m_str[m_cur + 3]);
        m_cur += 4;
    }
    else
    {
        return false;
    }

    return true;
}
