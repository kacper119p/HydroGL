#pragma once
#include <concepts>

namespace Core::Math
{
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
    [[nodiscard]] constexpr std::common_type_t<T, U> IntegerDivCeil(
        const T Numerator, const U Denominator)
    {
        using V = std::common_type_t<T, U>;
        V quotient = Numerator / Denominator;
        V remainder = Numerator % Denominator;
        return quotient + (remainder != 0 && (Numerator ^ Denominator) >= 0);
    }

    /**
     * @brief Performs linear interpolation between two floating-point values.
     *
     * @tparam T Floating-point type of the first value.
     * @tparam U Floating-point type of the second value.
     * @param A The starting value.
     * @param B The ending value.
     * @param Alpha The interpolation factor, typically in the range [0, 1].
     * @return A + Alpha * (B - A).
     */
    template<std::floating_point T, std::floating_point U>
    [[nodiscard]] constexpr std::common_type_t<T, U> Lerp(
        const T A, const U B, const std::common_type_t<T, U> Alpha)
    {
        return A + Alpha * (B - A);
    }

    /**
     * @brief Creates a 2D rotation matrix from a given unit 'up' vector.
     * @param NewUp New up vector in old coordinate system space (has to be normalized).
     * @return Rotation matrix that transforms vectors from a new one system to the old one.
     */
    [[nodiscard]] inline glm::mat2 CreateRotationMatrix2DFromUnitVector(const glm::vec2& NewUp)
    {
        return glm::mat2(-NewUp.y, NewUp.x, NewUp.x, NewUp.y);
    }

    /**
     * @brief Creates a 2D rotation matrix from a given unit 'up' vector.
     * @param NewUp New up vector in old coordinate system space (has to be normalized).
     * @return Rotation matrix that transforms vectors from the old coordinate system to a new one.
     */
    [[nodiscard]] inline glm::mat2 CreateInverseRotationMatrix2DFromUnitVector(const glm::vec2& NewUp)
    {
        return glm::transpose(CreateRotationMatrix2DFromUnitVector(NewUp));
    }
}
