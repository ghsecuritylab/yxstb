#ifndef CodeTransform_H
#define CodeTransform_H

namespace Hippo{
    
int getAcronymFromUtf8(unsigned char *pIn, int len, char *pOut, int *outLen);
int GBtoUtf8(unsigned char* inBuf, int inLen, unsigned char* outBuf, int* outLen);

}; //namespace Hippo

#endif //CodeTransform_H
