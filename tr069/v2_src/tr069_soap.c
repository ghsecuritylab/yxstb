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

#include "tr069_header.h"

//------------------------------------------------------------------------------
struct Taget *soap_tag(struct Soap *soap)
{
    return &(soap->tagstack[soap->level - 1]);
}

//------------------------------------------------------------------------------
//
//    SOAP send
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static int soap_out(struct Soap *soap, const char *value, u_int len)
{
    if (soap->length + len > SOAP_ENVELOPE_LEN_128K)
        TR069ErrorOut("length = %d, len = %d\n", soap->length, len);
    memcpy(soap->buffer + soap->length, value, len);
    soap->length += len;

    return 0;
Err:
    return -1;
}

static int soap_out_value(struct Soap *soap, const char *value, u_int len)
{
    char ch, *buf;
    u_int l, i;
    if (soap->length + len > SOAP_ENVELOPE_LEN_128K)
        TR069ErrorOut("length = %d\n", soap->length);
    l = 0;
    i = 0;
    buf = soap->buffer + soap->length;
    while (i < len) {
        ch = value[i];
        if (ch == '<') {//&lt; = '<' = 小于号
            memcpy(buf + l, "&lt;", 4);
            l += 4;
        } else if (ch == '>') {//&gt; = '>' = 大于号
            memcpy(buf + l, "&gt;", 4);
            l += 4;
        } else if (ch == '&') {//&amp; = '&' = 和
            memcpy(buf + l, "&amp;", 5);
            l += 5;
        } else if (ch == '\'') {//&apos; = '\'' = 单引号
            memcpy(buf + l, "&apos;", 6);
            l += 6;
        } else if (ch == '"') {//&quot; = '"' = 双引号
            memcpy(buf + l, "&quot;", 6);
            l += 6;
        } else {
            buf[l] = ch;
            l++;
        }
        i++;
    }
    soap->length += l;

    return 0;
Err:
    return -1;
}

static int soap_out_string(struct Soap *soap, const char *value)
{
    return soap_out(soap, value, strlen(value));
}

//------------------------------------------------------------------------------
int soap_out_element_type(struct Soap *soap, const char *tagname, const char *type, const char *value)
{
    if (!tagname || strlen(tagname) > TR069_NAME_FULL_SIZE_128)
        TR069ErrorOut("tag error\n");

    if (soap_out(soap, "<", 1) || 
        soap_out_string(soap, tagname))
        TR069ErrorOut("soap_out <\n");
    if (type) {
        if (soap_out(soap, " xsi:type=\"xsd:", 15) || 
            soap_out_string(soap, type) || 
            soap_out(soap, "\"", 1))
            TR069ErrorOut("soap_out type\n");
    }
    if (soap_out(soap, ">", 1))
        TR069ErrorOut("soap_out >\n");

    if (soap->length > SOAP_ENVELOPE_LEN_128K)
        TR069ErrorOut("length  = %d\n", soap->length);

    if (soap_out_value(soap, value, strlen(value)))
        TR069ErrorOut("soap_out_value\n");

    if (soap_out(soap, "</", 2) || 
        soap_out_string(soap, tagname) || 
        soap_out(soap, ">\r\n", 3))
        TR069ErrorOut("soap_out /\n");

    return 0;
Err:
    return -1;
}

