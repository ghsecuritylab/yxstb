#include "StringData.h"

#include <stdio.h>
#include <string.h>

namespace Hippo {

StringData::StringData(const char* data) : mData(0), mSize(0)
{
    if (data) {
        mSize = strlen(data) + 1;
        mData = new char[strlen(data) + 1];
        strcpy(mData, data);
    }
}

StringData::~StringData() 
{ 
    if (mData) 
        delete []mData; 
}

}
