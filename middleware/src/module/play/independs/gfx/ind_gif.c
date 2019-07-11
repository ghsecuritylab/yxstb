
#include <stdio.h>

#include "ind_gfx.h"
#include "app/Assertions.h"

static int int_GIF_Header(uint8_t *buf, int len, int *ver)
{
    int size, bytes;

    if (len <= 13)
        ERR_OUT("len = %d\n", len);
    if (memcmp(buf, "GIF87a", 6) == 0)
        *ver = 7;
    else if (memcmp(buf, "GIF89a", 6) == 0)
        *ver = 9;
    else
        ERR_OUT("version\n");

    //Size of Global Color Table
    if (buf[10] & 0x80)
        size = 1 << ((int)(buf[10] & 7) + 1);
    else
        size = 0;

    bytes = 13 + size * 3;

    if (len <= bytes)
        ERR_OUT("len = %d, bytes = %d\n", len, bytes);

    return bytes;
Err:
    return -1;
}

static int int_Data_blocks(uint8_t *buf, int len)
{
    int size, bytes;

    bytes = 0;

    while (1) {
        size = (int)((uint32_t)buf[0]);
        if (size == 0)
            break;
        size += 1;
        if (size < 0 || size >= len)
            ERR_OUT("size = %d, len = %d\n", size, len);

        bytes += size;
        buf += size;
        len -= size;
    }

    bytes += 1;

    return bytes;
Err:
    return -1;
}

static int int_Application_Extension(uint8_t *buf, int len)
{
    int l, bytes;

    if (len <= 11)
        ERR_OUT("len = %d\n", len);

    if (buf[2] != 0x0b)
        ERR_OUT("Block Size\n");

    bytes = 3 + 0x0b;
    buf += 3 + 0x0b;
    len -= 3 + 0x0b;

    l = int_Data_blocks(buf, len);
    if (l < 0)
        ERR_OUT("int_Data_blocks\n");
    bytes += l;

    return bytes;
Err:
    return -1;
}

static int int_Graphic_Control_Extension(uint8_t *buf, int len)
{
    int bytes;

    if (len <= 3 + 0x04 + 1)
        ERR_OUT("len = %d\n", len);

    if (buf[2] != 0x04 || buf[7] != 0)
        ERR_OUT("Size = %02x, Terminator = %02x\n", (uint32_t)buf[2], (uint32_t)buf[7]);

    bytes = 8;

    return bytes;
Err:
    return -1;
}

static int int_Comment_Extension_Block(uint8_t *buf, int len)
{
    int l, bytes;

    if (len <= 2)
        ERR_OUT("len = %d\n", len);

    bytes = 2;
    buf += 2;
    len -= 2;

    l = int_Data_blocks(buf, len);
    if (l < 0)
        ERR_OUT("int_Data_blocks\n");
    bytes += l;

    return bytes;
Err:
    return -1;
}

static int int_Plain_Text_Extension_Block(uint8_t *buf, int len)
{
    int l, bytes;

    if (len <= 3 + 0x0c)
        ERR_OUT("len = %d\n", len);

    if (buf[2] != 0x0c)
        ERR_OUT("Block Size\n");

    bytes = 3 + 0x0c;
    buf += 3 + 0x0c;
    len -= 3 + 0x0c;

    l = int_Data_blocks(buf, len);
    if (l < 0)
        ERR_OUT("int_Data_blocks\n");
    bytes += l;

    return bytes;
Err:
    return -1;
}

static int int_Extension_Block(uint8_t *buf, int len)
{
    int l, bytes;
    uint32_t label;

    bytes = 0;
    while (buf[0] == 0x21) {
        label = (uint32_t)buf[1];

        switch(label) {
        case 0x01:
            l = int_Plain_Text_Extension_Block(buf, len);
            if (l <= 0)
                ERR_OUT("int_Graphic_Control_Extension\n");
            break;

        case 0xf9:
            l = int_Graphic_Control_Extension(buf, len);
            if (l <= 0)
                ERR_OUT("int_Graphic_Control_Extension\n");
            break;

        case 0xfe:
            l = int_Comment_Extension_Block(buf, len);
            if (l <= 0)
                ERR_OUT("int_Graphic_Control_Extension\n");
            break;

        default:
            ERR_OUT("label = %02x\n", label);
        }
        bytes += l;
        buf += l;
        len -= l;
    }

    return bytes;
Err:
    return -1;
}

static int int_Image_Block(uint8_t *buf, int len)
{
    int l, bytes;

    if (len <= 10)
        ERR_OUT("len = %d\n", len);

    if (buf[0] != 0x2c)
        ERR_OUT("Image Separator = 0x%02x\n", (uint32_t)buf[0]);

    //Size of Local Color Table
    if (buf[9] & 0x80)
        l = 1 << ((int)(buf[9] & 7) + 1);
    else
        l = 0;

    bytes = 10 + l * 3 + 1;
    buf += bytes;
    len -= bytes;

    l = int_Data_blocks(buf, len);
    if (l < 0)
        ERR_OUT("int_Data_blocks\n");
    bytes += l;

    return bytes;
Err:
    return -1;
}

static int int_Graphic_Image(uint8_t *buf, int len)
{
    int l, bytes;

    bytes = 0;

    l = int_Extension_Block(buf, len);
    if (l < 0)
        ERR_OUT("int_Extension_Block\n");
    bytes += l;
    buf += l;
    len -= l;

    l = int_Image_Block(buf, len);
    if (l <= 0)
        ERR_OUT("int_Application_Extension\n");
    bytes += l;

    return bytes;
Err:
    return -1;
}

