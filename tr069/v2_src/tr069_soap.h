/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			SOAP 协议
 *******************************************************************************/

#ifndef __SOAP_H__
#define __SOAP_H__

#define SOAP_BUFFER_ERROR			-1
#define SOAP_MALLOC_ERROR			-2

#define SOAP_LEVEL_LEN_10			10

#define SOAP_TYPE_LEN_16			16
#define SOAP_ACTION_LEN_32			32

#define SOAP_ENVELOPE_LEN_128K		(1024 * 128)
#define SOAP_ENVELOPE_RESERVE_1K	(1024)//soap预留最小空间

#define SOAP_METHOD_LEVEL		3

struct Taget {
	char name[TR069_NAME_FULL_SIZE_128 + 4];
	u_int namelen;
};

struct Soap {
	struct Taget tagstack[SOAP_LEVEL_LEN_10 + 1];
	char tagtype[SOAP_TYPE_LEN_16 + 4];
	u_int begin; //value begin index
	u_int end; //value end index
	int single;//<X/>这种格式

	int level;
	u_int length;
	u_int index;
	char buffer[SOAP_ENVELOPE_LEN_128K + SOAP_ENVELOPE_RESERVE_1K + 4];
};

struct Taget *soap_tag(struct Soap *soap);

int soap_out_element(struct Soap *soap, const char *tagname, const char *value);
int soap_out_element_type(struct Soap *soap, const char *tagname, const char *type, const char *value);
int soap_out_element_begin(struct Soap *soap, const char *tagname, const char *type);
int soap_out_element_end(struct Soap *soap);
int soap_out_envelope_begin(struct Soap *soap, const char *cwmpid, const char *cwmpmthl);
int soap_out_envelope_end(struct Soap *soap);

void soap_in_clangle(struct Soap *soap);

int soap_in_element_begin(struct Soap *soap, const char *tagname);
int soap_in_element_end(struct Soap *soap);
int soap_in_element(struct Soap *soap, const char *tagname);
int soap_in_element_value(struct Soap *soap, char *vbuf, const u_int size);
char *soap_in_element_type(struct Soap *soap);
int soap_in_element_either(struct Soap *soap);
int soap_in_envelope_begin(struct Soap *soap, char *reqid, const u_int size);
int soap_in_envelope_end(struct Soap *soap);

#endif //__SOAP_H__

