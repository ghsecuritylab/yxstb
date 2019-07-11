#ifndef __IND_MEM_H__
#define __IND_MEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
	Copyright (c) 2011-2012, Hybroad Vision Electronic Technology Corp
	All Rights Reserved
	Confidential Property of Hybroad Vision Electronic Technology Corp

	Revision History:

	Created: 2012-6-27 14:09:05 by liujianhua

 ******************************************************************************/
/*******************************************************************************
	ʹ��˵����

	1. �滻�ļ��е��ڴ溯����
		malloc -> IND_MALLOC
		calloc -> IND_CALLOC
		strdup -> IND_STRDUP
		realloc -> IND_REALLOC
		free -> IND_FREE
		memcpy -> IND_MEMCPY
		memset -> IND_MEMSET
		strcpy -> IND_STRCPY
		strncpy -> IND_STRNCPY
		δ�޸����ͷ�ļ�ʱ��IND_MALLOC����malloc�������滻��ʵ�ʲ�δ��Դ�������ı�

	2. ���ڴ��ⳤ��ʱ�������������޸�
	2.1 �����ͷ�ļ��� ��#if 1�� �޸�Ϊ ��#if 0��
	2.2 ��main������ͷ����
		ind_mem_init(0);
	2.3 ��mid_middle_init( );���ú������
		mid_timer_create(5, 0, mem_check_timer, 0);
		�����һ������
		static void mem_check_timer(int arg)
		{
			static n = 0;
			ind_mem_chk( );//ÿ��5���ÿһ����IND_MALLOC�����ڴ���Խ����
			n ++;
			if (n >= 12)
				ind_mem_print(20);//ÿ��һ���ӣ���ӡ��������20���ڴ��������
		}
		ע�⣺�ǻ�Ϊ��Ŀû��mid_timer_create��Ҫ��������ʱ�������滻

	3 ������Ʒ��Ҫ��2���޸ĵĽ��лָ���

 ******************************************************************************/

typedef void* (*ind_malloc_f)(int size, char *file, int line);
typedef void* (*ind_realloc_f)(void* ptr, int size, char* file, int line);
typedef void (*ind_free_f)(void *ptr, char *file, int line);

/*
	�ڴ���䡢�ͷźͼ�ⷢ��������ӡ������Ϣ��������
	���²��Դ��룺
		ind_mem_chk( );
		p = IND_MALLOC(4);
		ind_mem_chk( );
		p[4] = 0;
		ind_mem_chk( );
	
------------------ ind_mem_chk:225 SIZE = 7500783
------------------ int_chk_malloc:126 ptr = 0xeb71bc, size = 4, index = 327 app_shell.c:94
------------------ ind_mem_chk:225 SIZE = 7500787
------------------ ind_mem_chk:225 SIZE = 7500787
------------------ int_chk_check:46 ERROR! ptr = 0xeb71bc index = 256
------------------ ind_mem_chk:229 ERROR! ptr = 0xeb71bc
 */
void ind_mem_ctrl(ind_malloc_f _malloc, ind_realloc_f _realloc, ind_free_f _free);

/*
	��Ӧ�ó���������ʼʱִ��һ��ind_mem_init�������ڴ���
	��Ҫ�Ѵ����е�malloc�޸�ΪIND_MALLOC��calloc�޸�ΪIND_CALLOC��free�޸�ΪIND_FREE
	�����øú���ʱ��_ind_malloc��_ind_free���Զ�ʹ��Ĭ�ϵ�malloc��free
	type: 0 ֻ��ӡ����1 ��ӡ�����ͷ�
 */
void ind_mem_init(int type);
/*
	�ڴ��⺯����������ö�ʱ�����ڵ���
	ÿ�ε���ind_mem_chk�����ɺ� "ind_mem_chk:225 SIZE = " ��һ�δ�ӡ������������������ڴ������
 */
void ind_mem_chk(void);

/*
	��ӡ�ڴ����㡢��������Լ�������ʱ��
	num������ӡ��������
 */
void ind_mem_print(int num);

typedef void (*ind_show_t)(int index, void* ptr);
int ind_mem_show(char *file, int line, ind_show_t show);

void* _ind_malloc(int size, char *file, int line);
/*
	��̬�������룬ѭ������ǲ����
 */
void *_ind_dalloc(int size, char *file, int line);
void* _ind_calloc(int size, char *file, int line);
char* _ind_strdup(const char* str, char *file, int line);
void* _ind_realloc(void* ptr, int size, char *file, int line);
void _ind_free(void *ptr, char *file, int line);

void* _ind_memcpy(void *dest, const void *src, int n, char *file, int line);
void* _ind_memset(void *s, int ch, int n, char *file, int line);
char* _ind_strcpy(char *dest, const char *src, char *file, int line);
char* _ind_strncpy(char* dest, const char* src, int n, char *file, int line);

#if 0
#define IND_MALLOC(size)			malloc(size)
#define IND_DALLOC(size)			malloc(size)
#define IND_CALLOC(size, n)			calloc(size, n)
#define IND_STRDUP(ptr)				strdup(ptr)
#define IND_REALLOC(ptr, size)		realloc(ptr, size)
#define IND_FREE(ptr)				free(ptr)
#define IND_MEMCPY(dest,src, n)		memcpy(dest,src, n)
#define IND_MEMSET(s, ch, n)		memset(s, ch, n)
#define IND_STRCPY(dest, src)		strcpy(dest, src)
#define IND_STRNCPY(dest, src, n)	strncpy(dest, src, n)
#else
#define IND_MALLOC(size)			_ind_malloc(size, __FILE__, __LINE__)
#define IND_DALLOC(size)			_ind_dalloc(size, __FILE__, __LINE__)
#define IND_CALLOC(size, n)			_ind_calloc((size) * (n), __FILE__, __LINE__)
#define IND_STRDUP(ptr)				_ind_strdup(ptr, __FILE__, __LINE__)
#define IND_REALLOC(ptr, size)		_ind_realloc(ptr, size, __FILE__, __LINE__)
#define IND_FREE(ptr)				_ind_free(ptr, __FILE__, __LINE__)
#define IND_MEMCPY(dest,src, n)		_ind_memcpy(dest,src, n, __FILE__, __LINE__)
#define IND_MEMSET(s, ch, n)		_ind_memset(s, ch, n, __FILE__, __LINE__)
#define IND_STRCPY(dest, src)		_ind_strcpy(dest, src, __FILE__, __LINE__)
#define IND_STRNCPY(dest, src, n)	_ind_strncpy(dest, src, n, __FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif//__IND_MEM_H__

