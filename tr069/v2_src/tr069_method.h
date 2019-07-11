/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			tr069 实现
 *******************************************************************************/

#define MAX_DELAY_SECONDS		(15 * 24 * 3600)

enum
{
	TRANSFER_STATE_INIT = 0,
	TRANSFER_STATE_START,
	TRANSFER_STATE_COMPLETE
};

enum
{
	REBOOT_STATE_NULL = 0,
	REBOOT_STATE_START,
	REBOOT_STATE_COMPLETE
};

enum
{
	FILETYPE_DOWNLOAD_FIREWARE = 0,
	FILETYPE_DOWNLOAD_WEB,
	FILETYPE_DOWNLOAD_AD,
	FILETYPE_DOWNLOAD_CONFIG,

	FILETYPE_DOWNLOAD_MAX
};

enum
{
	FILETYPE_UPLOAD_CONFIG = 0,
	FILETYPE_UPLOAD_LOG,
	FILETYPE_UPLOAD_CULOG,

	FILETYPE_UPLOAD_MAX
};

int tr069_c2s_fault(struct TR069 *tr069, char *method);
int tr069_c2s_inform(struct TR069 *tr069);
int tr069_c2s_unsupported_response(struct TR069 *tr069);
int tr069_c2s_getrpcmethods_response(struct TR069 *tr069);
int tr069_c2s_getparameternames_response(struct TR069 *tr069);
int tr069_c2s_setparametervalues_response(struct TR069 *tr069);
int tr069_c2s_getparametervalues_response(struct TR069 *tr069);
int tr069_c2s_setparameterattributes_response(struct TR069 *tr069);
int tr069_c2s_getparameterattributes_response(struct TR069 *tr069);
int tr069_c2s_addobject_response(struct TR069 *tr069);
int tr069_c2s_deleteobject_response(struct TR069 *tr069);
int tr069_c2s_reboot_response(struct TR069 *tr069);
int tr069_c2s_download_response(struct TR069 *tr069);
int tr069_c2s_upload_response(struct TR069 *tr069);
int tr069_c2s_factoryreset_response(struct TR069 *tr069);
int tr069_c2s_scheduleinform_response(struct TR069 *tr069);
int tr069_c2s_transfercomplete(struct TR069 *tr069);
int tr069_c2s_default(struct TR069 *tr069);
int tr069_s2c_inform_response(struct TR069 *tr069);
int tr069_s2c_getrpcmethods(struct TR069 *tr069);
int tr069_s2c_getrpcmethods_response(struct TR069 *tr069);
int tr069_s2c_getparameternames(struct TR069 *tr069);
int tr069_s2c_getparametervalues(struct TR069 *tr069);
int tr069_s2c_setparametervalues(struct TR069 *tr069);
int tr069_s2c_setparameterattributes(struct TR069 *tr069);
int tr069_s2c_getparameterattributes(struct TR069 *tr069);
int tr069_s2c_addobject(struct TR069 *tr069);
int tr069_s2c_deleteobject(struct TR069 *tr069);
int tr069_s2c_reboot(struct TR069 *tr069);
int tr069_s2c_download(struct TR069 *tr069);
int tr069_s2c_upload(struct TR069 *tr069);
int tr069_s2c_transfercomplete_response(struct TR069 *tr069);
int tr069_s2c_factoryreset(struct TR069 *tr069);
int tr069_s2c_fault(const struct TR069 *tr069);
int tr069_s2c_scheduleinform(struct TR069 *tr069);

