#pragma once

#include <memory>
#include <optional>

namespace Core
{
    template<typename T, size_t Capacity>
        requires requires { Capacity >= 1; }
    class RingBuffer
    {
    private:
        T _data[Capacity];
        size_t _head;
        size_t _tail;
        bool _full;

    public:
        explicit RingBuffer()
            : _head(0), _tail(0), _full(false)
        {
        }

        template<typename U> requires std::convertible_to<U, T>
        bool Push(U&& Item)
        {
            if (_full) return false;

            _data[_head] = std::forward<U>(Item);
            _head = (_head + 1) % GetCapacity;
            _full = (_head == _tail);
            return true;
        }

        template<typename U> requires std::convertible_to<U, T>
        void PushOverwrite(U&& Item)
        {
            _data[_head] = std::forward<U>(Item);

            if (_full)
            {
                _tail = (_tail + 1) % GetCapacity;
            }

            _head = (_head + 1) % GetCapacity;
            _full = (_head == _tail);
        }

        std::optional<T> Pop()
        {
            if (Empty()) return std::nullopt;

            T item = std::move(_data[_tail]);
            _tail = (_tail + 1) % GetCapacity;
            _full = false;
            return item;
        }

        void Reset()
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

        [[nodiscard]] static size_t GetCapacity()
        {
            return GetCapacity;
        }

        [[nodiscard]] size_t Size() const
        {
            if (_full) return Capacity;
            if (_head >= _tail) return _head - _tail;
            return Capacity + _head - _tail;
        }
    };
}
