/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            tr069 实现
 *******************************************************************************/
#ifndef __TR069_DEFINE_H__
#define __TR069_DEFINE_H__

#define TR069_FAULT_NUM             64

#define TR069_URL_LEN_1024          1024
#define TR069_REQUEST_PORT          7547

#define DEVICE_INDEX_NUM_2048       2048

#define TR069_NAME_SIZE_64          64
#define TR069_NAME_FULL_SIZE_128    128

#define TR069_ENABLE_SAVE           0x80//TR069 内部的配置文件会保存该值的
#define TR069_ENABLE_CONFIG         0x40//TR069 参数重置时将从默认配置文件中读取该参数值
#define TR069_ENABLE_ATTR           0x20//支持属性修改
#define TR069_ENABLE_APPLY          0x10//该参数是否设置后立即生效
#define TR069_ENABLE_WRITE          0x08//可写
#define TR069_ENABLE_READ           0x04//可读
#define TR069_ENABLE_ADD            0x02//只对TR069_OBJECT类型有效
#define TR069_ENABLE_DELETE         0x01//只对TR069_OBJECT类型有效

#define TR069_ENABLE_READ_WRITE                         (TR069_ENABLE_READ | TR069_ENABLE_WRITE)
#define TR069_ENABLE_READ_WRITE_APPLY                   (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_APPLY)
#define TR069_ENABLE_READ_WRITE_APPLY_SAVE              (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_SAVE)
#define TR069_ENABLE_READ_WRITE_APPLY_ATTR              (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR)
#define TR069_ENABLE_READ_WRITE_APPLY_ATTR_SAVE         (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR | TR069_ENABLE_SAVE)
#define TR069_ENABLE_READ_WRITE_APPLY_ATTR_CONFIG       (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR | TR069_ENABLE_CONFIG)
#define TR069_ENABLE_READ_WRITE_APPLY_ATTR_CONFIG_SAVE  (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR | TR069_ENABLE_CONFIG | TR069_ENABLE_SAVE)

#define TR069_ENABLE_READ_SAVE                          (TR069_ENABLE_READ | TR069_ENABLE_SAVE)
#define TR069_ENABLE_READ_ATTR                          (TR069_ENABLE_READ | TR069_ENABLE_ATTR)

#define TR069_ENABLE_READ_WRITE_ATTR                    (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_ATTR)
#define TR069_ENABLE_READ_WRITE_ATTR_SAVE               (TR069_ENABLE_READ | TR069_ENABLE_WRITE | TR069_ENABLE_ATTR | TR069_ENABLE_SAVE)

#define TR069_ENABLE_WRITE_APPLY_ATTR                   (TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR)
#define TR069_ENABLE_WRITE_APPLY_ATTR_SAVE              (TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR | TR069_ENABLE_SAVE)
#define TR069_ENABLE_WRITE_APPLY_ATTR_CONFIG            (TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR | TR069_ENABLE_CONFIG)
#define TR069_ENABLE_WRITE_APPLY_ATTR_CONFIG_SAVE       (TR069_ENABLE_WRITE | TR069_ENABLE_APPLY | TR069_ENABLE_ATTR | TR069_ENABLE_CONFIG | TR069_ENABLE_SAVE)

//扩展的参数默认属性设置为 TR069_ENABLE_READ_WRITE_APPLY_ATTR

struct TR069;
struct Param;

typedef unsigned int DateTime;
typedef unsigned int Boolean;
typedef char Base64;

typedef int (*Tr069ValueFunc)(char *, char *, unsigned int);

typedef void (*OnChgFunc)(struct TR069 *, char *);

#define TR069_INTEGER_LEN    12
enum {
    TR069_OBJECT = 0,

    TR069_INT,//整型
    TR069_UNSIGNED,//无符号整型
    TR069_BOOLEAN,//布尔型
    TR069_DATETIME,//时间 CCYY-MM-DDThh:mm:ss
    TR069_STRING,//字符串
    TR069_BASE64,//BASE64加密的字符串，4字节对齐

    TR069_VALUETYPE_MAX
};

enum {
    TR069_STATE_SUSPEND = 0,
    TR069_STATE_ACTIVE,
    TR069_STATE_ERROR_CONNECT,
    TR069_STATE_ERROR_PARSE,
};

#endif//__TR069_DEFINE_H__

