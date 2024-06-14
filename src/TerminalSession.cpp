#include "TerminalSession.h"

#include "ConPty.h"

CTerminalSession::CTerminalSession() : m_pPty(nullptr), m_bAlive(false)
{
}

CTerminalSession::~CTerminalSession()
{
}

bool CTerminalSession::Create(uint32_t dwCols, uint32_t dwRows)
{
    m_pPty = new CConPty();
    m_pPty->Create("C:\\windows\\system32\\cmd.exe", "", "", "", dwCols, dwRows);

    m_bAlive = true;

    m_readThread = std::thread(&CTerminalSession::ReadThread, this);

    return true;
}

void CTerminalSession::Destroy()
{
    m_bAlive = false;

    m_readThread.join();

    m_pPty->Destroy();
}

void CTerminalSession::SetSize(uint32_t dwCols, uint32_t dwRows)
{
    // todo
}

std::string CTerminalSession::GetScreenBuffer()
{
    return m_strScreenBuffer;
}

void CTerminalSession::Write(const std::string& str)
{
    m_pPty->Write(str);
}

void CTerminalSession::ReadThread()
{
    while (m_bAlive)
    {
        m_strScreenBuffer += m_pPty->ReadAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
