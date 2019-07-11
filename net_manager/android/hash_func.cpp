/*
 * hash_func.cpp
 *
 *  Created on: 2013Äê11ÔÂ26ÈÕ
 *      Author: chenbaoqiang
 */
extern "C"{
#include <stdio.h>
#include <string.h>
}

static void ILibToLower(const char *in, int inLength, char *out)
{
	int i;

	for(i=0;i<inLength;++i)
	{
		if (in[i]>=65 && in[i]<=90)
		{
			//CAP
			out[i] = in[i]+32;
		}
		else
		{
			//LOWER
			out[i] = in[i];
		}
	}
}


int ILibGetHashValueEx(char *key, int keylength, int caseInSensitiveText)
{
	int HashValue=0;
	char TempValue[4];

	if (keylength<=4)
	{
		//
		// If the key length is <= 4, the hash is just the key expressed as an integer
		//
		memset(TempValue,0,4);
		if (caseInSensitiveText==0)
		{
			memcpy(TempValue,key,keylength);
		}
		else
		{
			ILibToLower(key,keylength,TempValue);
		}

		HashValue = *((int*)TempValue);
	}
	else
	{
		//
		// If the key length is >4, the hash is the first 4 bytes XOR with the last 4
		//

		if (caseInSensitiveText==0)
		{
			memcpy(TempValue,key,4);
		}
		else
		{
			ILibToLower(key,4,TempValue);
		}
		HashValue = *((int*)TempValue);
		if (caseInSensitiveText==0)
		{
			memcpy(TempValue,(char*)key+(keylength-4),4);
		}
		else
		{
			ILibToLower((char*)key+(keylength-4),4,TempValue);
		}
		HashValue = HashValue^(*((int*)TempValue));


		//
		// If the key length is >= 10, the hash is also XOR with the middle 4 bytes
		//
		if (keylength>=10)
		{
			if (caseInSensitiveText==0)
			{
				memcpy(TempValue,(char*)key+(keylength/2),4);
			}
			else
			{
				ILibToLower((char*)key+(keylength/2),4,TempValue);
			}
			HashValue = HashValue^(*((int*)TempValue));
		}
	}
	return(HashValue);
}



