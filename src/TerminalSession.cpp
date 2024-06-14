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
    m_pPty = std::make_unique<CConPty>(this);
    m_pPty->Create("C:\\windows\\system32\\cmd.exe", "", "", "", dwCols, dwRows);

    m_bAlive = true;

    m_readThread = std::thread(&CTerminalSession::ReadThread, this);

    return true;
}

void CTerminalSession::Destroy()
{
    m_bAlive = false;
    m_cond.notify_one();

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

void CTerminalSession::OnPtyData()
{
    m_cond.notify_one();
}

void CTerminalSession::ReadThread()
{
    while (m_bAlive)
    {
        std::unique_lock<std::mutex> lock(m_mu);
        m_cond.wait(lock);

        m_strScreenBuffer += m_pPty->ReadAll();
    }
}
