

#pragma once

#include <sstream>
#include <string>
#include <list>
#include <string.h>
#include <stdio.h>

#include "iptv_logging.h"

namespace Hippo {

/*
 * 序列化的IO类
 * 简单地提供按[长度][内容]的格式塞进文件的方案。
 * 需扩展时再重构。
 */

class Serialization;
class Archive
{
public:
    Archive();
    ~Archive();

    template <class T>
        Archive& operator<<(T t) {
            Data* data = new Data(sizeof(T), &t);
            mDataList.push_back(data);
            return *this;
        }

    template <class T>
        Archive& operator>>(T& t) {
            if (mDataList.empty())
                return *this;
            Data* data = mDataList.front();
            mDataList.pop_front();
            data->dump(&t, sizeof(T));
            delete data;
            return *this;
        }

    Archive& operator<<(std::string str);
    Archive& operator>>(std::string& str);
    Archive& operator<<(Serialization& s);
    Archive& operator>>(Serialization& s);

    void Save(const char * file);
    bool Load(const char * file);

public:
    typedef enum {
        T_UNKNOWN,
        T_SCHAR,
        T_UCHAR,
        T_SHORT,
        T_USHORT,
        T_INT,
        T_UINT,
        T_LONG,
        T_ULONG,
        T_FLOAT,
        T_DOUBLE,
        T_LONGLONG,
        T_ULONGLONG,
        T_STR,
    } TYPE;

    template <class T> TYPE gettype(T t) { return T_UNKNOWN; }
    TYPE gettype(signed char& t) { return T_SCHAR; }
    TYPE gettype(unsigned char& t) { return T_UCHAR; }
    TYPE gettype(short int& t) { return T_SHORT; }
    TYPE gettype(unsigned short int& t) { return T_USHORT; }
    TYPE gettype(int& t) { return T_INT; }
    TYPE gettype(unsigned int& t) { return T_UINT; }
    TYPE gettype(long& t) { return T_LONG; }
    TYPE gettype(unsigned long& t) { return T_ULONG; }
    TYPE gettype(float& t) { return T_FLOAT; }
    TYPE gettype(double& t) { return T_DOUBLE; }
    TYPE gettype(long long& t) { return T_LONGLONG; }
    TYPE gettype(unsigned long long& t) { return T_ULONGLONG; }
    TYPE gettype(std::string& t) { return T_STR; }

private:
    Archive(const Archive& o);
    Archive& operator=(const Archive& o);

    void reset();

    class Data {
    public:
        Data(int l, const void * d);
        Data(const Data& o);
        Data(FILE* fp);
        ~Data();

        Data& operator=(const Data& o);

        void assign(int l, const void * d);
        void dump(void* p, int l);
        void Save(FILE* fp);

        int getLength() { return len; }
        void * getData() { return (void*)data; }
    private:
        Data() {}
        int len;
        char *  data;
    };
    std::list<Data*> mDataList;
};

} // namespace Hippo


