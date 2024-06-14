#ifndef _TERMINAL_WINDOW_H_
#define _TERMINAL_WINDOW_H_

#include "TerminalSession.h"

#include <set>
#include <thread>

class GLFWwindow;

class CTerminalWindow : public CTerminalSession::CTerminalListener
{
public:
    class CWindowListener
    {
    public:
        virtual void OnWindowDestroy() = 0;
    };

public:
    CTerminalWindow();
    ~CTerminalWindow();

    bool Create(uint32_t dwWidth, uint32_t dwSize);
    void Destroy();

    void RegisterListener(CWindowListener* pListener);
    void UnregisterListener(CWindowListener* pListener);

private:
    static void FramebufferSizeCallback(GLFWwindow* pWindow, int nWidth, int nHeight);
    static void WindowCloseCallback(GLFWwindow* pWindow);

    virtual void OnScreenUpdate();

    void FramebufferSizeCallback(int nWidth, int nHeight);
    void WindowCloseCallback();

    void RenderThread();

private:
    GLFWwindow* m_pWindow;
    uint32_t m_dwWidth;
    uint32_t m_dwHeight;

    std::thread m_renderThread;

    std::set<CWindowListener*> m_pListenerSet;

    CTerminalSession* m_pSession;
};

#endif // _TERMINAL_WINDOW_H_
