#pragma once

#include <cstdint>
#include <vector>

namespace dts {

class dynamic_bitset {
public:
    using block_type = uint32_t;
    using buffer_type = std::vector<block_type>;
    using size_type = buffer_type::size_type;

    static constexpr size_type block_byte_size = sizeof(block_type);
    static constexpr size_type block_bit_size = block_byte_size * 8;

    class reference {
    public:
        reference(block_type& blk, size_type pos);
        ~reference() = default;

        reference(const reference&) noexcept = default;
        reference& operator=(const reference&) noexcept = default;

        reference& operator=(bool val);

        reference& operator&=(bool val);
        reference& operator|=(bool val);
        reference& operator^=(bool val);
        bool operator~() const;

    private:
        block_type& blk_;
        const block_type mask_;
    };

    using const_reference = bool;

    explicit dynamic_bitset(size_type bit_cnt, uint64_t value = 0);
    ~dynamic_bitset() = default;

    dynamic_bitset(const dynamic_bitset&) = default;
    dynamic_bitset& operator=(const dynamic_bitset&) = default;

    dynamic_bitset(dynamic_bitset&&) noexcept = default;
    dynamic_bitset& operator=(dynamic_bitset&&) noexcept = default;

    dynamic_bitset& operator&=(const dynamic_bitset& other);
    dynamic_bitset& operator|=(const dynamic_bitset& other);
    dynamic_bitset& operator^=(const dynamic_bitset& other);
    dynamic_bitset& operator-=(const dynamic_bitset& other);
    dynamic_bitset& operator<<=(size_type offset);
    dynamic_bitset& operator>>=(size_type offset);
    dynamic_bitset operator<<(size_type offset) const;
    dynamic_bitset operator>>(size_type offset) const;
    dynamic_bitset operator~() const;

    friend bool operator==(const dynamic_bitset& lhs, const dynamic_bitset& rhs);
    friend bool operator!=(const dynamic_bitset& lhs, const dynamic_bitset& rhs);

    dynamic_bitset& set(size_type n, bool val = true);
    dynamic_bitset& set();
    dynamic_bitset& reset(size_type n);
    dynamic_bitset& reset();
    dynamic_bitset& flip(size_type n);
    dynamic_bitset& flip();

    bool test(size_type n) const;
    bool any() const;

    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    void push_back(bool bit);
    void adjust();

    size_type size() const;
    size_type num_blocks() const;
    bool empty() const;

private:
    buffer_type buf_;
    size_type bit_cnt_;
};

}  // namespace dts
