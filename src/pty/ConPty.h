#ifndef _CON_PTY_H_
#define _CON_PTY_H_

#include "Pty.h"

#include <memory>
#include <mutex>
#include <thread>
#include <windows.h>

class CConPty : public CPty
{
public:
    CConPty();
    virtual ~CConPty();

    virtual bool Create(const std::string& strCmd, const std::string& strArgs, const std::string& strWorkDir,
                        const std::string& strEnv, int16_t sRows, int16_t sCols);
    virtual void Destroy();

    virtual bool Resize(int16_t sRows, int16_t sCols);

    virtual std::string ReadAll();
    virtual int64_t Write(const std::string& strData);

private:
    bool CreatePseudoAndPipes(int16_t sRows, int16_t sCols);
    bool PrepareStartupInformation();

    void ReadThread();

private:
    std::mutex m_mu;
    std::string m_strBuffer; // todo

    HANDLE m_hInPipe;
    HANDLE m_hOutPipe;
    HPCON m_hPty;
    PROCESS_INFORMATION* m_pPI;
    STARTUPINFOEXW* m_pSI;

    std::thread m_readThread;
    bool m_bRunning;
};

#endif // _CON_PTY_H_
