#pragma once
#include <iterator>

namespace Core
{
    template<typename T, size_t Capacity>
        requires requires { Capacity >= 2; }
    class LoopBuffer
    {
    public:
        class Iterator
        {
        private:
            T* _ptr;

        public:
            using iterator_category = std::contiguous_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            explicit Iterator(T* Ptr) : _ptr(Ptr)
            {
            }

            reference operator*() const
            {
                return *_ptr;
            }

            pointer operator->() const
            {
                return _ptr;
            }

            Iterator& operator++()
            {
                ++_ptr;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator tmp = *this;
                ++_ptr;
                return tmp;
            }

            Iterator& operator--()
            {
                --_ptr;
                return *this;
            }

            Iterator operator--(int)
            {
                Iterator tmp = *this;
                --_ptr;
                return tmp;
            }

            bool operator==(const Iterator& Other) const
            {
                return _ptr == Other._ptr;
            }

            bool operator!=(const Iterator& Other) const
            {
                return _ptr != Other._ptr;
            }
        };

        class ConstIterator
        {
        private:
            const T* _ptr;

        public:
            using iterator_category = std::contiguous_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            explicit ConstIterator(const T* Ptr) : _ptr(Ptr)
            {
            }

            ConstIterator(const Iterator& Iterator) : _ptr(Iterator._ptr)
            {
            }

            reference operator*() const
            {
                return *_ptr;
            }

            pointer operator->() const
            {
                return _ptr;
            }

            ConstIterator& operator++()
            {
                ++_ptr;
                return *this;
            }

            ConstIterator operator++(int)
            {
                ConstIterator tmp = *this;
                ++_ptr;
                return tmp;
            }

            ConstIterator& operator--()
            {
                --_ptr;
                return *this;
            }

            ConstIterator operator--(int)
            {
                ConstIterator tmp = *this;
                --_ptr;
                return tmp;
            }

            bool operator==(const ConstIterator& Other) const
            {
                return _ptr == Other._ptr;
            }

            bool operator!=(const ConstIterator& Other) const
            {
                return _ptr != Other._ptr;
            }
        };

    private:
        T _data[Capacity]{};

    public:
        LoopBuffer() = default;

        LoopBuffer(std::initializer_list<T> InitializerList)
        {
            for (size_t i = 0; i < InitializerList.size(); ++i)
            {
                _data[i] = std::move(*(InitializerList.begin() + i));
            }
        }

    public:
        [[nodiscard]] static constexpr size_t Size()
        {
            return Capacity;
        }

    public:
        void AdvanceDown()
        {
            T tmp = std::move(_data[0]);
            for (size_t i = 1; i < Capacity; ++i)
            {
                _data[i - 1] = std::move(_data[i]);
            }
            _data[Capacity - 1] = std::move(tmp);
        }

        void AdvanceUp()
        {
            T tmp = std::move(_data[Capacity - 1]);
            for (size_t i = Capacity - 1; i > 0; --i)
            {
                _data[i] = std::move(_data[i - 1]);
            }
            _data[0] = std::move(tmp);
        }

        T* Data()
        {
            return _data;
        }

        const T* Data() const
        {
            return _data;
        }

    public:
        Iterator begin()
        {
            return Iterator(_data);
        }

        Iterator end()
        {
            return Iterator(_data + Capacity);
        }

        ConstIterator begin() const
        {
            return ConstIterator(_data);
        }

        Iterator end() const
        {
            return Iterator(_data + Capacity);
        }

        [[nodiscard]] ConstIterator cbegin() const
        {
            return ConstIterator(_data);
        }

        [[nodiscard]] ConstIterator cend() const
        {
            return ConstIterator(_data + Size);
        }

    public:
        T& operator[](const Iterator Iterator)
        {
            return *Iterator;
        }

        T& operator[](const size_t Index)
        {
            return _data[Index];
        }

        [[nodiscard]] const T& operator[](const ConstIterator Iterator) const
        {
            return *Iterator;
        }

        const T& operator[](const Iterator Iterator) const
        {
            return *Iterator;
        }

        const T& operator[](const size_t Index) const
        {
            return _data[Index];
        }
    };
}
