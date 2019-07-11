#ifndef __JSE_H__
#define __JSE_H__

#ifdef __cplusplus
extern "C" {
#endif

int jse_parse_param(char *str, char *cmd, char in, char *fmt, ...);

int jse_init(void);

#ifdef __cplusplus
}
#endif

#endif
