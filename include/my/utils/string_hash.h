// #my_engine_source_file

#pragma once

namespace my::strings {

    /**
        @brief FNV-1a
    */
    constexpr inline size_t ConstHash(const char* input)
    {
        constexpr size_t kPrime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;
        size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;

        while (*input)
        {
            hash ^= static_cast<size_t>(*input);
            hash *= kPrime;
            ++input;
        }

        return hash;
    }
}  // namespace my::strings