static int int_GIF87a(uint8_t *buf, int len)
{
    int bytes;

    bytes = int_Image_Block(buf, len);
    if (bytes <= 0)
        ERR_OUT("int_Image_Block\n");

    return bytes;
Err:
    return -1;
}

static int int_GIF89a(uint8_t *buf, int len)
{
    int l, bytes;

    if (buf[0] == 0x21) {//Extension Introducer
        bytes = 0;
        if (buf[1] == 0xff) {
            l = int_Application_Extension(buf, len);
            if (l <= 0)
                ERR_OUT("int_Application_Extension\n");
            bytes += l;
            buf += l;
            len -= l;
    
            while (buf[0] == 0x21 && len > 0) {
                l = int_Graphic_Image(buf, len);
                if (l <= 0)
                    ERR_OUT("int_Application_Extension\n");
                bytes += l;
                buf += l;
                len -= l;
            }
        } else {
            l = int_Graphic_Image(buf, len);
            if (l <= 0)
                ERR_OUT("int_Application_Extension\n");
            bytes += l;
        }
    } else {
        bytes = int_Image_Block(buf, len);
        if (bytes <= 0)
            ERR_OUT("int_Image_Block\n");
    }

    return bytes;
Err:
    return -1;
}

int int_gif_check(uint8_t *buf, int len)
{
    int l, ver = 0;

    if (buf == NULL || len <= 0)
        ERR_OUT("buf == %p, len == %d\n", buf, len);

    l = int_GIF_Header(buf, len, &ver);
    if (l <= 0)
        ERR_OUT("int_GIF_Header\n");

    buf += l;
    len -= l;

    switch (ver) {
    case 7:
        l = int_GIF87a(buf, len);
        if (l <= 0)
            ERR_OUT("int_GIF87a\n");
        break;
    case 9:
        l = int_GIF89a(buf, len);
        if (l <= 0)
            ERR_OUT("int_GIF89a\n");
        break;
    default:
        ERR_OUT("ver = %d\n", ver);
    }

    buf += l;
    len -= l;

    if (len != 1)
        ERR_OUT("len = %d\n", len);
    if (buf[0] != 0x3b)
        ERR_OUT("Trailer\n");

    return 0;
Err:
    return -1;
}

int int_jpg_check(uint8_t *buf, int len)
{
    int l, off;
    uint32_t code;

    if (len <= 2)
        ERR_OUT("len = %d\n", len);
    if (buf[0] != 0xff || buf[1] != 0xd8)
        ERR_OUT("SOI\n");
    buf += 2;
    len -= 2;
    off = 2;

    while (len >= 2) {
        if (buf[0] != 0xff)
            ERR_OUT("off = 0x%x: segment!\n", off);
        code = buf[1];
        if (0xff == code) {
            buf++;
            len--;
            off++;
            continue;
        }

        buf += 2;
        len -= 2;
        off += 2;

        if (0x01 == code || (code >= 0xd0 && code <= 0xd7))
            continue;

        if (0xff == buf[0])
            WARN_PRN("off = 0x%x!\n", off);
        l = (int)(((uint32_t)buf[0] << 8) + buf[1]);
        //PRINTF("code = 0x%02x, off = 0x%x: l = %d\n", code, off, l);
        if (len <= l)
            ERR_OUT("off = 0x%x: l = %d / %d\n", off, l, len);

        buf += l;
        len -= l;
        off += l;

        if (code == 0xda) {
            if (len < 2)
                ERR_OUT("off = 0x%x: len = %d!\n", off, len);
            if (0xff != buf[len - 2] || 0xd9 != buf[len - 1])
                ERR_OUT("EOF\n");
            return 0;
        }
    }

    ERR_PRN("off = 0x%x\n", off);
Err:
    return -1;
}

int int_jpg_check_ex(uint8_t *buf, int len)
{
    int l, off;
    uint32_t code;

    if (len <= 2)
        ERR_OUT("len = %d\n", len);
    if (buf[0] != 0xff || buf[1] != 0xd8)
        ERR_OUT("SOI\n");
    buf += 2;
    len -= 2;
    off = 2;

    while (len >= 2) {
        if (buf[0] != 0xff)
            ERR_OUT("off = 0x%x: segment!\n", off);
        code = buf[1];

        PRINTF("code = %02X\n", code);
      	if (code >= 0xE1 && code <= 0xEF) {
      		PRINTF("APP%d", code - 0xE0);
  	    } else if (code == 0xDB) {
      		PRINTF("DQT");
  	    } else if (code == 0xC4) {
  		    PRINTF("DHT");  		
      	} else if (code == 0xDD) {
  	    	PRINTF("DRI");
      	} else if (code == 0xC0 || code == 0xC2) {
  	    	PRINTF("SOF");
      	} else if (code == 0xDA) {
  	    	PRINTF("SOS");
            if (len < 2)
                ERR_OUT("off = 0x%x: len = %d!\n", off, len);
            if (buf[len - 2] != 0xff || buf[len - 1] != 0xd9)
                ERR_OUT("EOF\n");
            return 0;
      	} else {
  		    ERR_OUT("Header\n");
  	    }
  	
  	    buf += 2;
      	len -= 2;
  	
      	l = (int)(((uint32_t)buf[0] << 8) + buf[1]);
        if (len <= l)
            ERR_OUT(", off = 0x%x: l = %d / %d\n", off, l, len);
  	    buf += l;
      	len -= l;
  	    PRINTF(", len = %d\n", l);
    }
Err:
    return -1;
}

