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
        /**
         * @brief Performs integer division with ceiling. Handles positive numbers only. Faster than IntegerDivCeil.
         * @param Numerator The numerator.
         * @param Denominator The denominator.
         * @tparam T Integral type of the numerator.
         * @tparam U Integral type of the denominator.
         * @return The result of the division rounded up to the nearest integer.
         */
        template<std::integral T, std::integral U>
        [[nodiscard]] static constexpr std::common_type_t<T, U> IntegerDivCeilPositive(
            const T Numerator, const U Denominator)
        {
            return Numerator / Denominator + !!(Numerator % Denominator);
        }

        /**
         * @brief Performs integer division with ceiling. Handles both positive and negative numbers.
         * @param Numerator The numerator.
         * @param Denominator The denominator.
         * @tparam T Integral type of the numerator.
         * @tparam U Integral type of the denominator.
         * @return The result of the division rounded up to the nearest integer.
         */
        template<std::integral T, std::integral U>
        [[nodiscard]] static constexpr std::common_type_t<T, U> IntegerDivCeil(
            const T Numerator, const U Denominator)
        {
            using V = std::common_type_t<T, U>;
            V quotient = Numerator / Denominator;
            V remainder = Numerator % Denominator;
            return quotient + (remainder != 0 && (Numerator ^ Denominator) >= 0);
        }
    };
}