int soap_out_element(struct Soap *soap, const char *tagname, const char *value)
{
    if (!value)
        TR069ErrorOut("value is NULL\n");
    return soap_out_element_type(soap, tagname, NULL, value);
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_out_element_begin(struct Soap *soap, const char *tagname, const char *type)
{
    struct Taget *tag;
    u_int len;

    if (!tagname)
        TR069ErrorOut("tag error\n");
    len = strlen(tagname);
    if ( len > TR069_NAME_FULL_SIZE_128)
        TR069ErrorOut("len = %d\n", len);

    if (soap_out(soap, "<", 1) || 
        soap_out_string(soap, tagname))
        TR069ErrorOut("soap_out <\n");

    if (type)
        if (soap_out(soap, " SOAP-ENC:arrayType=\"", 21) || 
            soap_out_string(soap, type) || 
            soap_out(soap, "\"", 1))
            TR069ErrorOut("soap_out arrayType\n");

    if (soap_out(soap, ">\r\n", 3))
        TR069ErrorOut("soap_out >\n");

    tag = &(soap->tagstack[soap->level]);
    memcpy(tag->name, tagname, len);
    tag->namelen = len;
    soap->level ++;
    if (soap->level >= SOAP_LEVEL_LEN_10)
        TR069ErrorOut("level= %d\n", soap->level);

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_out_element_end(struct Soap *soap)
{
    struct Taget *tag;

    soap->level --;

    tag = &(soap->tagstack[soap->level]);
    if (soap_out(soap, "</", 2) || 
        soap_out(soap, tag->name, tag->namelen) || 
        soap_out(soap, ">\r\n", 3))
        TR069ErrorOut("soap_out\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_out_envelope_begin(struct Soap *soap, const char *cwmpid, const char *cwmpmthl)
{
    struct Taget *tag;

    soap->index = 0;
    soap->level = 0;
    soap->length = 0;

    if (soap_out(soap, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\r\n", 277))
        TR069ErrorOut("soap_out Envelope\n");

    tag = &(soap->tagstack[soap->level]);
    strcpy(tag->name, "Envelope");
    tag->namelen = 8;
    soap->level ++;
    if (soap_out(soap, "<SOAP-ENV:Header>\r\n", 19) || 
        soap_out(soap, "<cwmp:ID SOAP-ENV:mustUnderstand=\"1\">", 37) || 
        soap_out_string(soap, cwmpid) || 
        soap_out(soap, "</cwmp:ID>\r\n", 12) || 
        soap_out(soap, "</SOAP-ENV:Header>\r\n", 20))
        TR069ErrorOut("soap_out Header\n");

    if (soap_out(soap, "<SOAP-ENV:Body>\r\n", 17))
        TR069ErrorOut("soap_out Body\n");
    tag = &(soap->tagstack[soap->level]);
    strcpy(tag->name, "Body");
    tag->namelen = 4;
    soap->level ++;

    if (strcmp(cwmpmthl, "Fault") == 0)
        soap_out(soap, "<SOAP-ENV:", 10);
    else
        soap_out(soap, "<cwmp:", 6);
    if (soap_out_string(soap, cwmpmthl) || 
        soap_out(soap, ">\r\n", 3))
        TR069ErrorOut("soap_out cwmp\n");
    tag = &(soap->tagstack[soap->level]);
    strcpy(tag->name, cwmpmthl);
    tag->namelen = strlen(cwmpmthl);
    soap->level ++;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_out_envelope_end(struct Soap *soap)
{
    struct Taget *tag;

    if (soap->level != 3)
        TR069ErrorOut("level = %d\n", soap->level);
    soap->level --;
    tag = &(soap->tagstack[soap->level]);
    if (strcmp(tag->name, "Fault") == 0)
        soap_out(soap, "</SOAP-ENV:", 11);
    else
        soap_out(soap, "</cwmp:", 7);
    if (soap_out(soap, tag->name, tag->namelen) || 
        soap_out(soap, ">\r\n", 3))
        TR069ErrorOut("soap_out /cwmp\n");

    soap->level --;    
    if (soap_out(soap, "</SOAP-ENV:Body>\r\n", 18))
        TR069ErrorOut("soap_out Body\n");

    soap->level --;
    if (soap_out(soap, "</SOAP-ENV:Envelope>\r\n", 22))
        TR069ErrorOut("soap_out Envelope\n");
    soap->buffer[soap->length] = 0;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
//
//    SOAP receive
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#if 0
static void in_blank(struct Soap *soap)
{
    int l, length;
    char *buffer;

    buffer = soap->buffer + soap->index;
    length = soap->length - soap->index;

    for (l = 0; l < length && isspace(buffer[l]); l ++) ;
    soap->index += l;
}
#endif

//------------------------------------------------------------------------------
static u_int in_tag_len(const char *buffer, u_int length, u_int *pIndex)
{
    u_int index = *pIndex;

    for (; index < length && (isalnum(buffer[index]) || buffer[index] == '-'); index ++) ;
    if (buffer[index] != ':')
        return (index - *pIndex);

    index ++;
    *pIndex = index;
    for (; index < length && isalnum(buffer[index]); index ++) ;

    return (index - *pIndex);
}

//------------------------------------------------------------------------------
void soap_in_clangle(struct Soap *soap)
{
    u_int index, length;
    char *buffer;

    buffer = soap->buffer;
    length = soap->length;

    for (index = soap->index; index < length && buffer[index] != '<'; index ++) ;
    soap->index = index;
}

static int soap_in_element_tag(struct Soap *soap, int begin)
{
    struct Taget *tag;
    u_int index, len, length;
    char *pBegin, *pEnd, *buffer;

    soap_in_clangle(soap);

    length = soap->length;
    buffer = soap->buffer;
    index = soap->index;
    if (buffer[index] != '<') {
        TR069ErrorOut("mis < buffer[%d] = %c\n", index, buffer[index]);
    }
    index ++;

    if (buffer[index] == '?') {
        for (; index < length; index ++) {
            if (buffer[index] == '<')
             break;
        }
        index ++;
    }

    if (begin == 0) {
        soap->end = soap->index;
        if (buffer[index] != '/')
            TR069ErrorOut("mis / buffer[%d] = %c\n", index, buffer[index]);
        index ++;
    }

    len = in_tag_len(buffer, length, &index);

    if (len == 0 || len > TR069_NAME_FULL_SIZE_128 || len >= length)
        TR069ErrorOut("len = %d\n", len);

    pBegin = buffer + index;
    if (begin) {
        tag = &(soap->tagstack[soap->level]);
        memcpy(tag->name, pBegin, len);
        tag->name[len] = 0;
        tag->namelen = len;
        soap->level ++;
    } else {
        tag = &(soap->tagstack[soap->level - 1]);
        if (len != tag->namelen || memcmp(tag->name, pBegin, len))
            TR069ErrorOut("mis match %s %c%c\n", tag->name, pBegin[0], pBegin[1]);
        soap->level --;
    }
    index += len;
    pBegin = buffer + index;

    for (; index < length; index ++) {
        if (buffer[index] == '>')
             break;
        if (buffer[index] == '/' && buffer[index + 1] == '>') {
            if (begin != 1)
                TR069ErrorOut("/> should only exist in begin!\n");
           //begin = 0;
            break;
        }
    }
    do {//分离出type
        if (begin == 0)
            break;
        pEnd = buffer + index;
        soap->tagtype[0] = 0;
        if (pEnd - pBegin < 15)
            break;
        pBegin = strstr(pBegin, "xsi:type=\"xsd:");
        if (!pBegin || pBegin >= pEnd)
            break;
        pBegin += 14;
        pEnd = strchr(pBegin, '"');
        if (!pEnd || pEnd >= buffer + index)
            break;
        len = (u_int)(pEnd - pBegin);
        if (len > SOAP_TYPE_LEN_16)
            break;
        memcpy(soap->tagtype, pBegin, len);
        soap->tagtype[len] = 0;
    } while (0);
    if (buffer[index] == '/' && buffer[index + 1] == '>') {
        soap->index = index + 2;
        soap->single = 1;
    } else {
        if (buffer[index] != '>')
            TR069ErrorOut("mis >\n");
        soap->index = index + 1;
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_in_element_begin(struct Soap *soap, const char *tagname)
{
    soap->single = 0;
    if (soap_in_element_tag(soap, 1))
        TR069ErrorOut("mis tag\n");

    if (tagname && stricmp(soap->tagstack[soap->level - 1].name, tagname))
        TR069ErrorOut("mis match\n");

    soap->begin = soap->index;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_in_element_end(struct Soap *soap)
{
    soap->end = soap->index;

    if (soap->single == 1) {
        soap->level --;
        soap->single = 0;
        return 0;
    }

    if (soap_in_element_tag(soap, 0))
        TR069ErrorOut("soap_in_element_tag\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_in_element(struct Soap *soap, const char *tag)
{
    if (soap_in_element_begin(soap, tag))
        TR069ErrorOut("soap_in_element_begin\n");

    if (soap_in_element_end(soap))
        TR069ErrorOut("soap_in_element_end\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_in_element_value(struct Soap *soap, char *vbuf, u_int vlen)
{
    char ch, *p, *buf, *value;
    u_int i, len;

    if (!vbuf)
        TR069ErrorOut("vbuf is NULL\n");

    vbuf[0] = 0;
    len = soap->end - soap->begin;
    if (len >= vlen)
        TR069ErrorOut("len = %d, vlen = %d\n", len, vlen);
    i = 0;
    buf = soap->buffer + soap->begin;
    p = vbuf;
    while (i < len) {
        ch = buf[i];
        i++;
        if (ch != '&') {
            *p = ch;
            p++;
            continue;
        }
        value = buf + i;
        if (strncmp(value, "lt;", 3) == 0) {//&lt; = '<' = 小于号
            i += 3;
            *p++ = '<';
        } else if (strncmp(value, "gt;", 3) == 0) {//&gt; = '>' = 大于号
            i += 3;
            *p++ = '>';
        } else if (strncmp(value, "amp;", 4) == 0) {//&amp; = '&' = 和
            i += 4;
            *p++ = '&';
        } else if (strncmp(value, "apos;", 5) == 0) {//&apos; = '\'' = 单引号
            i += 5;
            *p++ = '\'';
        } else if (strncmp(value, "quot;", 5) == 0) {//&quot; = '"' = 双引号
            i += 5;
            *p++ = '"';
        } else {
            TR069Warn("\n");
            vbuf[0] = '<';
            vbuf[1] = 0;
            return 0;
        }
    }
    *p = 0;

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
char *soap_in_element_type(struct Soap *soap)
{
    return soap->tagtype;
}

//------------------------------------------------------------------------------
int soap_in_element_either(struct Soap *soap)
{
    soap_in_clangle(soap);

    if (soap->buffer[soap->index + 1] == '/')
        return soap_in_element_end(soap);
    else
        return soap_in_element_begin(soap, NULL);
}

//------------------------------------------------------------------------------
int soap_in_envelope_begin(struct Soap *soap, char *reqid, u_int size)
{
    struct Taget *tag;
    soap->index = 0;
    soap->level = 0;

    soap->buffer[soap->length] = 0;

    if (soap_in_element_begin(soap, "Envelope") || 
        soap_in_element_begin(soap, NULL))
        TR069ErrorOut("mis Header\n");
    tag = soap_tag(soap);
    if (strcmp(tag->name, "Header")) {
        if (soap_in_element_begin(soap, NULL))
            TR069ErrorOut("mis method\n");
        return 0;
    }

    reqid[0] = 0;
    for (;;) {
        int valid = 0;

        soap_in_clangle(soap);
        if (soap->buffer[soap->index + 1] == '/')
            break;
        if (soap_in_element_begin(soap, NULL))
            TR069ErrorOut("soap_in_element_begin\n");

        tag = soap_tag(soap);
        if (strcmp(tag->name, "ID") == 0)
            valid = 1;
        if (soap_in_element_end(soap))
            TR069ErrorOut("soap_in_element_end\n");

        if (valid) {
            if (soap_in_element_value(soap, reqid, size))
                TR069ErrorOut("soap_in_element_value\n");
        }
    }
    soap_in_element_end(soap);
    if (reqid[0] == 0)
        TR069ErrorOut("mis ID\n");

    if (soap_in_element_begin(soap, "Body"))
        TR069ErrorOut("mis Body\n");
    if (soap_in_element_begin(soap, NULL))
        TR069ErrorOut("mis method\n");

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int soap_in_envelope_end(struct Soap *soap)
{
    if (soap_in_element_end(soap) || 
        soap_in_element_end(soap) || 
        soap_in_element_end(soap))
        TR069ErrorOut("soap_in_element_end\n");
    if (soap->level != 0)
        TR069ErrorOut("level = %d\n", soap->level);

    soap->buffer[soap->length] = 0;

#ifdef TR069_DEBUG_TRACE
    if (soap->trackfp) {
        fclose(soap->trackfp);
        soap->trackfp = NULL;
    }
#endif
    return 0;
Err:
    return -1;
}


