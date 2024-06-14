#ifndef _TERMINAL_SESSION_H_
#define _TERMINAL_SESSION_H_

#include "Pty.h"

#include <mutex>
#include <string>
#include <thread>

class CTerminalSession : public CPty::CPtyListener
{
public:
    class CTerminalListener
    {
        virtual void OnScreenUpdate() = 0;
    };

public:
    CTerminalSession();
    virtual ~CTerminalSession();

    bool Create(uint32_t dwCols, uint32_t dwRows);
    void Destroy();

    void SetSize(uint32_t dwCols, uint32_t dwRows);

    std::string GetScreenBuffer();
    void Write(const std::string& str);

private:
    virtual void OnPtyData();
    void ReadThread();

private:
    std::unique_ptr<CPty> m_pPty;

    bool m_bAlive;

    std::mutex m_mu;
    std::condition_variable m_cond;
    std::thread m_readThread;

    // todo
    std::string m_strScreenBuffer;
};

#endif // _TERMINAL_SESSION_H_
