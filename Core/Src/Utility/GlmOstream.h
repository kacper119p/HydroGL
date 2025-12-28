#pragma once
#include <cstdint>
#include <ostream>
#include <glm/detail/qualifier.hpp>

/**
 * @brief Outputs a glm vector to an ostream.
 * @param Ostream Stream to output to.
 * @param Vector Vector to output.
 * @return Reference to the output stream.
 */
template<int32_t L, typename T, glm::qualifier Q>
std::ostream& operator<<(std::ostream& Ostream, const glm::vec<L, T, Q>& Vector)
{
    Ostream << "{";
    for (int32_t i = 0; i < L; ++i)
    {
        Ostream << Vector[i] << (i < L - 1 ? ", " : "");
    }
    Ostream << "}";
    return Ostream;
}

/**
 * @brief Outputs a glm matrix to an ostream in a column-major order.
 * @param Ostream Stream to output to.
 * @param Matrix Matrix to output.
 * @return Reference to the output stream.
 */
template<int32_t C, int32_t R, typename T, glm::qualifier Q>
std::ostream& operator<<(std::ostream& Ostream, const glm::mat<C, R, T, Q>& Matrix)
{
    Ostream << "{";
    for (int32_t c = 0; c < C; ++c)
    {
        Ostream << "{";
        for (int32_t r = 0; r < R; ++r)
        {
            Ostream << Matrix[c][r] << (r < R - 1 ? ", " : "");
        }
        Ostream << "}" << (c < C - 1 ? ", " : "");
    }
    Ostream << "}";
    return Ostream;
}

/**
 * @brief Outputs a glm quaternion to an ostream.
 * Output order is {w, x, y, z}.
 * @param Ostream Stream to output to.
 * @param Qua Quaternion to output.
 * @return Reference to the output stream.
 */
template<typename T, glm::qualifier Q>
std::ostream& operator<<(std::ostream& Ostream, const glm::qua<T, Q>& Qua)
{
    Ostream << "{" << Qua.w << ", " << Qua.x << ", " << Qua.y << ", " << Qua.z << "}";
    return Ostream;
}
