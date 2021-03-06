#ifndef _NK_VECTOR_H
#define _NK_VECTOR_H

#include <string.h>
#include <kernel/kdebug.h>

namespace nk
{

    template <typename T>
    class Vector
    {
    private:
        T *data = nullptr;
        size_t size = 0;
        size_t capacity = 0;

    public:
        ~Vector()
        {
            for (size_t i = 0; i < size; i++)
                data[i].~T();
            delete[] data;
        }

        void Add(T obj)
        {
            EnsureCapacity(size + 1);
            data[size] = obj;
            size++;
        }

        void Remove(size_t idx)
        {
            for (size_t i = idx + 1; i < size; i++)
            {
                data[i - 1] = data[i];
            }

            size--;
        }

        void RemoveBlock(size_t idx, size_t size)
        {
            memcpy(&data[idx], &data[idx + size], (this->size - (idx + size)) * sizeof(T));
            this->size -= size;
        }

        void Clear()
        {
            size = 0;
        }

        const T &At(size_t index) const
        {
            return data[index];
        }

        T &At(size_t index)
        {
            return data[index];
        }

        T &operator[](size_t index) const
        {
            return data[index];
        }

        size_t Size() const
        {
            return size;
        }

        size_t Capacity() const
        {
            return capacity;
        }

        int IndexOf(const T &value) const
        {
            for (size_t i = 0; i < size; i++)
                if (data[i] == value)
                    return i;
            return -1;
        }

        bool Contains(const T &value) const
        {
            return IndexOf(value) != -1;
        }

        void Reserve(size_t min)
        {
            EnsureCapacity(min);
            if (min > size)
                size = min;
        }

        void EnsureCapacity(size_t min)
        {
            if (min > capacity)
            {
                size_t newCapacity = min * 2;
                T *newData = new T[newCapacity];

                if (this->data != nullptr)
                {
                    for (size_t i = 0; i < size; i++)
                    {
                        newData[i] = data[i];
                        data[i].~T();
                    }
                    delete[] this->data;
                }

                this->data = newData;
                this->capacity = newCapacity;
            }
        }
    };

} // namespace nk

#endif