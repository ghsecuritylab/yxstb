//==========================================================================
//
//      ftpclient.c
//
//      A simple FTP client
//      http://sources.redhat.com/ecos/ecos-license/
//
//==========================================================================

#ifndef __TR069_FTP_H__
#define __TR069_FTP_H__

int tr069_ftp_put(char *url, char *username, char *password, char *content, int length);
int tr069_ftp_get(char *url, char *username, char *password, char *content, int length);

#endif//__TR069_FTP_H__
