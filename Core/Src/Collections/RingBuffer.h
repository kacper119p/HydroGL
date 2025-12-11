#pragma once

#include <concepts>
#include <iterator>
#include <memory>

#include "Debug.h"

namespace Core
{
    /**
     * @brief A fixed-capacity ring buffer container.
     * @tparam T Type of the elements contained in the buffer.
     * @tparam Capacity The fixed size of the buffer. Must be at least 1.
     */
    template<typename T, size_t Capacity>
        requires (Capacity >= 1)
    class RingBuffer
    {
    private:
        using Slot = std::aligned_storage_t<sizeof(T), alignof(T)>;

    public:
        /**
         * @brief Iterator implementation for RingBuffer, supporting both const and non-const access.
         * @tparam Const Boolean flag indicating if the iterator is constant.
         */
        template<bool Const>
        class IteratorImpl
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = std::conditional_t<Const, const T*, T*>;
            using reference = std::conditional_t<Const, const T&, T&>;

        private:
            using SlotPtr = std::conditional_t<Const, const Slot*, Slot*>;

            SlotPtr _buffer;
            size_t _tail;
            size_t _pos;

            pointer GetTypedPtr(size_t Index) const
            {
                return std::launder(reinterpret_cast<pointer>(&_buffer[Index]));
            }

        public:
            IteratorImpl(SlotPtr Buffer, const size_t Tail, const size_t Pos)
                : _buffer(Buffer), _tail(Tail), _pos(Pos)
            {
            }

            template<bool WasConst, typename = std::enable_if_t<Const && !WasConst>>
            explicit IteratorImpl(const IteratorImpl<false>& Other)
                : _buffer(Other._buffer), _tail(Other._tail), _pos(Other._pos)
            {
            }

            reference operator*() const
            {
                const size_t index = (_tail + _pos) % Capacity;
                return *GetTypedPtr(index);
            }

            pointer operator->() const
            {
                const size_t index = (_tail + _pos) % Capacity;
                return GetTypedPtr(index);
            }

            IteratorImpl& operator++()
            {
                ++_pos;
                return *this;
            }

            IteratorImpl operator++(int)
            {
                IteratorImpl tmp = *this;
                ++_pos;
                return tmp;
            }

            IteratorImpl& operator--()
            {
                --_pos;
                return *this;
            }

            IteratorImpl operator--(int)
            {
                IteratorImpl tmp = *this;
                --_pos;
                return tmp;
            }

            friend bool operator==(const IteratorImpl& Lhs, const IteratorImpl& Rhs)
            {
                return Lhs._pos == Rhs._pos && Lhs._buffer == Rhs._buffer;
            }

            friend bool operator!=(const IteratorImpl& Lhs, const IteratorImpl& Rhs)
            {
                return !(Lhs == Rhs);
            }
        };

        using Iterator = IteratorImpl<false>;
        using ConstIterator = IteratorImpl<true>;

    private:
        Slot _data[Capacity];
        size_t _head;
        size_t _tail;
        bool _full;

    private:
        T* GetPtr(size_t Index)
        {
            return std::launder(reinterpret_cast<T*>(&_data[Index]));
        }

        const T* GetPtr(size_t Index) const
        {
            return std::launder(reinterpret_cast<const T*>(&_data[Index]));
        }

    public:
        /**
         * @brief Default constructor. Initializes an empty ring buffer.
         */
        explicit RingBuffer()
            : _head(0), _tail(0), _full(false)
        {
        }

        /**
         * @brief Copy constructor.
         * Creates a deep copy of the elements from the other buffer.
         * @param Other The buffer to copy from.
         */
        RingBuffer(const RingBuffer& Other)
            : _head(Other._head), _tail(Other._tail), _full(Other._full)
        {
            if (Other.Empty()) return;

            const size_t count = Other.Size();
            size_t current = _tail;

            for (size_t i = 0; i < count; ++i)
            {
                std::construct_at(GetPtr(current), *Other.GetPtr(current));
                current = (current + 1) % Capacity;
            }
        }

