#pragma once
#include <concepts>

namespace Core
{
    class Math
    {
    private:
        Math() = default;

    public:
        ~Math() = delete;

        Math(const Math& Other) = delete;

        Math(Math&& Other) noexcept = delete;

        Math& operator=(const Math& Other) = delete;

        Math& operator=(Math&& Other) noexcept = delete;

    public:
        template<std::integral T, std::integral U>
        [[nodiscard]] static constexpr std::common_type_t<T, U> IntegerDivCeil(const T Numerator, const U Denominator)
        {
            return Numerator / Denominator + !!(Numerator % Denominator);
        }
    };
}
