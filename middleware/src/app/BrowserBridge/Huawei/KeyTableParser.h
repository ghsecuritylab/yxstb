#ifndef _KeyTableParser_H_
#define _KeyTableParser_H_

namespace Hippo {

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int  KeyTableParser(const char* table);

int KeyTableGet(const char* keyTableUrl);
int KeyTableUrlGet();

int KeyTableUpdate();

#ifdef __cplusplus
}
#endif // __cplusplus

}
#endif // _KeyTableParser_H_
