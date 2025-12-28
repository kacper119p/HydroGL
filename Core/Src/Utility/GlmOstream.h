#pragma once
#include <ostream>
#include <glm/detail/qualifier.hpp>
#include <glm/detail/setup.hpp>

/**
 * @brief Outputs a glm vector to an ostream.
 * @param Ostream Stream to output to.
 * @param Vector Vector to output.
 * @return Reference to the output stream.
 */
template<glm::length_t L, typename T, glm::qualifier Q>
std::ostream& operator<<(std::ostream& Ostream, const glm::vec<L, T, Q>& Vector)
{
    Ostream << "{";
    for (glm::length_t i = 0; i < L; ++i)
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
template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
std::ostream& operator<<(std::ostream& Ostream, const glm::mat<C, R, T, Q>& Matrix)
{
    Ostream << "{";
    for (glm::length_t c = 0; c < C; ++c)
    {
        Ostream << "{";
        for (glm::length_t r = 0; r < R; ++r)
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
