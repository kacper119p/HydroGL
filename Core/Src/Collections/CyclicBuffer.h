#pragma once
#include <iterator>

namespace Core
{
    /**
     * @brief A fixed-capacity buffer container that supports cyclic rotation of its elements.
     * @tparam T Type of the elements contained in the buffer.
     * @tparam Capacity The fixed size of the buffer. Must be at least 2.
     */
    template<typename T, size_t Capacity>
        requires requires { Capacity >= 2; }
    class CyclicBuffer
    {
    public:
        /**
         * @brief Random access iterator for CyclicBuffer.
         */
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

        /**
         * @brief Constant random access iterator for CyclicBuffer.
         */
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
        /**
         * @brief Default constructor. Initializes the buffer with default values.
         */
        CyclicBuffer() = default;

        /**
         * @brief Constructs the buffer from an initializer list.
         * @param InitializerList List of elements to initialize the buffer with.
         */
        CyclicBuffer(std::initializer_list<T> InitializerList)
        {
            for (size_t i = 0; i < InitializerList.size(); ++i)
            {
                _data[i] = std::move(*(InitializerList.begin() + i));
            }
        }

    public:
        /**
         * @brief Gets the fixed capacity of the buffer.
         * @return The capacity of the buffer.
         */
        [[nodiscard]] static constexpr size_t Size()
        {
            return Capacity;
        }

    public:
        /**
         * @brief Rotates the buffer contents to the left.
         * The element at index 0 moves to the end (index Capacity - 1).
         * All other elements shift to the left (index - 1).
         */
        void AdvanceLeft()
        {
            T tmp = std::move(_data[0]);
            for (size_t i = 1; i < Capacity; ++i)
            {
                _data[i - 1] = std::move(_data[i]);
            }
            _data[Capacity - 1] = std::move(tmp);
        }

        /**
         * @brief Rotates the buffer contents to the right.
         * The last element (index Capacity - 1) moves to index 0.
         * All other elements shift to the right (index + 1).
         */
        void AdvanceRight()
        {
            T tmp = std::move(_data[Capacity - 1]);
            for (size_t i = Capacity - 1; i > 0; --i)
            {
                _data[i] = std::move(_data[i - 1]);
            }
            _data[0] = std::move(tmp);
        }

        /**
         * @brief Gets a pointer to the underlying data array.
         * @return Pointer to the data.
         */
        T* Data()
        {
            return _data;
        }

        /**
         * @brief Gets a constant pointer to the underlying data array.
         * @return Constant pointer to the data.
         */
        const T* Data() const
        {
            return _data;
        }

    public:
        /**
         * @brief Returns an iterator to the beginning of the buffer.
         * @return Iterator to the first element.
         */
        Iterator begin()
        {
            return Iterator(_data);
        }

        /**
         * @brief Returns an iterator to the end of the buffer.
         * @return Iterator to the element following the last element.
         */
        Iterator end()
        {
            return Iterator(_data + Capacity);
        }

        /**
         * @brief Returns a constant iterator to the beginning of the buffer.
         * @return Constant iterator to the first element.
         */
        ConstIterator begin() const
        {
            return ConstIterator(_data);
        }

        /**
         * @brief Returns an iterator to the end of the buffer (const context).
         * @return Iterator to the element following the last element.
         */
        Iterator end() const
        {
            return Iterator(_data + Capacity);
        }

        /**
         * @brief Returns a constant iterator to the beginning of the buffer.
         * @return Constant iterator to the first element.
         */
        [[nodiscard]] ConstIterator cbegin() const
        {
            return ConstIterator(_data);
        }

        /**
         * @brief Returns a constant iterator to the end of the buffer.
         * @return Constant iterator to the element following the last element.
         */
        [[nodiscard]] ConstIterator cend() const
        {
            return ConstIterator(_data + Size);
        }

    public:
        /**
         * @brief Accesses an element using an iterator.
         * @param Iterator The iterator pointing to the element.
         * @return Reference to the element.
         */
        T& operator[](const Iterator Iterator)
        {
            return *Iterator;
        }

        /**
         * @brief Accesses an element by index.
         * @param Index The index of the element.
         * @return Reference to the element.
         */
        T& operator[](const size_t Index)
        {
            return _data[Index];
        }

        /**
         * @brief Accesses an element using a constant iterator.
         * @param Iterator The constant iterator pointing to the element.
         * @return Constant reference to the element.
         */
        [[nodiscard]] const T& operator[](const ConstIterator Iterator) const
        {
            return *Iterator;
        }

        /**
         * @brief Accesses an element using an iterator (const context).
         * @param Iterator The iterator pointing to the element.
         * @return Constant reference to the element.
         */
        const T& operator[](const Iterator Iterator) const
        {
            return *Iterator;
        }

        /**
         * @brief Accesses an element by index (const context).
         * @param Index The index of the element.
         * @return Constant reference to the element.
         */
        const T& operator[](const size_t Index) const
        {
            return _data[Index];
        }
    };
}
