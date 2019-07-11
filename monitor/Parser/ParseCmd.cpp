
#include "ParseCmd.h"
#include "MonitorDef.h"
#include "MonitorAgent.h"

#include <string.h>
#include <stdlib.h>


/***************************************
*ParseCmd  Class
*A basic class of parse
****************************************/

ParseCmd::ParseCmd()
{
	for (int i = 0; i < 4; i++)
	{
		m_szCmdInfo[i] = NULL;
	}

}

ParseCmd::~ParseCmd()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_szCmdInfo[i] != NULL)
		{
			free(m_szCmdInfo[i]);
			m_szCmdInfo[i] = NULL;
		}
	}

}

char* ParseCmd::GetFuncCall()
{
	return m_szCmdInfo[0];
}

int ParseCmd::Parse(char *pCmd, moni_buf_t pBuf)
{
	int str_index 	= 0;
	int subcmd_len 	= 0;
	char *pToken 	= NULL;
	char *pRest 	= pBuf->buf;
	int ret = 0;

	if (pRest == NULL)
		return 0;

	//解析命令和参数
	while(1)
	{
		pToken = strchr(pRest, '^');

		if (pToken == NULL)
			break;

		subcmd_len = pToken - pRest;
		m_szCmdInfo[str_index] = (char *)malloc(sizeof(char) * (subcmd_len + 1));

		if (m_szCmdInfo[str_index] == NULL){
			return 0;
		}

		memcpy(m_szCmdInfo[str_index], pRest, subcmd_len);
		m_szCmdInfo[str_index][subcmd_len] = '\0';
		str_index++;
		pRest = pToken + 1;
	}

	m_szCmdInfo[str_index] = (char *)malloc(sizeof(char) * strlen(pRest) + 1);
	memcpy(m_szCmdInfo[str_index], pRest, strlen(pRest));
	m_szCmdInfo[str_index][strlen(pRest)] = '\0';

	//执行命令语句
	printf("\n=========EXEC start ==========\n");
	ret = Exec(pBuf);
    printf("=========EXEC end==========\n");
	return ret;
}


