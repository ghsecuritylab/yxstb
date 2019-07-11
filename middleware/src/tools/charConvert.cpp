#include "charConvert.h"

#include <stdio.h>
#include <ctype.h>

int lower2Upper(char* str, int len)
{
    if (!str || len < 0)
    	return -1;

    int i;
    for(i = 0 ; i < len; i++)
        str[i] = toupper(str[i]);
    return 0;
}

int upper2Lower(char* str, int len)
{
    if (!str || len < 0)
        return -1;

    int i;
    for(i = 0 ; i < len; i++)
        str[i] = tolower(str[i]);
    return 0;
}

char hex2Char(char c)
{
    if (c >= 0 && c <= 9)
        c = c + '0';
    else if (c >= 10 && c <= 15)
        c = c + 'a' - 10;
    return c;
}

int data2Hex(char* in, int inLen, char* out, int outLen)
{
    if (((inLen << 1) + 1) > outLen)
        return -1;
    if (!in || !out)
        return -1;
    int i;

    for (i = inLen - 1; i >= 0; i--) {
        out[(i<<1)+1] = hex2Char((in[i]) & 0x0f);
        out[i<<1] = hex2Char(in[i]>>4);
    }
    out[2*inLen] = '\0';
    return 2 * inLen;
}

int char2Int(int c)
{
    if( c >= '0' && c <= '9'){
        c = c - '0';
    } else if (c >= 'a' && c <= 'z') {
        c = c - 'a' + 10;
    } else if (c >= 'A' && c <- 'Z') {
        c = c - 'A' + 10;
    }
    return c;
}

int hex2Data(char* in, int inLen, char* out, int outLen)
{
    if (((inLen + 1) >> 1) > outLen)
        return -1;
    if (!in || !out)
        return -1;

    int d, i;

    for (i = ((inLen - 1) >> 1); i >= 0; i--) {
        d = char2Int(in[i << 1]);
        d = (d << 4) + char2Int(in[(i << 1) + 1]);
        *(out+i) = d;
    }
    out[(inLen + 1) >> 1] = '\0';

    return (inLen + 1) >> 1;
}
