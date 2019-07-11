#ifndef __APP_TOOL_H__
#define __APP_TOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

int line_get_tag( char* s_str, int s_len, char* tag, char* value );
int line_get_bracket( char* s_str, int s_len, char* title );
int line_to_cap( char* str, int size );
int GetFrontValue( char* ptr, int size, char* value );
int GetAfterValue( char* ptr, int size, char* value );
int GetRemainValue( char* ptr, int size, char* value );


int mid_get_language_full_name_from_iso639(const char *pIsoLanguage, char *pFullNameBuf, int fullNameBufLan );

char* file_get_line( char* s_str, int s_len, char* buf );
int	string_is_digital(const char* str);

#ifdef __cplusplus
}
#endif

#endif

