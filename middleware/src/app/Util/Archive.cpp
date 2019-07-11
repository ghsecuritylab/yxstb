

#include "Archive.h"
#include "Serialization.h"

namespace Hippo { 

Archive::Archive()
{
}

Archive::~Archive()
{
    reset();
}

Archive::Archive(const Archive& o)
{
}

Archive& Archive::operator=(const Archive& o)
{
    return *this;
}

Archive& Archive::operator<<(std::string str) {
    Data* data = new Data(str.length() + 1, str.c_str());
    mDataList.push_back(data);
    return *this;
}

Archive& Archive::operator>>(std::string& str) {
    if (mDataList.empty())
        return *this;
    Data* data = mDataList.front();
    mDataList.pop_front();
    str = std::string((char *)data->getData());
    delete data;
    return *this;
}
    
Archive& Archive::operator<<(Serialization& s)
{
    s.Serialize(*this);
    return *this;
}

Archive& Archive::operator>>(Serialization& s)
{
    s.Deserialize(*this);
    return *this;
}

void Archive::Save(const char * file)
{
    FILE * fp = fopen(file, "wb+");
    if (!fp) {
        return;
    }
    std::list<Data*>::const_iterator it;
    for (it = mDataList.begin(); it != mDataList.end(); it++) {
        (*it)->Save(fp);
    }
    fclose(fp);
}

bool Archive::Load(const char * file)
{
    reset();
    FILE * fp = fopen(file, "rb");
    if (!fp) {
        return false;
    }
    while (!feof(fp)) {
        Data* data = new Data(fp);
        mDataList.push_back(data);
    }
    fclose(fp);
    return true;
}

void Archive::reset()
{
    if (mDataList.empty())
        return;
    std::list<Data*>::const_iterator it;
    for (it = mDataList.begin(); it != mDataList.end(); it++) {
        delete *it;
    }
    mDataList.clear();
}

Archive::Data::Data(int l, const void * d)
    : data(NULL)
{
    assign(l, d);
}

Archive::Data::~Data()
{
    assign(0, NULL);
}

Archive::Data::Data(const Data& o)
    : data(NULL)
{
    assign(o.len, o.data);
}

Archive::Data::Data(FILE* fp)
    : len(0)
    , data(NULL)
{
    fread(&len, 1, sizeof(len), fp);
    if (len > 0) {
        data = new char[len];
        fread(data, 1, len, fp);
    }
}

Archive::Data& Archive::Data::operator=(const Data& o)
{
    assign(o.len, o.data);
    return *this;
}

void Archive::Data::assign(int l, const void * d)
{
    if (data)
        delete [] data;
    len = 0;
    data = NULL;
    if (l <= 0)
        return;

    len = l;
    data = new char[len];
    memcpy(data, d, len);
}

void Archive::Data::dump(void* p, int l)
{
    if (l > len)
        l = len;
    memcpy(p, data, l);
}

void Archive::Data::Save(FILE* fp)
{
    fwrite(&len, 1, sizeof(len), fp);
    fwrite(data, 1, len, fp);
}

} // namespace Hipo

