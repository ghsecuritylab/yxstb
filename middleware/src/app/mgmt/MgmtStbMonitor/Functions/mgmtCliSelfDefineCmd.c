#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

//for test for sample!!!!!
/******************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
void shellCliCmdLs(int argc, char *argv[])
{
   	DIR *dir;
	char buffer[1024] = {0};
	int len = 0;
	struct dirent *ptr;
	dir = opendir(".");
	while((ptr = readdir(dir)) != NULL){
		if('.' != ptr->d_name[0]){
		    len += sprintf(buffer + len, "%s\t", ptr->d_name);
        }
    }
	mgmtCliCmdPrint(buffer);
}

void shellCliCmdLsa(int argc, char *argv[])
{
   	DIR *dir;
	char buffer[1024] = {0};
	int len = 0;
	struct dirent *ptr;
	dir = opendir(".");
	while((ptr = readdir(dir)) != NULL)
		len += sprintf(buffer + len, "%s\t", ptr->d_name);
	mgmtCliCmdPrint(buffer);
}
void shellCliCmdLogMgmtOpen(int argc, char *argv[])
{
#ifdef INCLUDE_HMWMGMT
    hmw_mgmtRegLogCallback(NULL);
#endif
}

void shellSelfDefinedInit()
{
    mgmtCmdParamRegist("ls", (void *)shellCliCmdLs);
    mgmtCmdParamRegist("ls -a", (void *)shellCliCmdLsa);

    mgmtCmdParamRegist("logmgmtopen", (void *)shellCliCmdLogMgmtOpen);

}

#ifdef __cplusplus
}
#endif
/***************************************************************************/

