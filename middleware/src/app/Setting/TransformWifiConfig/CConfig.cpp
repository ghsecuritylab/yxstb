
#include "CConfig.h"
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

namespace Hippo
{

CConfig::CConfig()
{
}

CConfig::~CConfig()
{
        m_map.clear();
}

bool CConfig::LoadBuffer(const char * buffer)
{
    stringstream    oss;
    oss << buffer;

    string          section;
    size_t          pos;
    string          str1, str2;

    while (std::getline(oss, str1, '\n')) {
        pos = str1.find_first_not_of(" \t");
        if(pos != string::npos)
            str1 = str1.substr(pos);

        if(str1.find_first_of("[") == 0 && str1.find_last_of("]") > 0)
        {
            pos = str1.find_first_not_of("[");
            if(pos == string::npos)
                continue;
            str1 = str1.substr(pos);
            pos = str1.find_last_of("]");
            if(pos == string::npos)
                continue;
            str1 = str1.substr(0, pos);

            section = str1;
            continue;
        }
        if(str1.find_first_of("#;") == 0)
            continue;
        pos = str1.find_first_of("=");
        if (pos == string::npos)
            continue;
        str2 = str1.substr(pos + 1);
        str1 = str1.substr(0, pos);

        pos = str2.find_first_not_of("\r\n\t ");
        if (pos != string::npos)
            str2 = str2.substr(pos);
        pos = str2.find_last_not_of("\r\n\t ");
        if (pos != string::npos)
            str2 = str2.substr(0, pos + 1);

        pos = str1.find_first_not_of("\r\n\t ");
        if (pos != string::npos)
            str1 = str1.substr(pos);
        pos = str1.find_last_not_of("\r\n\t ");
        if (pos != string::npos)
            str1 = str1.substr(0, pos + 1);

        Write(section, str1, str2);
    }
    return true;
}

bool CConfig::Load(const char * filename)
{
        m_map.clear();
        if(filename == NULL)
                return false;
        if(strlen(filename) == 0)
                return false;
        ifstream        ifr(filename);
        if(!ifr)
                return false;

        string          section;
        size_t          pos;
        string          str1, str2;

        while (getline(ifr, str1, '\n')) {
            pos = str1.find_first_not_of(" \t");
            if(pos != string::npos)
                str1 = str1.substr(pos);

            if(str1.find_first_of("[") == 0 && str1.find_last_of("]") > 0)
            {
                pos = str1.find_first_not_of("[");
                if(pos == string::npos)
                    continue;
                str1 = str1.substr(pos);
                pos = str1.find_last_of("]");
                if(pos == string::npos)
                    continue;
                str1 = str1.substr(0, pos);

                section = str1;
                continue;
            }
            if(str1.find_first_of("#;") == 0)
                continue;
            pos = str1.find_first_of("=");
            if (pos == string::npos)
                continue;
            str2 = str1.substr(pos + 1);
            str1 = str1.substr(0, pos);

            pos = str2.find_first_not_of("\r\n\t ");
            if (pos != string::npos)
                str2 = str2.substr(pos);
            pos = str2.find_last_not_of("\r\n\t ");
            if (pos != string::npos)
                str2 = str2.substr(0, pos + 1);

            pos = str1.find_first_not_of("\r\n\t ");
            if (pos != string::npos)
                str1 = str1.substr(pos);
            pos = str1.find_last_not_of("\r\n\t ");
            if (pos != string::npos)
                str1 = str1.substr(0, pos + 1);

            Write(section, str1, str2);
        }
        ifr.close();
        return true;
}

void CConfig::Save(const char * filename)
{
        map<string, map<string, string> >::iterator     iter;
        map<string, string>::iterator it;

        string  str;
        ofstream        ofn(filename);
        if(!ofn)
                return;
        iter = m_map.find("");
        if(iter != m_map.end())
        {
                for(it = (iter->second).begin(); it != (iter->second).end(); it ++)
                {
                        str = it->first + string("=") + it->second;
                        ofn << str << endl;
                }
        }

        for(iter=m_map.begin(); iter != m_map.end(); iter ++)
        {
                if((iter->first) == "")
                        continue;
                str = "[" + iter->first + "]";
                ofn << str << endl;
                for(it = (iter->second).begin(); it != (iter->second).end(); it ++)
                {
                        str = it->first + string("=") + it->second;
                        ofn << str << endl;
                }
        }
        ofn.close();
}

string CConfig::Read(string section, string name)
{
        map<string, map<string, string> >::iterator     iter;
        iter = m_map.find(section);
        if(iter == m_map.end())
                return "";

        // 不知道为什么iter->second.find()不管用了 ...
        map<string, string>::iterator it;
        for(it = (iter->second).begin(); it != (iter->second).end(); it ++)
        {
                // printf("%s = %s\n", (it->first).c_str(), (it->second).c_str());
                if(it->first == name)
                        return it->second;
        }
        return "";
#if 0
        map<string, string>::iterator it;
        it = (iter->second).find(name);
        if(it == (iter->second).end());
                return "";
        return it->second;
#endif
}

void CConfig::Write(string section, string name, string value)
{
        map<string, map<string, string> >::iterator     iter;
        iter = m_map.find(section);
        if(iter == m_map.end())
        {
                map<string, string>     m;
                m[name] = value;
                m_map[section] = m;
        } else {
                (iter->second)[name] = value;
        }
}

void CConfig::Remove(string section, string name)
{
        map<string, map<string, string> >::iterator     iter;
        iter = m_map.find(section);
        if(iter == m_map.end())
                return;
        map<string, string>::iterator it;
        it = (iter->second).find(name);
        if(it == (iter->second).end())
                return;
        (iter->second).erase(it);
        if((iter->second).size() == 0)
                m_map.erase(iter);
}

string CConfig::Read(string name)
{
        return Read("", name);
}

void CConfig::Write(string name, string value)
{
        Write("", name, value);
}

void CConfig::Remove(string name)
{
        Remove("", name);
}

void CConfig::Enum(lpfnConfigEnum proc, void * param)
{
        if(proc == NULL)
                return;
        map<string, map<string, string> >::iterator     iter;
        map<string, string>::iterator it;
        string  str;

        iter = m_map.find("");
        if(iter != m_map.end())
        {
                for(it = (iter->second).begin(); it != (iter->second).end(); it ++)
                {
                        if(!proc("", (it->first).c_str(), (it->second).c_str(), param))
                                return;
                }
        }

        for(iter=m_map.begin(); iter != m_map.end(); iter ++)
        {
                if((iter->first) == "")
                        continue;
                for(it = (iter->second).begin(); it != (iter->second).end(); it ++)
                {
                        if(!proc((iter->first).c_str(), (it->first).c_str(), (it->second).c_str(), param))
                                return;
                }
        }

}
}


