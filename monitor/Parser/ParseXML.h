#ifndef ParseXML_H
#define ParseXML_H

#include "TinyXML.h"

#include <iostream>
#include <map>


class ParseXML {
public:
    ParseXML(std::string strFileName);
    ~ParseXML();
    std::string GetSTBPara(std::string strMT);

private:
    void InitParseXML();
    std::string m_strFileName;
    TiXmlDocument* myDocument;
    std::map<std::string, std::string> m_MapPara;
    std::map<std::string, std::string>::iterator m_MapIter;
    std::string m_strMT;
    std::string m_strSTB;
    std::string m_paraMT;
    std::string m_paraSTB;

};

#endif // ParseXML_H

