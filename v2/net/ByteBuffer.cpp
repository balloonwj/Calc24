#include "ByteBuffer.h"

#include <string.h> //for memcpy

ByteBuffer::operator const char* () {
    return m_internalBuf.c_str();
}

void ByteBuffer::append(const char* buf, int bufLen) {
    m_internalBuf.append(buf, bufLen);
}

void ByteBuffer::retrieve(std::string& outBuf, size_t bufLen/* = 0*/) {
    if (bufLen == 0 || bufLen >= m_internalBuf.length()) {
        outBuf = std::move(m_internalBuf);
        return;
    }

    outBuf = m_internalBuf.substr(0, bufLen);
    m_internalBuf.erase(0, bufLen);
}

size_t ByteBuffer::retrieve(char* buf, size_t bufLen) {
    if (bufLen == 0) {
        return 0;
    }

    if (bufLen >= m_internalBuf.size()) {
        bufLen = m_internalBuf.size();
    }

    memcpy(buf, m_internalBuf.c_str(), bufLen);
    m_internalBuf.erase(0, bufLen);

    return bufLen;
}

size_t ByteBuffer::peek(char* buf, size_t bufLen) {
    if (bufLen == 0) {
        return 0;
    }

    if (bufLen >= m_internalBuf.size()) {
        bufLen = m_internalBuf.size();
    }

    memcpy(buf, m_internalBuf.c_str(), bufLen);

    return bufLen;
}

void ByteBuffer::erase(size_t bufLen/* = 0*/) {
    if (bufLen == 0 || bufLen >= m_internalBuf.length()) {
        m_internalBuf.clear();
        return;
    }

    m_internalBuf.erase(0, bufLen);
}

size_t ByteBuffer::remaining() {
    return m_internalBuf.length();
}

void ByteBuffer::clear() {
    m_internalBuf.clear();
}

bool ByteBuffer::isEmpty() {
    return m_internalBuf.empty();
}