/*
 * This file is part of the CitizenFX project - http://citizen.re/
 */

#pragma once
#include <algorithm>
#include <memory>

template<typename TValue>
class atArray
{
public:
    TValue* m_offset;
    uint16_t m_count;
    uint16_t m_size;

public:
    atArray()
    {
        m_offset = (TValue*)0;
        m_count = 0;
        m_size = 0;
    }

    // atArray(const atArray& right)
    // {
    //     m_count = right.m_count;
    //     m_size = right.m_size;
    // 
    //     m_offset = (TValue*)rage::GetAllocator()->allocate(m_size * sizeof(TValue), 16, 0);
    //     std::uninitialized_copy(right.m_offset, right.m_offset + right.m_count, m_offset);
    // }

    // atArray(int capacity)
    // {
    //     m_offset = (TValue*)rage::GetAllocator()->allocate(capacity * sizeof(TValue), 16, 0);
    //     m_count = 0;
    //     m_size = capacity;
    // }

    // ~atArray()
    // {
    //     std::destroy(m_offset, m_offset + m_count);
    // 
    //     rage::GetAllocator()->free(m_offset);
    // }

    inline uint16_t GetCount()
    {
        return m_count;
    }

    inline uint16_t GetSize()
    {
        return m_size;
    }

    TValue& Get(uint16_t offset)
    {
        if (offset >= m_size)
        {
#ifdef _DEBUG
            assert(!"atArray index out of bounds");
#endif
        }

        return m_offset[offset];
    }

    // void Expand(uint16_t newSize)
    // {
    //     if (m_size >= newSize)
    //     {
    //         return;
    //     }
    // 
    //     TValue* newOffset = (TValue*)rage::GetAllocator()->allocate(newSize * sizeof(TValue), 16, 0);
    // 
    //     // initialize the new entries
    //     std::uninitialized_fill(newOffset, newOffset + newSize, TValue());
    // 
    //     // copy the existing entries
    //     if (m_offset)
    //     {
    //         std::copy(m_offset, m_offset + m_count, newOffset);
    // 
    //         rage::GetAllocator()->free(m_offset);
    //     }
    // 
    //     m_offset = newOffset;
    //     m_size = newSize;
    // }

    // void Clear()
    // {
    //     m_size = 0;
    //     m_count = 0;
    // 
    //     if (m_offset)
    //     {
    //         rage::GetAllocator()->free(m_offset);
    // 
    //         m_offset = nullptr;
    //     }
    // }

    void ClearCount()
    {
        m_count = 0;
    }

    //void Set(uint16_t offset, const TValue& value)
    //{
    //    if (offset >= m_size)
    //    {
    //        Expand(std::max((int)m_size, 2) * 2);
    //    }
    //
    //    if (offset >= m_count)
    //    {
    //        m_count = offset + 1;
    //    }
    //
    //    m_offset[offset] = value;
    //}

    void Remove(int index)
    {
        for (int i = index; i < (m_count - 1); i++)
        {
            m_offset[i] = m_offset[i + 1];
        }

        m_count--;
    }

    auto begin()
    {
        return &m_offset[0];
    }

    auto end()
    {
        return &m_offset[m_count];
    }

    auto& operator[](uint16_t idx)
    {
        return Get(idx);
    }
};

#pragma pack(push)
template <typename T, uint32_t Size>
class atFixedArray // actually namespace rage...
{
public:
    constexpr uint32_t size() const
    {
        return m_size;
    }

    T* begin() const
    {
        return &m_offset[0];
    }

    T* end() const
    {
        return &m_offset[m_size];
    }

    T* data() const
    {
        return m_offset;
    }

    T& at(uint16_t index)
    {
        return m_offset[index];
    }

    T& operator[](std::uint16_t index)
    {
        return m_offset[index];
    }

    T m_offset[Size];
    uint32_t m_size = 0;
};
#pragma pack(pop)