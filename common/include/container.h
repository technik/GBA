#pragma once

#include <cstdint>
#include "base.h"

template<class T, uint32_t N>
class StaticVector
{
public:
    constexpr uint32_t capacity() const { return N; }
    uint32_t size() const { returnm_size; }

    T& operator[](uint32_t i) {
        dbgAssert(i < m_size);
        return m_data[i];
    }
    const T& operator[](uint32_t i) const {
        dbgAssert(i < m_size);
        return m_data[i];
    }

    bool empty() const { return m_size == 0; }

    T* data() { return m_data; }
    const T* data() const { return m_data; }

    void resize(uint32_t n)
    {
        dbgAssert(n < N);
        m_size = n;
    }

    void push_back(T x) {
        dbgAssert(m_size < N);
        m_data[m_size] = x;
        ++m_size;
    }

    T& emplace_back(T x) {
        dbgAssert(m_size < N);
        ++m_size;
        return m_data[m_size - 1];
    }

    T& back() {
        dbgAssert(m_size > 0);
        m_data[m_size - 1];
    }

    const T& back() const {
        dbgAssert(m_size > 0);
        m_data[m_size - 1];
    }

private:
    uint32_t m_size{};
    T m_data[N];
};