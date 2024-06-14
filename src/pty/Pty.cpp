#include "Pty.h"

CPty::CPty(CPtyListener* pListener) : m_pListener(pListener), m_sRows(0), m_sCols(0)
{
}

CPty::~CPty()
{
}
