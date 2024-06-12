#include "ConPty.h"

#include <codecvt>

CConPty::CConPty()
    : m_hInPipe(INVALID_HANDLE_VALUE),
      m_hOutPipe(INVALID_HANDLE_VALUE),
      m_hPty(INVALID_HANDLE_VALUE),
      m_pPI(NULL),
      m_pSI(NULL),
      m_bRunning(false)
{
}

CConPty::~CConPty()
{
}

bool CConPty::Create(const std::string& strCmd, const std::string& strArgs, const std::string& strWorkDir,
                     const std::string& strEnv, int16_t sRows, int16_t sCols)
{
    if (!CreatePseudoAndPipes(sRows, sCols))
    {
        return false;
    }

    m_bRunning = true;
    m_readThread = std::thread(&CConPty::ReadThread, this);

    if (!PrepareStartupInformation())
    {
        return false;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstrCmd = converter.from_bytes(strCmd);

    m_pPI = new PROCESS_INFORMATION;
    memset(m_pPI, 0, sizeof(PROCESS_INFORMATION));
    if (!CreateProcessW(NULL, (LPWSTR)wstrCmd.c_str(), NULL, NULL, FALSE,
                        EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &m_pSI->StartupInfo,
                        m_pPI))
    {
        return false;
    }

    return true;
}

void CConPty::Destroy()
{
    m_bRunning = false;

    if (m_pPI)
    {
        CloseHandle(m_pPI->hThread);
        CloseHandle(m_pPI->hProcess);
    }

    if (m_pSI)
    {
        DeleteProcThreadAttributeList(m_pSI->lpAttributeList);
        free(m_pSI->lpAttributeList);
    }

    if (m_hPty != INVALID_HANDLE_VALUE)
    {
        ClosePseudoConsole(m_hPty);
    }

    if (m_hInPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hInPipe);
    }

    if (m_hOutPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hOutPipe);
    }

    m_readThread.join();
}

bool CConPty::Resize(int16_t sRows, int16_t sCols)
{
    return true;
}

std::string CConPty::ReadAll()
{
    std::lock_guard<std::mutex> lock(m_mu);
    return std::move(m_strBuffer);
}

int64_t CConPty::Write(const std::string& strData)
{
    DWORD dwWrittenBytes = 0;
    WriteFile(m_hOutPipe, strData.c_str(), (DWORD)strData.size(), &dwWrittenBytes, NULL);
    return dwWrittenBytes;
}

bool CConPty::CreatePseudoAndPipes(int16_t sRows, int16_t sCols)
{
    HANDLE hPipePtyIn = INVALID_HANDLE_VALUE;
    HANDLE hPipePtyOut = INVALID_HANDLE_VALUE;

    if (!CreatePipe(&m_hInPipe, &hPipePtyOut, NULL, 0))
    {
        return false;
    }

    if (!CreatePipe(&hPipePtyIn, &m_hOutPipe, NULL, 0))
    {
        return false;
    }

    HRESULT hr = CreatePseudoConsole({sRows, sCols}, hPipePtyIn, hPipePtyOut, 0, &m_hPty);
    if (FAILED(hr))
    {
        return false;
    }

    CloseHandle(hPipePtyIn);
    CloseHandle(hPipePtyOut);

    return true;
}

bool CConPty::PrepareStartupInformation()
{
    m_pSI = new STARTUPINFOEXW;
    memset(m_pSI, 0, sizeof(STARTUPINFOEXW));
    m_pSI->StartupInfo.cb = sizeof(STARTUPINFOEXW);

    size_t bytesRequired = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    m_pSI->lpAttributeList = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(malloc(bytesRequired));

    if (!InitializeProcThreadAttributeList(m_pSI->lpAttributeList, 1, 0, &bytesRequired))
    {
        free(m_pSI->lpAttributeList);
        return false;
    }

    if (!UpdateProcThreadAttribute(m_pSI->lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, m_hPty,
                                   sizeof(m_hPty), NULL, NULL))
    {
        free(m_pSI->lpAttributeList);
        return false;
    }

    return true;
}

void CConPty::ReadThread()
{
    char szBuffer[512];
    memset(szBuffer, 0, sizeof(szBuffer));

    while (m_bRunning)
    {
        DWORD dwReadBytes = 0;
        bool bRead = ReadFile(m_hInPipe, szBuffer, sizeof(szBuffer), &dwReadBytes, NULL);
        if (!bRead)
        {
            return;
        }

        if (dwReadBytes > 0)
        {
            std::lock_guard<std::mutex> lock(m_mu);
            m_strBuffer.append(szBuffer, dwReadBytes);
        }
    }
}
