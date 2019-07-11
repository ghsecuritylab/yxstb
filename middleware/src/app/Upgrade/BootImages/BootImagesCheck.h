#ifndef __BootImagesCheck__H
#define __BootImagesCheck__H 

namespace Hippo {

int BootImagesCheckJPG(const char* filePath, unsigned long* width, unsigned long* height);
int BootImagesCheckPNG(const char* filePath, unsigned long* width, unsigned long* height);
int BootImagesCheckGIF(const char* filePath, unsigned long* width, unsigned long* height);
int BootImagesCheckBMP(const char* filePath, unsigned long* width, unsigned long* height);
}

#endif
