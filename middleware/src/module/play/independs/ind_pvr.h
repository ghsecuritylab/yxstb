
#ifndef __IND_PVR_H__
#define __IND_PVR_H__

#include "ind_ts.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PVR_ATTR_CHANNEL,
	PVR_ATTR_TITLE,
	PVR_ATTR_TIME,
	PVR_ATTR_LENGTH
};

enum {
	PVR_ANNOUNCE_LOST_INDEX = -7,//�����ļ���ʧ
	PVR_ANNOUNCE_LOST = -6,
	PVR_ANNOUNCE_DAMAGE = -5,
	PVR_ANNOUNCE_WRITE = -4,//���ŵ���ͬʱ��¼�ƣ��������Ż�����������
	PVR_ANNOUNCE_BEGIN = -3,//���˵�����ʼ
	PVR_ANNOUNCE_END = -2,//�������Ż�����������
	PVR_ANNOUNCE_ERROR = -1,
	PVR_ANNOUNCE_NONE = 0
};

enum {
	BREAK_TYPE_AUTO = 0,
	BREAK_TYPE_MANUAL,
};

#define PVR_PATH_LEN		256
#define PVR_TIME_MIN		5

#define PVR_INFO_SIZE		4096

typedef void* (*mutex_create_f)(void);
typedef void (*mutex_lock_f)(void* mutex, const char *filename, const int line);
typedef void (*mutex_unlock_f)(void* mutex);

typedef struct tagPvrArgument {
	unsigned int	id;//localtime
	struct ts_psi*	psi;
	int				networkid;
	int				realtime;//ʵʱ���ݣ�����IPTV��DVBS
	int				breaktype;//Ĭ�ϼ�¼�ϵ㣺 0 �Զ���¼�� 1 �ֶ���¼
	char			info_buf[PVR_INFO_SIZE];
	int				info_len;
	int				time_shift;
	unsigned int	time_bmark;//VOD��ǩ����ʹ��
	unsigned int	time_length;
	long long		byte_bmark;//VOD��ǩ����ʹ��
	long long		byte_length;
	unsigned int	byte_rate;
} PvrArgument;
typedef PvrArgument* PvrArgument_t;

struct PVRInfo {
	unsigned int id;
	int unauthorized;//δ����Ȩ�ģ�0:����¼�ƣ�1:����¼��
	int recording;//0:δ¼�ƣ�1:����¼�ƣ�2:¼�����

	unsigned int open_clk;	//recordingΪ1�˱�־�������壬��ʾ�ӿ�ʼ¼�Ƶ����ڵ�10������

	unsigned int prev_clks;//ǰһ��������10������
	unsigned int prev_date;//ǰһ������ֹͣʱ��
/*
	�����Ѻ�ʱ����㹫ʽΪ��
	if (recording == 1)
		recordsec += (prev_clks + (mid_10ms() - open_clk)) / 100;
	else
		recordsec = prev_clks / 100;
 */
	unsigned int time_len;
	unsigned int time_bmark;//VOD��ǩ����ʹ��
	unsigned int time_length;//Ŀǰδʹ��
	long long byte_len;
	long long byte_bmark;//VOD��ǩ����ʹ��
	long long byte_length;

	int networkid;//source network id
};

typedef struct __PvrFile* PvrFile_t;

struct __PvrFInfo {
	long long	key;
	long long	byte_len;
	int			time_len;
	int			reserve1;
	long long	reserve2;
};
typedef struct __PvrFInfo PvrFInfo;
typedef struct __PvrFInfo* PvrFInfo_t;

typedef struct PvrElem* PvrElem_t;
typedef struct PvrRecord* PvrRecord_t;

typedef struct tagPvrOperation {
	mutex_create_f mutex_create;
	mutex_lock_f mutex_lock;
	mutex_unlock_f mutex_unlock;

	int		(*disk_size)	(unsigned int* psize, unsigned int* pspace);
	void	(*cfg_param)	(char* rootpath, int rootsize, int* prognum);
	int		(*dir_create)	(char* path);
	void	(*dir_delete)	(char* path);
	void*	(*dir_open)		(char* path);
	int		(*dir_read)		(void* dir, char* name);
	void	(*dir_close)	(void* dir);

	int		(*file_size)	(char* path, int* filesize);
	int		(*file_truncate)(char* path, int filesize);
	int		(*file_open)	(char* name, int flags);
	int		(*file_seek)	(int fd, int offset);
	int		(*file_write)	(int fd, char* buf, int len);
	int		(*file_sync)	(int fd);
	int		(*file_read)	(int fd, char* buf, int len);
	void	(*file_close)	(int fd);
	void	(*file_delete)	(char* path);

	int		(*strm_key)		(long long* key);
	int		(*strm_write)	(int fd, char* buf, int len);
	int		(*strm_read)	(int fd, char* buf, int len);
	void	(*strm_error)	(unsigned int id);
} PvrOperation;
typedef PvrOperation* PvrOperation_t;

