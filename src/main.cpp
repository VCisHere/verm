#include "ConPty.h"
#include "TerminalWindow.h"

#include <codecvt>
#include <iostream>

std::string UTF8ToGBK(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring tmp_wstr = conv.from_bytes(str);
    // GBK locale name in windows
    const char* GBK_LOCALE_NAME = ".936";
    std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> convert(
        new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    return convert.to_bytes(tmp_wstr);
}

class CTerminalApp : public CTerminalWindow::CWindowListener
{
public:
    virtual void OnDestroy()
    {
        m_cond.notify_one();
    }

    void Run()
    {
        CTerminalWindow TerminalWindow;
        TerminalWindow.Create(800, 600);
        TerminalWindow.RegisterListener(this);

        do
        {
            std::unique_lock<std::mutex> lock(m_mu);
            m_cond.wait(lock);
        } while (false);

        TerminalWindow.Destroy();
    }

private:
    std::mutex m_mu;
    std::condition_variable m_cond;
};

int main()
{
    CTerminalApp App;
    App.Run();
}
