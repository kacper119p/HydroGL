#pragma once
#include <cstdint>
#include <string_view>

namespace Core::Hash
{
    /**
     * @brief Computes the FNV-1a hash of a given string at compile time.
     *
     * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
     *
     * @param String The input string to hash.
     * @return The computed hash value as a 64-bit unsigned integer.
     */
    [[nodiscard]] constexpr uint64_t HashString64(const std::string_view String)
    {
        constexpr uint64_t fnvOffsetBasis = 0xcbf29ce484222325;
        constexpr uint64_t fnvPrime = 0x00000100000001b3;

        uint64_t hash = fnvOffsetBasis;
        for (const char ch : String)
        {
            hash ^= static_cast<uint64_t>(ch);
            hash *= fnvPrime;
        }

        return hash;
    }

    /**
     * @brief Computes the FNV-1a hash of a given string at compile time.
     *
     * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
     *
     * @param String The input string to hash.
     * @return The computed hash value as a 32-bit unsigned integer.
     */
    [[nodiscard]] constexpr uint32_t HashString32(const std::string_view String)
    {
        constexpr uint32_t fnvOffsetBasis = 0x811c9dc5;
        constexpr uint32_t fnvPrime = 0x01000193;

        uint32_t hash = fnvOffsetBasis;
        for (const char ch : String)
        {
            hash ^= static_cast<uint32_t>(ch);
            hash *= fnvPrime;
        }

        return hash;
    }

    /**
     * @brief Folds a 64-bit hash into a 32-bit hash.
     * @param Hash The 64-bit hash to fold.
     * @return The folded 32-bit hash.
     */
    [[nodiscard]] constexpr uint32_t FoldHash(const uint64_t Hash)
    {
        return static_cast<uint32_t>(Hash >> 32) ^ static_cast<uint32_t>(Hash);
    }
}
