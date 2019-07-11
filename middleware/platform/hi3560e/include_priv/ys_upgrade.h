
#ifndef _YSUPGRADE_H_
#define _YSUPGRADE_H_

#define YAFFS 1

typedef struct
{
	short video_system;
	short width;
	short height;
	short color_mode;
	short compress_mode;
	short progress_bar_x;
	short progress_bar_y;
	short progress_bar_width;
	short progress_bar_height;
	short progress_bar_style;
	unsigned int progress_bar_color;
	int   osd_size;
	int   osd2_size;
}OSD_INFO;

typedef struct
{
	int flag;
	char name[12];
	unsigned int address;
	unsigned int size;
	int major_version;
	int mino_version;
	int svn_version;
}YS_PARTITION_INFO;

typedef struct _tag_YS_CFG_INFO {
	unsigned int checksum;
	char product_id[12];
	unsigned int pcb_version;
	unsigned int chip_version;
	char product_serial_code[16];
	unsigned int net_mac_base_addr;
	unsigned char net_mac_addr[8];
	unsigned int user_size;
	unsigned int osmem_size;
	unsigned int reserve;
	OSD_INFO osd_info;
	char reserve2[32];
	unsigned int checksumPatition;
	int partition_num;
	YS_PARTITION_INFO fat[10];
}YS_CFG_INFO, *YS_CFG_INFO_T;

typedef struct _tag_UP_HEADER_INFO{
	char product_id[12];
	unsigned int checksum;
	unsigned int pcb_version;
	unsigned int chip_version;
	int major_version;
	int minor_version;
	int svn_version;
	int force;
	int compressmode;
	unsigned int upgradesize;
	unsigned int datasize;
	char reserve[8];
	unsigned int filesystem;
	char target[16];
	unsigned char MD5[16];
	unsigned char copyright[160];
}UP_HEADER_INFO, *UP_HEADER_INFO_T;

enum {
	UBOOT=0,
	NVRAM,
	RESERVE,
	CONFIG0,
	CONFIG1,
	KERNEL0,
	KERNEL1,
	ROOT0,
	ROOT1,
	ROOTHOME
};

enum {
	LZMA=0,
	RAW,
	GZIP,
	ZLIB
};

enum {
	DEVICE_IDLE,
	DEVICE_BUSY,
	DEVICE_NOTEXIST
};

typedef struct upgrade_info{
	int target;
	int badblock_counter;
	int where;
}FACTORY_UPGRADE_INFO, *FACTORY_UPGRADE_INFO_T;

typedef void (* UPGRADE_GUAGE_FUN_T )( int );

/*
	�������ܣ�������ں�����
	����˵����
		filename	�����ļ�·����
		version		�����ļ��İ汾��(�����svn�İ汾��)
		func		�����ٷְ���ʾ����(�ص�����)
		pInfo(���)	������Ϣ��������
	����ֵ��
		int		�����ɹ����� 0��ʧ�ܷ��� ����ֵ
	����˵����Ϊ������������ʱ��ʾ�������һЩ��Ҫ��flash��Ϣ,Ŀǰ�ú�����û��ʵ��
*/
int __firmware_burn_ex( const char* filename, int version, UPGRADE_GUAGE_FUN_T func, FACTORY_UPGRADE_INFO_T pInfo );

/*
	�������ܣ�������ں���
	����˵����
		filename	�����ļ�·����
		version		�����ļ��İ汾��(�����svn�İ汾��)
		func		�����ٷְ���ʾ����(�ص�����)
	����ֵ��
		int		�����ɹ����� 0��ʧ�ܷ��� ����ֵ
	����˵����
*/
//int firmware_burn_ex( const char* filename, int version, UPGRADE_GUAGE_FUN_T func );

#endif // _YSUPGRADE_H_
