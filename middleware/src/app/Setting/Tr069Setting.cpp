
#include "Assertions.h"
#include "Tr069Setting.h"
#include "config/pathConfig.h"

#include <string>
#include <stdio.h>


namespace Hippo {

static Tr069Setting g_tr069Setting(CONFIG_FILE_DIR"/yx_config_tr069.cfg");

Tr069Setting::Tr069Setting(std::string fileName)
    : Setting(fileName)
{
}

Tr069Setting::~Tr069Setting()
{
}

int
Tr069Setting::restore(int flag)
{
    FILE *fp = NULL;
    std::string line(""), fielNameBak(""), global("<Global>\n"), parameter("<Param>\n"), cksums("<Cksums>\n")
        , useds("<Useds>\n"), notifications("<Notifications>\n"), rootTag(""), tagName("");
    std::string::size_type position;

	std::vector<SettingItem*>::iterator it;

    if(!dirty && !flag) {
        LogSafeOperDebug("%s no changed! dirty = %d, flag = %d no need to save.\n",
            m_fileName.c_str(), dirty, flag);
        return -1;
    }

    fielNameBak = m_fileName + "_bak";

    if (( fp = fopen(m_fileName.c_str(), "wb"))== NULL)  //"/root/yx_config_system.ini"
        return -1;

    mid_mutex_lock(m_mutex);
	for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it) {
        SettingItem* item = *it;
        position = item->m_name.find('.');
        rootTag = item->m_name.substr(0, position);
        tagName = item->m_name.substr(position + 1);

        if(rootTag == "Global")
			global += tagName + '=' + item->m_value + '\n';
		else if(rootTag == "Param")
	        parameter += tagName + '=' + item->m_value + '\n';
		else if(rootTag == "Useds")
			useds += tagName + '=' + item->m_value + '\n';
		else if(rootTag ==  "Notifications")
			notifications += tagName + '=' + item->m_value + '\n';
		else if(rootTag ==  "Cksums")
			cksums += tagName + '=' + item->m_value + '\n';
    }
    global = global + "</Global>\n";
	parameter = parameter + "</Param>\n";
	useds = useds + "</Useds>\n";
	notifications = notifications + "</Notifications>\n";
	cksums = cksums + "</Cksums>\n";

	line = global + parameter + useds + notifications + cksums;
	mid_mutex_unlock(m_mutex);

    fwrite(line.c_str(), 1, line.length(), fp);
	fclose(fp);
    dirty = 0;
    return 0;
}

int
Tr069Setting::load()
{
    FILE *fp = NULL;
    std::string tag(""), value(""), line(""), rootTag("");
    std::string::size_type position, start, end, flag;
    char ch;
    char valueRead[1024] = {0};

    if (( fp = fopen(m_fileName.c_str(), "r"))== NULL)
        return -1;

    while ((ch = fgetc(fp)) != (char)EOF || !line.empty()) {
        if(ch != '\n' && ch != (char)EOF) {
            line += ch;
            continue;
        }

        position = line.find('=');
        if (position == std::string::npos) {  // <Param> or </Param>
            LogSafeOperDebug("undefined content[%s]\n", line.c_str());
            start = line.find('<');
			flag = line.find('/');
            end  = line.find('>');
			if(start != std::string::npos){
                rootTag.clear();
                if(flag == std::string::npos)//eg :<Param>
                    rootTag = line.substr(1,line.length() - 2);
				else    //eg: </Param>
			        rootTag = line.substr(2,line.length() - 3);
	        }
	        line.clear();
            continue;
        }else{ //eg: URL=http://110.1.1.183:37021/acs
             tag = rootTag + '.' +  line.substr(0, position);
             value = line.substr(position + 1);
	  	}
//save it as Param.URL=http://110.1.1.183:37021/acs
        if (!get(tag.c_str(), valueRead, 1024)) {
            LogSafeOperDebug("##set [%s]:[%s] to map\n", tag.c_str(), value.c_str());
            set(tag.c_str(), value.c_str());
        } else {
            LogSafeOperDebug("##add [%s]:[%s] to map\n", tag.c_str(), value.c_str());
            add(tag.c_str(), value.c_str());
        }
        line.clear();
    }

    fclose(fp);
    return 0;

}

Tr069Setting& tr069Setting()
{
    return g_tr069Setting;
}

} // namespace Hippo

#ifdef __cplusplus
extern "C" {
#endif

int tr069SettingGetString(const char* name, char* value, int valueLen, int searchFlag)
{
    LogSafeOperDebug("Tr069SettingGetString:%s\n", name);
    return Hippo::tr069Setting().get(name, value, valueLen);
}

int tr069SettingGetInt(const char* name, int* value, int searchFlag)
{
    LogSafeOperDebug("Tr069SettingGetInt:%s\n", name);
    return Hippo::tr069Setting().get(name, value);
}

int tr069SettingSetString(const char* name, const char* value)
{
    LogSafeOperDebug("Tr069SettingSetString:%s=%s\n", name, value);
    return Hippo::tr069Setting().set(name, value);
}

int tr069SettingSetInt(const char* name, const int value)
{
    LogSafeOperDebug("Tr069SettingSetInt:%s=%d\n", name, value);
    return Hippo::tr069Setting().set(name, value);
}

int tr069SettingSave()
{
   return Hippo::tr069Setting().restore(0);
}

}
