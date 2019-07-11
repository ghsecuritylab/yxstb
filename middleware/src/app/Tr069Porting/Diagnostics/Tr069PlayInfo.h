#ifndef _Tr069PlayInfo_H_
#define _Tr069PlayInfo_H_

#ifdef __cplusplus

namespace Hippo {

class Tr069PlayInfo {
};

} // namespace Hippo

extern "C" {
#endif // __cplusplus

int Tr069GetCurrentPlayURL(char *url, int pUrlLen);
int Tr069GetCurrentPlayState();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _Tr069PlayInfo_H_

