/**
 * @file ConfigFileParser.cpp
 * @brief 
 * @author Michael
 * @version 1.0
 * @date 2012-09-13
 */

#include "ConfigFileParser.h"
#include <fstream>
#include <string.h>

#define LINESIZE 4096
#define _FSTREAM_SWITCH_TO_FOPEN //Android <fstream> libstlport.so have one bug; will delete when libstlport.so is ok or remove 'else'

using namespace std;
namespace Hippo {

/**
 * @brief ConfigFileParser constructor
 */
ConfigFileParser::ConfigFileParser()
    : m_cCommentFlg(';')
    , m_strFileName("")
    , m_didModify(false)
{
    m_mapField[DEFAULT_FIELD] = new MapItem;
}

/**
 * @brief ConfigFileParser copy constructor
 *
 * @param other another class object
 */
ConfigFileParser::ConfigFileParser(const ConfigFileParser& other)
    : m_cCommentFlg(';')
    , m_strFileName("")
    , m_didModify(false)
{

}

/**
 * @brief ~ConfigFileParser destructor
 */
ConfigFileParser::~ConfigFileParser()
{
#ifdef _FSTREAM_SWITCH_TO_FOPEN
    string tStr("");
    if (m_didModify) {
        FILE* outFile = fopen(m_strFileName.c_str(), "wb");
        if (outFile) {
            for (MapField::iterator itField = m_mapField.begin(); itField != m_mapField.end(); ++itField) {
                tStr = "[" + itField->first + "]" + "\n";
                fwrite(tStr.c_str(), 1, tStr.length(), outFile);
                for(MapItem::iterator itItem = (*itField->second).begin(); itItem != (*itField->second).end(); ++itItem) {
                    tStr = itItem->first + " = " + itItem->second + "\n";
                    fwrite(tStr.c_str(), 1, tStr.length(), outFile);
                }
                fwrite("\n", 1, 1, outFile);
            }
            fflush(outFile);
            fclose(outFile);
        }
    }
#else
    if (m_didModify) {
        ofstream outFile(m_strFileName.c_str());
        for (MapField::iterator itField = m_mapField.begin(); itField != m_mapField.end(); ++itField) {
            outFile << "[" << itField->first << "]" << endl;
            for(MapItem::iterator itItem = (*itField->second).begin(); itItem != (*itField->second).end(); ++itItem)
              outFile << itItem->first << " = " << itItem->second << endl;
            outFile << endl;
        }
        outFile.close();
    }
#endif

    for (MapField::iterator it = m_mapField.begin(); it != m_mapField.end(); ++it)
      delete it->second;
	m_mapField.clear();
}

/**
 * @brief operator = assignment operator
 *
 * @param other '=' right class object
 *
 * @return this
 */
ConfigFileParser& 
ConfigFileParser::operator = (const ConfigFileParser& other)
{
}

/**
 * @brief fileOpen open configuration file
 *
 * @param fileName configuration file name
 *
 * @return -1 on error , 0 is ok
 */
int 
ConfigFileParser::fileOpen(const string& fileName)
{
    MapField::iterator itField;
    VStrLine::iterator itLine;
    VStrLine vStrLine;

#ifdef _FSTREAM_SWITCH_TO_FOPEN
    FILE* inFile = fopen(fileName.c_str(), "rb");
    if (!inFile)
        return -1;
    SetFileName(fileName);
    char* ltok = NULL;
    char line[LINESIZE + 1] = "";
    while (fgets(line, LINESIZE, inFile)) {
        ltok = strrchr(line, '\r'); 
        if (!ltok)
            ltok = strrchr(line, '\n'); 
        if (ltok)
            *ltok = 0;
        vStrLine.push_back(Trim(line));
    }
    fclose(inFile);
#else
	ifstream inFile(fileName.c_str());
	if(!inFile.is_open())
		return -1;
    SetFileName(fileName);
    string line = "";
    while (getline(inFile, line)) 
      vStrLine.push_back(Trim(line));
	inFile.close();
#endif

	unsigned int eqFind = 0;
	string field = DEFAULT_FIELD;
	for (VStrLine::iterator it = vStrLine.begin(); it != vStrLine.end(); ++it) {
		if ((*it)[0] == m_cCommentFlg) 
          continue;
		else if (('[' == (*it)[0]) && (']' == (*it)[it->length() - 1])) {
			field = it->substr(1, it->length() - 2);
			if (m_mapField.find(field) == m_mapField.end())
              m_mapField[field] = new MapItem;
            else
              continue;
        } else {
			eqFind = it->find('=');
			if (string::npos == eqFind || field.length() <= 0)
				continue;
			else {
                itField = m_mapField.find(field);
				if (itField != m_mapField.end()) 
                  (*itField->second)[Trim(it->substr(0, eqFind))] = Trim(it->substr(eqFind + 1, it->length()));
				else
                  continue;
			}
		}
	}
	return 0;
}

/**
 * @brief Trim get rid of blank space of one string's head or tail
 *
 * @param str string 
 *
 * @return string
 */
string 
ConfigFileParser::Trim(string str)
{
	if (str.empty()) 
      return str;
	str.erase(0,str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}

/**
 * @brief getItem 
 *
 * @param field 
 * @param name 
 *
 * @return 
 */
string 
ConfigFileParser::getItem(const string& field, const string& name) 
{
    return (m_mapField.find(field) == m_mapField.end()) 
        ? "" 
        : ((m_mapField[field]->find(name) == m_mapField[field]->end()) 
                    ? "" 
                    : (*(m_mapField[field]))[name]);
}

/**
 * @brief setItem 
 *
 * @param field 
 * @param name 
 * @param value 
 */
void 
ConfigFileParser::setItem(const string& field, const string& name, const string& value)
{
    if (m_mapField.find(field) == m_mapField.end())
      m_mapField[field] = new MapItem;
    (*(m_mapField[field]))[name] = value;
	m_didModify = true;
}

string 
ConfigFileParser::GetFileName()
{
    return m_strFileName;
}

char 
ConfigFileParser::GetCommentFlg()
{
    return m_cCommentFlg;
}

string 
ConfigFileParser::GetVarStr(const std::string& field, const std::string& name)
{
    return getItem(field, name);
}

int 
ConfigFileParser::GetVarInt(const std::string& field, const std::string& name)
{
    string str = getItem(field, name);
    return str.empty() ? INVALID_VALUE : atoi(str.c_str());
}

void 
ConfigFileParser::SetFileName(const std::string& value)
{
    m_strFileName = value;
}

void 
ConfigFileParser::SetCommentFlg(const char& value)
{
    m_cCommentFlg = value;
}

void 
ConfigFileParser::SetVarStr(const std::string& field, const std::string& name, const std::string& value)
{
    return setItem(field, name, value);    
}

void 
ConfigFileParser::SetVarInt(const std::string& field, const std::string& name, const int& value)
{
    char toStr[32] = { 0 };
    snprintf(toStr, 31, "%d", value);
    return setItem(field, name, toStr);
}

}
