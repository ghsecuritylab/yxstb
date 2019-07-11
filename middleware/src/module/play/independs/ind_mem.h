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
	使用说明：

	1. 替换文件中的内存函数，
		malloc -> IND_MALLOC
		calloc -> IND_CALLOC
		strdup -> IND_STRDUP
		realloc -> IND_REALLOC
		free -> IND_FREE
		memcpy -> IND_MEMCPY
		memset -> IND_MEMSET
		strcpy -> IND_STRCPY
		strncpy -> IND_STRNCPY
		未修改这个头文件时，IND_MALLOC就是malloc，所以替换后实际并未对源代码做改变

	2. 做内存检测长稳时，还需做如下修改
	2.1 把这个头文件中 “#if 1” 修改为 “#if 0”
	2.2 在main函数开头调用
		ind_mem_init(0);
	2.3 在mid_middle_init( );调用后面添加
		mid_timer_create(5, 0, mem_check_timer, 0);
		并添加一个函数
		static void mem_check_timer(int arg)
		{
			static n = 0;
			ind_mem_chk( );//每隔5秒对每一个由IND_MALLOC申请内存做越界检查
			n ++;
			if (n >= 12)
				ind_mem_print(20);//每隔一分钟，打印最近申请的20处内存申请情况
		}
		注意：非华为项目没有mid_timer_create，要用其他定时器函数替换

	3 正常产品中要将2所修改的进行恢复。

 ******************************************************************************/

typedef void* (*ind_malloc_f)(int size, char *file, int line);
typedef void* (*ind_realloc_f)(void* ptr, int size, char* file, int line);
typedef void (*ind_free_f)(void *ptr, char *file, int line);

/*
	内存分配、释放和检测发生错误会打印出错信息，并死机
	如下测试代码：
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
	在应用程序启动开始时执行一次ind_mem_init就启动内存检测
	需要把代码中的malloc修改为IND_MALLOC，calloc修改为IND_CALLOC，free修改为IND_FREE
	不调用该函数时，_ind_malloc、_ind_free会自动使用默认的malloc、free
	type: 0 只打印错误，1 打印申请释放
 */
void ind_mem_init(int type);
/*
	内存检测函数，最好是用定时期周期调用
	每次调用ind_mem_chk会生成含 "ind_mem_chk:225 SIZE = " 的一次打印，后面的数字是申请内存的总数
 */
void ind_mem_chk(void);

/*
	打印内存分配点、分配次数以及最后分配时间
	num：最多打印分配点个数
 */
void ind_mem_print(int num);

typedef void (*ind_show_t)(int index, void* ptr);
int ind_mem_show(char *file, int line, ind_show_t show);

void* _ind_malloc(int size, char *file, int line);
/*
	动态大量申请，循环检测是不检测
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

