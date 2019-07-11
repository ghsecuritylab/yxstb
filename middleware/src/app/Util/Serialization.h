
#pragma once

#include "Archive.h"

namespace Hippo {
/*
 * 序列化
 * 从文件读时会调到Serialize
 * 可以满足 ar << (Serialization&)s;
 */

class Serialization
{
public:
    Serialization() {}
    virtual ~Serialization() {}

    virtual void Serialize(Archive& ar) = 0;
    virtual void Deserialize(Archive& ar) = 0;

    void SaveToFile(const char * file) {
        Archive ar;
        Serialize(ar);
        ar.Save(file);
    }

    void LoadFromFile(const char * file) {
        Archive ar;
        if (ar.Load(file))
            Deserialize(ar);
    }
};

class ArchiveSerializationFunctor {
public:
    ArchiveSerializationFunctor(Archive& a) : ar(a) {}
    void operator()(Serialization* b) {
        if (b)
            ar << *b;
    }
private:
    Archive& ar; 
};

class ArchiveDeserializationFunctor {
public:
    ArchiveDeserializationFunctor(Archive& a) : ar(a) {}
    void operator()(Serialization* b) {
        if (b)
            ar >> *b; 
    }   
private:
    Archive& ar; 
};

} // namespace Hippo
