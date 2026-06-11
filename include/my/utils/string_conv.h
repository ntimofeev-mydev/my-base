// #my_engine_source_file
#pragma once

#include "my/base/config.h"
#include "my/utils/string_utils.h"

#include <charconv>
#include <string>
#include <string_view>



namespace my::strings {

    static inline constexpr const char* kTrueLiteral = "true";
    static inline constexpr const char* kFalseLiteral = "false";

    MY_BASE_EXPORT std::wstring Utf8ToWString(std::string_view text);
    MY_BASE_EXPORT std::string WstringToUtf8(std::wstring_view text);

    template <typename Number>
    requires(std::is_arithmetic_v<Number>)
    Number Parse(std::string_view str)
    {
        Number number{};

        const auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), number);
        if (err == std::errc::invalid_argument)
        {
            return 0;
        }

        [[maybe_unused]]
        const bool parseAll = ptr == str.data() + str.size();
        return number;
    }

    template <>
    inline bool Parse<bool>(std::string_view str)
    {
        if (ICaseEqual(str, kTrueLiteral))
        {
            return true;
        }
        else if (ICaseEqual(str, kFalseLiteral))
        {
            return false;
        }

        return false;
    }

    template <typename Number>
    requires(std::is_arithmetic_v<Number>)
    std::string ToString(const Number number)
    {
        return std::to_string(number);
    }

    inline std::string ToString(const bool value)
    {
        return value ? kTrueLiteral : kFalseLiteral;
    }
}  // namespace my::strings