#define ind_pvr_init 		ind_pvr_init_v1
int		ind_pvr_init		(PvrOperation_t op);
void	ind_pvr_virtual		(int flag);

#define ind_pvr_mount 		ind_pvr_mount_v1
int		ind_pvr_mount		(unsigned int cls_clk);
#define ind_pvr_unmount 	ind_pvr_unmount_v1
void	ind_pvr_unmount		(unsigned int cls_clk);

void	ind_rec_suffix		(char* sinfo, char* smedia);

//v10 ȥ����Ա encrypt;
#define ind_pvr_rec_open ind_pvr_rec_open_v10
int		ind_pvr_rec_open	(PvrArgument_t arg, unsigned int clk);
int		ind_pvr_rec_psi		(int rec, struct ts_psi* psi);
int		ind_pvr_rec_pcr		(int rec, int pcr);
int		ind_pvr_rec_arg		(int index, PvrArgument_t arg);
int		ind_pvr_rec_rebreak	(unsigned int id);
int		ind_pvr_rec_stamp	(int index);
int		ind_pvr_rec_reopen	(unsigned int id, unsigned int clk, int* announce);
int		ind_pvr_rec_write	(int rec, char* buf, int len);
int		ind_pvr_rec_break	(int rec);
void	ind_pvr_rec_close	(int rec, int end, unsigned int cls_clk, unsigned int cls_time);

int		ind_pvr_mix_open	(int rec, PvrArgument_t arg, unsigned int clk);
void	ind_pvr_mix_close	(int rec, unsigned int id, int end, unsigned int cls_clk, unsigned int cls_date);

int		ind_pvr_fragment	(void);//��ȡ��Ƭ��С

typedef void (*PvrMsgCall)(unsigned int id, int msg);
#define ind_pvr_delete ind_pvr_delete_v1
void ind_pvr_delete(unsigned int id, PvrMsgCall call);

/*
    ����ֵ��
    0��������
    1������
    2��¼�����
 */
#define ind_pvr_exist ind_pvr_exist_v1
int ind_pvr_exist(unsigned int id);

/*
	����ֵ
	PVR_ANNOUNCE_NONE �ɹ�
	PVR_ANNOUNCE_LOST �ļ�δ�ҵ�
	PVR_ANNOUNCE_ERROR ��������
 */
int	ind_pvr_open(unsigned int time_start, PvrElem_t* ppvr);

int		ind_pvr_play	(PvrElem_t pvr, int offset, int scale);
int		ind_pvr_speed	(PvrElem_t pvr, int speed);
int		ind_pvr_read	(PvrElem_t pvr, unsigned int clk, char* buf, int len);
void	ind_pvr_close	(PvrElem_t pvr);

void	ind_pvr_set_fill(int fill);

int		ind_pvr_get_num(void);

unsigned int	ind_pvr_get_base		(unsigned int id);
/*
 sizeӦ����sizeof(struct PVRInfo)
 v2 ȥ������size
 v3 �ṹ��PVRInfo����������Ա
 */
#define 		ind_pvr_get_info		ind_pvr_get_info_v3
int				ind_pvr_get_info		(unsigned int id, struct PVRInfo* info);
#define 		ind_pvr_get_info_ex		ind_pvr_get_info_ex_v3
int				ind_pvr_get_info_ex		(int index, struct PVRInfo* info, char* info_buf, int* pinfo_len);
unsigned int	ind_pvr_get_byterate	(unsigned int id);
int				ind_pvr_get_time		(unsigned int id);

int		ind_pvr_breaktype	(unsigned int id);
int		ind_pvr_breaknum	(unsigned int id);

int		ind_pvr_user_write	(unsigned int id, char* buf, int len);
int		ind_pvr_user_read	(unsigned int id, char* buf, int size);
int		ind_pvr_user_share	(unsigned int id, int flag);

//path ��¼���ļ����޸�Ϊ¼�Ƽ�¼��Ϣ�ļ�
#define		ind_pvr_finfo	ind_pvr_finfo_v1
int			ind_pvr_finfo	(char* path, PvrFInfo_t info, char* info_buf, int info_size);
PvrFile_t	ind_pvr_fopen	(char* path);
int 		ind_pvr_fseek(PvrFile_t pf, int64_t offset);
int			ind_pvr_fread	(PvrFile_t pf, char* buf, int len);
void		ind_pvr_fclose	(PvrFile_t pf);

/*
	��ind_pvr.c���Ƴ��������ϲ�ʵ��
 */
enum {
	PVR_EVENT_NONE = 0,
	PVR_EVENT_ADD,
	PVR_EVENT_DELETE,
};
typedef void (*pvr_event_f)(unsigned int id, int event);
void	ind_pvr_event_regist(pvr_event_f func);

#ifdef __cplusplus
}
#endif

#endif//__IND_PVR_H__
