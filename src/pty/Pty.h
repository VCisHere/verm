#ifndef _PTY_H_
#define _PTY_H_

#include <string>

class CPty
{
public:
    class CPtyListener
    {
    public:
        virtual void OnPtyData() = 0;
    };

public:
    CPty(CPtyListener* pListener);
    virtual ~CPty();

    virtual bool Create(const std::string& strCmd, const std::string& strArgs, const std::string& strWorkDir,
                        const std::string& strEnv, int16_t sRows, int16_t sCols) = 0;
    virtual void Destroy() = 0;

    virtual bool Resize(int16_t sRows, int16_t sCols) = 0;

    virtual std::string ReadAll() = 0;
    virtual int64_t Write(const std::string& strData) = 0;

protected:
    CPtyListener* m_pListener;

    int16_t m_sRows;
    int16_t m_sCols;
};

#endif // _PTY_H_
