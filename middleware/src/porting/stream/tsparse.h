#ifndef _TS_PARSE_H_
#define  _TS_PARSE_H_

#define SUPPORT_TELETEXT
#define MAX_PROGRAM_NUMBER  32
#define MAX_AUDIO_NUM       32
#define MAX_SUBTITLE_NUM    32

typedef struct ts_pat_info_s
{
	unsigned short      i_ts_id;            /*!< transport_stream_id */
	unsigned char       i_version;          /*!< version_number */
	unsigned short      i_number[MAX_PROGRAM_NUMBER];       /*!< program_number */
	unsigned short      i_pid[MAX_PROGRAM_NUMBER];          /*!< PID of NIT/PMT */
}ts_pat_info;


#define  MAX_TAG_NUM 4
#define DSMCC_MAX_CNT 16
typedef struct dsmcc_info{
	unsigned short pid;
	unsigned short dsmccType;
	unsigned char tag_num;
	unsigned char tag[MAX_TAG_NUM];	
	unsigned char des[MAX_TAG_NUM][4];
}Dsmcc_info;
typedef struct mheg5_dscmcc_info_t{
	Dsmcc_info dsmcc_info[DSMCC_MAX_CNT];
	unsigned short dsmcc_num;
}mheg5_dsmcc_info;

typedef struct ts_pmt_info_s
{
	unsigned  short  VID;
	unsigned  short  PCRID;
	unsigned  short  AID[MAX_AUDIO_NUM];
	unsigned  short  num_of_AID;
	unsigned  short  cur_aid_index;
    
	unsigned  short  DsmccID[16];		/* for dsmcc pid */
	unsigned  short  dsmccType[16];
	unsigned  short  num_of_DsmccID;
	unsigned  short  cur_dsmccid_index;
	
	unsigned  short  SUBTITLEID[MAX_SUBTITLE_NUM];
	unsigned  char   SUBTITLE_Language[MAX_SUBTITLE_NUM][4];
	unsigned  char   SUBTITLE_type[MAX_SUBTITLE_NUM];
	unsigned  short  SUBTITLE_composition_page_id[MAX_SUBTITLE_NUM];
	unsigned  short  SUBTITLE_ancillary_page_id[MAX_SUBTITLE_NUM];
	unsigned  short  num_of_SUBTITLE;
	unsigned  short  cur_subid_index;
	
	unsigned  short audioType[MAX_AUDIO_NUM];
	unsigned char audio_language[MAX_AUDIO_NUM][4];
	unsigned  short videoType;
	unsigned int dec_specific_info_len;
	unsigned char *p_dec_specific_info;

	unsigned short num_of_ancillary;
	unsigned short ancillary_pid[8];
	unsigned char  ancillary_type[8];

	unsigned  short ECMPID;
	unsigned  short i_ca_sys_id;

#ifdef SUPPORT_TELETEXT	
	unsigned short TTX_pid;
	unsigned char TTX_num_of_page;
	unsigned char TTX_page_index;
	unsigned char TTX_page_language[64][4];
	unsigned char TTX_page_type[64];
	unsigned char TTX_page_magazine_number[64];
	unsigned char TTX_page_number[64];
#endif

}ts_pmt_info;

int TSGetParseBuffer( char** buf, int* size );
int TSFreeParseBuffer( void );
void TSInitParse(void);
int TSParsePSI(unsigned char* buf,unsigned int ts_size);
void SetTsParseDbg(int s);
int TSGetAVInfo(ts_pmt_info *p);

extern int GetTSPMTInfo(ts_pmt_info *pmt_info);
extern int GetTSPATInfo(ts_pat_info *pat_info);

#endif


