#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>

namespace dts {

inline constexpr std::size_t bits_per_byte = 8;
inline constexpr std::size_t uint64_width = sizeof(uint64_t) * bits_per_byte;

class dynamic_bitset {
public:
    using block_type = uint32_t;
    using buffer_type = std::vector<block_type>;
    using size_type = buffer_type::size_type;

    static constexpr size_type bytes_per_block = sizeof(block_type);
    static constexpr size_type bits_per_block = bytes_per_block * 8;

    class reference {
    public:
        reference(block_type& blk, size_type pos)
            : blk_(blk),
              mask_((assert(pos < bits_per_block), block_type(1) << pos)) {}

        ~reference() = default;
        reference(const reference&) noexcept = default;

        reference& operator=(const reference& other) {
            if (other) {
                set();
            }
            else {
                reset();
            }
            return *this;
        }

        reference& operator=(bool val) {
            if (val) {
                set();
            }
            else {
                reset();
            }
            return *this;
        }

        reference& operator&=(bool val) {
            if (!val) {
                reset();
            }
            return *this;
        }

        reference& operator|=(bool val) {
            if (val) {
                set();
            }
            return *this;
        }

        reference& operator^=(bool val) {
            if (val) {
                blk_ ^= mask_;
            }
            return *this;
        }

        operator bool() const {
            return (blk_ & mask_) != 0;
        }

        bool operator~() const {
            return !(*this);
        }

    private:
        block_type& blk_;
        const block_type mask_;

        void set() {
            blk_ |= mask_;
        }

        void reset() {
            blk_ &= ~mask_;
        }
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
    dynamic_bitset& operator<<=(size_type offset);
    dynamic_bitset& operator>>=(size_type offset);
    dynamic_bitset operator<<(size_type offset) const;
    dynamic_bitset operator>>(size_type offset) const;
    dynamic_bitset operator~() const;

    friend bool operator==(const dynamic_bitset& lhs,
                           const dynamic_bitset& rhs);
    friend bool operator!=(const dynamic_bitset& lhs,
                           const dynamic_bitset& rhs);

    dynamic_bitset& set(size_type pos, size_type len, bool val);
    dynamic_bitset& set(size_type pos, bool val = true);
    dynamic_bitset& set();
    dynamic_bitset& reset(size_type pos);
    dynamic_bitset& reset();
    dynamic_bitset& flip(size_type pos);
    dynamic_bitset& flip();

    bool test(size_type pos) const;
    bool any() const;

    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    void push_back(bool bit);

    size_type size() const;
    size_type num_blocks() const;
    bool empty() const;
    std::string to_string(char zero = '0', char one = '1') const;

private:
    buffer_type buf_;
    size_type bit_cnt_;

    void expand_if_smaller_than(size_type new_bit_cnt);

    static size_type block_cnt_from_bit_cnt(size_type bit_cnt);
    static size_type bit_pos_to_block_index(size_type pos);
    static size_type bit_pos_to_bit_index(size_type pos);

    static constexpr block_type zeros = static_cast<block_type>(0);
    static constexpr block_type ones = static_cast<block_type>(~0);
};

}  // namespace dts
