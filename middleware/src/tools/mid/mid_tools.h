#ifndef __MID_TOOLS_H__
#define __MID_TOOLS_H__


#ifdef __cplusplus
extern "C" {
#endif
void mid_tool_mem_line(unsigned char *p);

void mid_tool_mem_show(unsigned char *p, int len);
void mid_tool_string_show(unsigned char *str);
void mid_tool_lines_show(char *str, int delay);
void mid_tool_string_cutcpy(char *buf, char *str, int max);

void mid_tool_line_first(char *string, char *buf);
int mid_tool_line_len(char *string);
int mid_tool_line_print(char *line);

//YYYYMMDDHHMMSS
unsigned int mid_tool_string2time(char* str);
//YYYYxMMxDDxHHxMMxSS
int mid_tool_time2string(int sec, char* buf, char insert);
//HHxMMxSS
int mid_tool_time2str(int sec, char* buf, char insert);

int mid_tool_resolvehost(char *buf, unsigned int *hostip);
int mid_tool_addr2string(unsigned int addr, char *buf);

int mid_tool_checkURL( const char* url, char *p_addr, int *p_port);

int mid_tool_check_RTSPURL(char* url, char *p_addr);

int mid_tool_inet_ntoa(unsigned int addr, char *buf);

int mid_tool_time2string_min(int sec, char* buf);

char* distillValuebyTag ( const char *xml, char *tagName, int *tagValueLen, int deadline );


int mid_tool_timezone2str(int pTimezone, char* buf);
int mid_tool_str2timezone(char* str);
int mid_tool_timezone2sec(int pTimezone);

int mid_tool_checksum(char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif//__MID_TOOL_H__