        /**
         * @brief Move constructor.
         * Moves elements from the other buffer and resets the source buffer to empty.
         * @param Other The buffer to move from.
         */
        RingBuffer(RingBuffer&& Other) noexcept(std::is_nothrow_move_constructible_v<T>)
            : _head(Other._head), _tail(Other._tail), _full(Other._full)
        {
            if (Other.Empty()) return;

            const size_t count = Other.Size();
            size_t current = _tail;

            for (size_t i = 0; i < count; ++i)
            {
                T* src = Other.GetPtr(current);
                std::construct_at(GetPtr(current), std::move(*src));
                std::destroy_at(src);
                current = (current + 1) % Capacity;
            }

            Other._head = 0;
            Other._tail = 0;
            Other._full = false;
        }

    public:
        /**
         * @brief Destructor. Destroys all elements currently in the buffer.
         */
        ~RingBuffer()
        {
            Clear();
        }

    public:
        /**
         * @brief Pushes a new element to the buffer.
         * Fails if the buffer is full.
         * @tparam U Type of the element (must be convertible to T).
         * @param Item The item to push.
         * @return True if the item was pushed, false if the buffer was full.
         */
        template<typename U> requires std::convertible_to<U, T>
        bool Push(U&& Item)
        {
            if (_full) return false;

            std::construct_at(GetPtr(_head), std::forward<U>(Item));

            _head = (_head + 1) % Capacity;
            _full = (_head == _tail);
            return true;
        }

        /**
         * @brief Pushes a new element to the buffer, overwriting the oldest element if full.
         * @tparam U Type of the element (must be convertible to T).
         * @param Item The item to push.
         */
        template<typename U> requires std::convertible_to<U, T>
        void PushForce(U&& Item)
        {
            if (_full)
            {
                std::destroy_at(GetPtr(_tail));
                _tail = (_tail + 1) % Capacity;
            }

            std::construct_at(GetPtr(_head), std::forward<U>(Item));
            _head = (_head + 1) % Capacity;
            _full = (_head == _tail);
        }

        /**
         * @brief Constructs an element in-place in the buffer.
         * Fails if the buffer is full.
         * @tparam ArgsType Argument types for the element constructor.
         * @param Args Arguments to forward to the constructor.
         * @return True if the element was constructed, false if the buffer was full.
         */
        template<typename... ArgsType>
        bool Emplace(ArgsType&&... Args)
        {
            if (_full) return false;

            std::construct_at(GetPtr(_head), std::forward<ArgsType>(Args)...);

            _head = (_head + 1) % Capacity;
            _full = (_head == _tail);
            return true;
        }

        /**
         * @brief Constructs an element in-place in the buffer, overwriting the oldest element if full.
         * @tparam ArgsType Argument types for the element constructor.
         * @param Args Arguments to forward to the constructor.
         */
        template<typename... ArgsType>
        void EmplaceForce(ArgsType&&... Args)
        {
            if (_full)
            {
                std::destroy_at(GetPtr(_tail));
                _tail = (_tail + 1) % Capacity;
            }

            std::construct_at(GetPtr(_head), std::forward<ArgsType>(Args)...);

            _head = (_head + 1) % Capacity;
            _full = (_head == _tail);
        }

        /**
         * @brief Removes and returns the oldest element from the buffer.
         * Undefined behavior if the buffer is empty.
         * @return The removed element.
         */
        T Pop()
        {
            CHECK(!Empty(), "Cannot pop from an empty RingBuffer!");
            T* itemPtr = GetPtr(_tail);
            T result = std::move(*itemPtr);
            std::destroy_at(itemPtr);
            _tail = (_tail + 1) % Capacity;
            _full = false;
            return result;
        }

        /**
         * @brief Returns a reference to the oldest element without removing it.
         * @return Constant reference to the oldest element.
         */
        [[nodiscard]] const T& Peek() const
        {
            CHECK(!Empty(), "Cannot peek from an empty RingBuffer!");
            return *GetPtr(_tail);
        }

        /**
         * @brief Returns a reference to the oldest element without removing it.
         * @return Reference to the oldest element.
         */
        [[nodiscard]] T& Peek()
        {
            CHECK(!Empty(), "Cannot peek from an empty RingBuffer!");
            return *GetPtr(_tail);
        }

