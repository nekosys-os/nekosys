#ifndef _VECTOR_H
#define _VECTOR_H

#include <stdlib.h>
#include <string.h>

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

        const T &At(size_t index) const
        {
            return data[index];
        }

        T &At(size_t index)
        {
            return data[index];
        }

        size_t Size()
        {
            return size;
        }

        size_t Capacity()
        {
            return capacity;
        }

        ~Vector()
        {
            free((void *)data);
        }

    private:
        void EnsureCapacity(size_t min)
        {
            if (min > capacity)
            {
                size_t newCapacity = min * 2;
                T *newData = (T *)malloc(sizeof(T) * newCapacity);
                if (this->data != nullptr)
                {
                    memcpy(newData, data, sizeof(T) * size);
                }
                free((void *)this->data);
                this->data = newData;
                this->capacity = newCapacity;
            }
        }
    };

} // namespace nk

#endif