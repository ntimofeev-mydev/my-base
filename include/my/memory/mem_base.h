// #my_engine_source_file
#pragma once

#include "my/diag/assert.h"

#include <bit>
#include <compare>

namespace my
{
    constexpr inline bool IsPowerOf2(size_t value)
    {
        return (value & (value - 1)) == 0;
    }

    constexpr inline auto NearestPowerOf2(std::integral auto value)
    {
        return std::bit_ceil(value);
    }

    constexpr inline size_t AlignedSize(size_t size, size_t alignment)
    {
        if consteval
        {
            //static_assert(IsPowerOf2(alignment));
            //
        }
        else
        {
            MY_DBG_ASSERT(IsPowerOf2(alignment), "alignment expected to be a power of two. Actual value: ({})", alignment);
        }
        //
        return (size + alignment - 1) & ~(alignment - 1);
    }

    /**
     */
    template <size_t Factor>
    class MemSize
    {
        template <size_t>
        friend class MemSize;

    public:
        static inline constexpr size_t FactorValue = Factor;

        constexpr MemSize(size_t value = 0) :
            m_count{value}
        {
        }

        template <size_t U>
        requires((U >= Factor) && (U % Factor == 0))
        constexpr MemSize(const MemSize<U>& other) :
            m_count{(U / Factor) * other.m_count}
        {
        }

        constexpr operator size_t() const
        {
            return this->GetByteCount();
        }

        constexpr size_t GetByteCount() const
        {
            return m_count * Factor;
        }

        constexpr size_t GetCount() const
        {
            return m_count;
        }

        template <size_t U>
        requires(U >= Factor && (U % Factor == 0))
        constexpr MemSize& operator=(const MemSize<U>& other)
        {
            this->m_count = (U / Factor) * other.m_count;
            return *this;
        }

        template <size_t U>
        [[nodiscard]] constexpr std::strong_ordering operator<=>(const MemSize<U>& other) const
        {
            return this->GetByteCount() <=> other.GetByteCount();
        }

        template <size_t U>
        [[nodiscard]] constexpr bool operator==(const MemSize<U>& other) const
        {
            return this->GetByteCount() == other.GetByteCount();
        }

    private:
        size_t m_count = 0;
    };

    template <size_t U, size_t V>
    inline constexpr auto operator+(const MemSize<U>& u, const MemSize<V>& v)
    {
        constexpr size_t kMinFactor = (U < V) ? U : V;

        const size_t totalBytes = u.GetByteCount() + v.GetByteCount();
        return MemSize<kMinFactor>(totalBytes / kMinFactor);
    }

    using ByteSize = MemSize<1>;
    using Kilobyte = MemSize<1024>;
    using Megabyte = MemSize<1024 * 1024>;
    using Gigabyte = MemSize<1024 * 1024 * 1024>;

    namespace my_literals
    {
        [[nodiscard]] inline constexpr ByteSize operator""_b(unsigned long long count)
        {
            return ByteSize{static_cast<size_t>(count)};
        }

        [[nodiscard]] inline constexpr Kilobyte operator""_Kb(unsigned long long count)
        {
            return Kilobyte{static_cast<size_t>(count)};
        }

        [[nodiscard]] inline constexpr Megabyte operator""_Mb(unsigned long long count)
        {
            return Megabyte{static_cast<size_t>(count)};
        }

        [[nodiscard]] inline constexpr Gigabyte operator""_Gb(unsigned long long count)
        {
            return Gigabyte{static_cast<size_t>(count)};
        }

    }  // namespace my_literals

    namespace mem
    {
        constexpr inline size_t kPageSize = Kilobyte(4);
        constexpr inline size_t kAllocationGranularity = Kilobyte(64);

    }  // namespace mem
}  // namespace my