        /**
         * @brief Clears the buffer, destroying all contained elements.
         */
        void Clear()
        {
            while (!Empty())
            {
                std::destroy_at(GetPtr(_tail));
                _tail = (_tail + 1) % Capacity;
                if (_head == _tail) _full = false;
            }
            _head = 0;
            _tail = 0;
            _full = false;
        }

        /**
         * @brief Checks if the buffer is empty.
         * @return True if empty, false otherwise.
         */
        [[nodiscard]] bool Empty() const
        {
            return !_full && _head == _tail;
        }

        /**
         * @brief Checks if the buffer is full.
         * @return True if full, false otherwise.
         */
        [[nodiscard]] bool Full() const
        {
            return _full;
        }

        /**
         * @brief Gets the fixed capacity of the buffer.
         * @return The capacity of the buffer.
         */
        [[nodiscard]] static constexpr size_t GetCapacity()
        {
            return Capacity;
        }

        /**
         * @brief Calculates the number of elements currently in the buffer.
         * @return The number of elements.
         */
        [[nodiscard]] size_t Size() const
        {
            if (_full) return Capacity;
            if (_head >= _tail) return _head - _tail;
            return Capacity + _head - _tail;
        }

    public:
        /**
         * @brief Returns an iterator to the beginning of the buffer.
         * @return Iterator to the beginning.
         */
        [[nodiscard]] Iterator begin()
        {
            return Iterator(_data, _tail, 0);
        }

        /**
         * @brief Returns an iterator to the end of the buffer.
         * @return Iterator to the end.
         */
        [[nodiscard]] Iterator end()
        {
            return Iterator(_data, _tail, Size());
        }

        /**
         * @brief Returns a constant iterator to the beginning of the buffer.
         * @return Constant iterator to the beginning.
         */
        [[nodiscard]] ConstIterator begin() const
        {
            return ConstIterator(_data, _tail, 0);
        }

        /**
         * @brief Returns a constant iterator to the end of the buffer.
         * @return Constant iterator to the end.
         */
        [[nodiscard]] ConstIterator end() const
        {
            return ConstIterator(_data, _tail, Size());
        }

        /**
         * @brief Returns a constant iterator to the beginning of the buffer.
         * @return Constant iterator to the beginning.
         */
        [[nodiscard]] ConstIterator cbegin() const
        {
            return ConstIterator(_data, _tail, 0);
        }

        /**
         * @brief Returns a constant iterator to the end of the buffer.
         * @return Constant iterator to the end.
         */
        [[nodiscard]] ConstIterator cend() const
        {
            return ConstIterator(_data, _tail, Size());
        }

        /**
         * @brief Copy assignment operator.
         * Destroys current elements and copies from the other buffer.
         * @param Other The buffer to copy from.
         * @return Reference to this buffer.
         */
        RingBuffer& operator=(const RingBuffer& Other)
        {
            if (this == &Other) return *this;

            Clear();

            _head = Other._head;
            _tail = Other._tail;
            _full = Other._full;

            if (Other.Empty()) return *this;

            size_t count = Other.Size();
            size_t current = _tail;

            for (size_t i = 0; i < count; ++i)
            {
                std::construct_at(GetPtr(current), *Other.GetPtr(current));
                current = (current + 1) % Capacity;
            }

            return *this;
        }

        /**
         * @brief Move assignment operator.
         * Destroys current elements, moves from the other buffer, and resets the source.
         * @param Other The buffer to move from.
         * @return Reference to this buffer.
         */
        RingBuffer& operator=(RingBuffer&& Other)
            noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>)
        {
            if (this == &Other) return *this;

            Clear();

            _head = Other._head;
            _tail = Other._tail;
            _full = Other._full;

            if (Other.Empty()) return *this;

            const size_t count = Other.Size();
            size_t current = _tail;

            for (size_t i = 0; i < count; ++i)
            {
                T* src = Other.GetPtr(current);
                std::construct_at(GetPtr(current), std::move(*src));
                std::destroy_at(src);
                current = (current + 1) % Capacity;
            }

            Other._head = 0;
            Other._tail = 0;
            Other._full = false;

            return *this;
        }
    };
}
