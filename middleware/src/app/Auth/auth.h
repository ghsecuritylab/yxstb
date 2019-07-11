
#pragma once

#include <string>
#include <map>
#include "RefCnt.h"


namespace Hippo {

template <typename T>
class AutoUnrefT {
public:
    AutoUnrefT() {
        m_t = new T();
    }
    ~AutoUnrefT() {
        m_t->safeUnref();
    }
    T* Get() { return m_t; }
private:
    T* m_t;
};

class Auth {
public:
    class EdsRoller : public RefCnt {
    public:
        EdsRoller() {}
        virtual ~EdsRoller() {}
        virtual std::string Current() = 0;
        virtual void Next() = 0;
        virtual void Reset() = 0;
        virtual bool IsEnd() = 0;
    };

    class UrlChecker : public RefCnt {
    public:
        virtual bool operator()(std::string url) const = 0;
        bool operator()(const char * url) const { return operator()(std::string(url)); }
    };

    class Concatenator : public RefCnt {
    public:
        virtual std::string operator()(std::string url) const = 0;
        std::string operator()(const char * url) { return operator()(std::string(url)); }
    };


    // heler
    class EDS {
    public:
        void Reset();
        void Next();
        std::string Current();
        std::string First();
    protected:
        friend class Auth;
        EDS(EdsRoller* r, UrlChecker* uc, Concatenator* c);
        EDS();
        ~EDS();
    private:
        EdsRoller*  m_roller;
        UrlChecker* m_checker;
        Concatenator* m_concatenator;
    };


public:
    Auth(EdsRoller* r, UrlChecker * uc, Concatenator* c);
    ~Auth();

public:
    typedef enum {
        eAuth_infoUrl = 1,
        eAuth_playendUrl,
        eAuth_platformCode,
        eAuth_encryptionType,
        eAuth_epgGroupNMB,
        eAuth_userGroupNMB,
        eAuth_epgDomain,
        eAuth_epgDomainBackup,
        eAuth_epgHomeAddr
    } AuthVar;

public:
    EDS& eds() { return m_eds; }
    std::string Get(AuthVar key);
    void Set(AuthVar key, std::string val);
    void Set(AuthVar key, const char * val) { Set(key, std::string(val)); }

    // std::string ConcatenateUrl(std::string baseUrl, Concatenator& c);
    std::string AvailableEpgAuthUrl();
    std::string AvailableEpgUrl();
    std::string AvailableEpgUrlWithoutPath();

    bool CheckUrl(std::string url);
    std::string ConcatenateUrl(std::string url);

private:
    Auth();
    std::map<AuthVar, std::string>  m_AuthMap;
    UrlChecker*     m_checker;
    Concatenator*   m_concatenator;
    EDS             m_eds;
};

} // namespace Hippo


