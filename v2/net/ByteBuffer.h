/**
 * 2024.03.14 zhangxf
 * 自定义缓冲区类
 */
#pragma once

#include <string>

class ByteBuffer {
public:
    ByteBuffer() = default;
    ~ByteBuffer() = default;

    //TODO: 待完成
    ByteBuffer(const ByteBuffer& rhs);
    ByteBuffer& operator=(const ByteBuffer& rhs);

    ByteBuffer(ByteBuffer&& rhs);
    ByteBuffer& operator=(ByteBuffer&& rhs);

public:
    operator const char* ();

    void append(const char* buf, int bufLen);
    void retrieve(std::string& outBuf, size_t bufLen = 0);
    size_t retrieve(char* buf, size_t bufLen);

    size_t peek(char* buf, size_t bufLen);

    void erase(size_t bufLen = 0);

    //寻找\n
    size_t findLF();

    size_t  remaining();

    void clear();

    bool isEmpty();

private:
    std::string  m_internalBuf;
};
