#ifndef _TERMINAL_SESSION_H_
#define _TERMINAL_SESSION_H_

#include <string>
#include <thread>

class CPty;

class CTerminalSession
{
public:
    class CTerminalListener
    {
        virtual void OnScreenUpdate() = 0;
    };

public:
    CTerminalSession();
    ~CTerminalSession();

    bool Create(uint32_t dwCols, uint32_t dwRows);
    void Destroy();

    void SetSize(uint32_t dwCols, uint32_t dwRows);

    std::string GetScreenBuffer();
    void Write(const std::string& str);

private:
    void ReadThread();

private:
    CPty* m_pPty;

    bool m_bAlive;

    std::thread m_readThread;

    // todo
    std::string m_strScreenBuffer;
};

#endif // _TERMINAL_SESSION_H_
