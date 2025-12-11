#pragma once

#include <concepts>
#include <iterator>
#include <memory>

#include "Debug.h"

namespace Core
{
    template<typename T, size_t Capacity>
        requires (Capacity >= 1)
    class RingBuffer
    {
    private:
        using Slot = std::aligned_storage_t<sizeof(T), alignof(T)>;

    public:
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
        explicit RingBuffer()
            : _head(0), _tail(0), _full(false)
        {
        }

    public:
        ~RingBuffer()
        {
            while (!Empty())
            {
                std::destroy_at(GetPtr(_tail));
                _tail = (_tail + 1) % Capacity;
                if (_head == _tail) _full = false;
            }
        }

    public:
        template<typename U> requires std::convertible_to<U, T>
        bool Push(U&& Item)
        {
            if (_full) return false;

            std::construct_at(GetPtr(_head), std::forward<U>(Item));

            _head = (_head + 1) % Capacity;
            _full = (_head == _tail);
            return true;
        }

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

        template<typename... ArgsType>
        bool Emplace(ArgsType&&... Args)
        {
            if (_full) return false;

            std::construct_at(GetPtr(_head), std::forward<ArgsType>(Args)...);

            _head = (_head + 1) % Capacity;
            _full = (_head == _tail);
            return true;
        }

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

        [[nodiscard]] const T& Peek() const noexcept
        {
            CHECK(!Empty(), "Cannot peek from an empty RingBuffer!");
            return *GetPtr(_tail);
        }

        [[nodiscard]] T& Peek() noexcept
        {
            CHECK(!Empty(), "Cannot peek from an empty RingBuffer!");
            return *GetPtr(_tail);
        }

        void Clear()
        {
            _head = _tail;
            _full = false;
        }

        [[nodiscard]] bool Empty() const
        {
            return (!_full && (_head == _tail));
        }

        [[nodiscard]] bool Full() const
        {
            return _full;
        }

        [[nodiscard]] static constexpr size_t GetCapacity()
        {
            return Capacity;
        }

        [[nodiscard]] size_t Size() const
        {
            if (_full) return Capacity;
            if (_head >= _tail) return _head - _tail;
            return Capacity + _head - _tail;
        }

    public:
        [[nodiscard]] Iterator begin()
        {
            return Iterator(_data, _tail, 0);
        }

        [[nodiscard]] Iterator end()
        {
            return Iterator(_data, _tail, Size());
        }

        [[nodiscard]] ConstIterator begin() const
        {
            return ConstIterator(_data, _tail, 0);
        }

        [[nodiscard]] ConstIterator end() const
        {
            return ConstIterator(_data, _tail, Size());
        }

        [[nodiscard]] ConstIterator cbegin() const
        {
            return ConstIterator(_data, _tail, 0);
        }

        [[nodiscard]] ConstIterator cend() const
        {
            return ConstIterator(_data, _tail, Size());
        }
    };
}
