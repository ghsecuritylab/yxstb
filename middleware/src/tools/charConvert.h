#ifndef charConversion_h
#define  charConversion_h

#ifdef __cplusplus
extern "C" {
#endif

char hex2Char(char c);
int data2Hex(char* in, int inLen, char* out, int outLen);
int char2Int(int c);
int hex2Data(char* in, int inLen, char* out, int outLen);

int lower2Upper(char* str, int len);
int upper2Lower(char* str, int len);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //charConversion_h
