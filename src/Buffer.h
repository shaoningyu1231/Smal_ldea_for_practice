#pragma once
#include <string>

class Buffer
{
private:
    std::string buf;
public:
    Buffer();
    ~Buffer();
    void append(const char* _str, int _size);
    size_t size() const;
    const char* c_str() const;
    void clear();
    void getline();
};
