/**
 * @file ConfigFileParser.h
 * @brief 
 * @author Michael
 * @version 1.0
 * @date 2012-09-13
 */

#ifndef _CONFIGFILEPARSER_H
#define _CONFIGFILEPARSER_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>

#define DEFAULT_FIELD "DEFAULT"
#define INVALID_VALUE 0xfefefe

namespace Hippo {

class ConfigFileParser {
    public:
        /* ====================  LIFECYCLE     ======================================= */
        ConfigFileParser(); /* constructor      */
        ConfigFileParser(const ConfigFileParser &other); /* copy constructor */
        ~ConfigFileParser(); /* destructor       */

        /* ====================  ACCESSORS     ======================================= */
        int fileOpen(const std::string& fileName);

        /* ====================  MUTATORS      ======================================= */
        std::string GetFileName();
        char GetCommentFlg();
        std::string GetVarStr(const std::string& field, const std::string& name);
        int GetVarInt(const std::string& field, const std::string& name);

        void SetFileName(const std::string& value);
        void SetCommentFlg(const char& value);
        void SetVarStr(const std::string& field, const std::string& name, const std::string& value);
        void SetVarInt(const std::string& field, const std::string& name, const int& value);

        /* ====================  OPERATORS     ======================================= */
        ConfigFileParser& operator = (const ConfigFileParser& other); /* assignment operator */

        static std::string Trim(std::string str);

    protected:
        /* ====================  METHODS       ======================================= */
        void setItem(const std::string& field, const std::string& name, const std::string& value);
        std::string getItem(const std::string& field, const std::string& name); 

        /* ====================  DATA MEMBERS  ======================================= */

    private:
        /* ====================  METHODS       ======================================= */

        /* ====================  DATA MEMBERS  ======================================= */
        typedef std::vector<std::string> VStrLine;
        typedef std::map<std::string, std::string> MapItem; /* <Name,Value> */
        typedef std::map<std::string, MapItem*> MapField; /* <Name,MapItem> */
        bool m_didModify;
        std::string m_strFileName;
        char m_cCommentFlg;
        MapField m_mapField;
        
}; /* -----  end of class ConfigFileParser  ----- */


}

#endif
