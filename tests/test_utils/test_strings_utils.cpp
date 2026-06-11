// #my_engine_source_file

#include "my/utils/string_utils.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace my::test
{

    TEST(StringUtils, SplitEmptyString)
    {
        const strings::SplitSequence seq = strings::Split(strings::kEmptyStr, ";");

        const auto count = std::distance(seq.begin(), seq.end());
        ASSERT_EQ(count, 0);
    }

    TEST(StringUtils, SplitSingleSeparator)
    {
        const char* kStr = "first";
        const strings::SplitSequence seq = strings::Split(kStr, ";");
        auto iter = seq.begin();
        ASSERT_NE(iter, seq.end());
        EXPECT_EQ(*iter, "first");
        EXPECT_EQ(++iter, seq.end());
    }

    TEST(StringUtils, SplitMultiSeparator)
    {
        const char* kStr = "first;second|third,fourth";
        const char* kSep = ";|,";
        const strings::SplitSequence seq = strings::Split(kStr, kSep);

        std::vector<std::string> elements;
        std::transform(seq.begin(), seq.end(), std::back_inserter(elements), [](auto sv)
        {
            return std::string{sv};
        });

        ASSERT_EQ(elements.size(), 4);
        EXPECT_EQ(elements[0], "first");
        EXPECT_EQ(elements[1], "second");
        EXPECT_EQ(elements[2], "third");
        EXPECT_EQ(elements[3], "fourth");
    }

    TEST(StringUtils, SplitNoSkipEmptyElements)
    {
        const char* kStr = ",A,,B,";
        const std::vector<std::string_view> expectedElements = {"", "A", "", "B", ""};

        const strings::SplitSequence seq = strings::Split(kStr, ",");
        std::vector<std::string_view> elements;
        std::copy(seq.begin(), seq.end(), std::back_inserter(elements));

        ASSERT_EQ(elements.size(), 5);
        EXPECT_EQ(elements, expectedElements);
    }

    TEST(StringUtils, TrimStart)
    {
        EXPECT_EQ(strings::TrimStart(""), "");
        EXPECT_EQ(strings::TrimStart("  "), "");
        EXPECT_EQ(strings::TrimStart("  \t "), "");
        EXPECT_EQ(strings::TrimStart(" \n "), "");
        EXPECT_EQ(strings::TrimStart("123"), "123");
        EXPECT_EQ(strings::TrimStart(" 123"), "123");
        EXPECT_EQ(strings::TrimStart("  \t 123"), "123");
        EXPECT_EQ(strings::TrimStart(" \t 123 456"), "123 456");
    }

    TEST(StringUtils, TrimEnd)
    {
        EXPECT_EQ(strings::TrimEnd(""), "");
        EXPECT_EQ(strings::TrimEnd("  \t "), "");
        EXPECT_EQ(strings::TrimEnd(" \n "), "");
        EXPECT_EQ(strings::TrimEnd("123"), "123");
        EXPECT_EQ(strings::TrimEnd("123  "), "123");
        EXPECT_EQ(strings::TrimEnd(" 123 456 \t "), " 123 456");
    }

    TEST(StringUtils, Trim)
    {
        EXPECT_EQ(strings::Trim(""), "");
        EXPECT_EQ(strings::Trim("  \t "), "");
        EXPECT_EQ(strings::Trim(" \n "), "");
        EXPECT_EQ(strings::Trim("123"), "123");
        EXPECT_EQ(strings::Trim("  \t123  "), "123");
        EXPECT_EQ(strings::Trim(" 123 \t "), "123");
        EXPECT_EQ(strings::Trim(" 123 456 \t "), "123 456");
    }

    TEST(StringUtils, IgnoreCase_Map)
    {
        std::map<std::string, int, strings::ICaseStringComparer<>> map = {
            {"Zero", 0},
            { "ONE", 1},
            { "two", 2}
        };

        const auto KeyOk = [&map](std::string_view key, int expected_val)
        {
            const auto iter = map.find(key);
            return iter != map.end() && iter->second == expected_val;
        };

        EXPECT_TRUE(KeyOk("zero", 0));
        EXPECT_TRUE(KeyOk("one", 1));
        EXPECT_TRUE(KeyOk("two", 2));
        EXPECT_EQ(map.find("three"), map.end());
    }

}  // namespace my::test
