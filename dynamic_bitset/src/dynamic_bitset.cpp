#include "dynamic_bitset.hpp"

#include <sstream>

namespace dts {

template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
T n_lsb(T val, std::size_t n) {
    static constexpr std::size_t type_width = sizeof(T) * bits_per_byte;
    assert(n <= type_width);
    return n < type_width ? (val & ((1 << n) - 1)) : val;
}

template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
T n_msb(T val, std::size_t n) {
    static constexpr std::size_t type_width = sizeof(T) * bits_per_byte;
    assert(n <= type_width);
    return n > 0 ? (val >> (type_width - n)) : 0;
}

dynamic_bitset::dynamic_bitset(size_type bit_cnt, uint64_t value)
    : buf_(block_cnt_from_bit_cnt(bit_cnt)),
      bit_cnt_(bit_cnt) {
    if (bit_cnt < uint64_width) {
        value = n_lsb(value, bit_cnt);
    }
    size_type blks_to_write =
      std::min(sizeof(value) / bytes_per_block, buf_.size());
    for (size_type i = 0; i < blks_to_write; ++i) {
        buf_[i] = static_cast<block_type>(value);
        value >>= bits_per_block;
    }
}

dynamic_bitset& dynamic_bitset::operator&=(const dynamic_bitset& other) {
    assert(size() == other.size());
    for (size_type i = 0; i < num_blocks(); ++i) {
        buf_[i] &= other.buf_[i];
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::operator|=(const dynamic_bitset& other) {
    assert(size() == other.size());
    for (size_type i = 0; i < num_blocks(); ++i) {
        buf_[i] |= other.buf_[i];
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::operator^=(const dynamic_bitset& other) {
    assert(size() == other.size());
    for (size_type i = 0; i < num_blocks(); ++i) {
        buf_[i] ^= other.buf_[i];
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::operator<<=(size_type offset) {
    if (offset < size()) {
        const size_type blk_idx_diff = bit_pos_to_block_index(offset);
        const size_type bit_idx_diff = bit_pos_to_bit_index(offset);
        for (size_type high_blk_idx = num_blocks() - 1,
                       low_blk_idx = high_blk_idx - blk_idx_diff;
             low_blk_idx > 0; --high_blk_idx, --low_blk_idx)
        {
            buf_[high_blk_idx] = (buf_[low_blk_idx] << bit_idx_diff) |
                                 n_msb(buf_[low_blk_idx - 1], bit_idx_diff);
        }
        buf_[blk_idx_diff] = (buf_[0] << bit_idx_diff);
        set(0, offset, 0);
    }
    else {
        reset();
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::operator>>=(size_type offset) {
    if (offset < size()) {
        const size_type blk_idx_diff = bit_pos_to_block_index(offset);
        const size_type bit_idx_diff = bit_pos_to_bit_index(offset);
        for (size_type low_blk_idx = 0, high_blk_idx = blk_idx_diff;
             high_blk_idx + 1 < num_blocks(); ++low_blk_idx, ++high_blk_idx)
        {
            buf_[low_blk_idx] = (buf_[high_blk_idx] >> bit_idx_diff) |
                                n_lsb(buf_[high_blk_idx + 1], bit_idx_diff);
        }
        buf_[num_blocks() - blk_idx_diff - 1] = (buf_.back() >> bit_idx_diff);
        set(size() - offset, offset, 0);
    }
    else {
        reset();
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::set(size_type pos, size_type len, bool val) {
    for (size_type i = pos; i < pos + len; ++i) {
        set(i, val);
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::set(size_type pos, bool val) {
    (*this)[pos] = val;
    return *this;
}

dynamic_bitset& dynamic_bitset::set() {
    for (block_type& blk : buf_) {
        blk = static_cast<block_type>(-1);
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::reset(size_type pos) {
    return set(pos, 0);
}

dynamic_bitset& dynamic_bitset::reset() {
    for (block_type& blk : buf_) {
        blk = static_cast<block_type>(0);
    }
    return *this;
}

dynamic_bitset::reference dynamic_bitset::operator[](size_type pos) {
    return reference(buf_[bit_pos_to_block_index(pos)],
                     bit_pos_to_bit_index(pos));
}

dynamic_bitset::const_reference dynamic_bitset::operator[](
  size_type pos) const {
    return static_cast<const_reference>(
      const_cast<dynamic_bitset&>(*this)[pos]);
}

dynamic_bitset::size_type dynamic_bitset::size() const {
    return bit_cnt_;
}

dynamic_bitset::size_type dynamic_bitset::num_blocks() const {
    return buf_.size();
}

bool dynamic_bitset::empty() const {
    return size() == static_cast<size_type>(0);
}

std::string dynamic_bitset::to_string(char zero, char one) const {
    std::ostringstream stream;
    for (size_type i = size(); i > 0; --i) {
        stream << ((*this)[i - 1] ? one : zero);
    }
    return stream.str();
}

void dynamic_bitset::visit_each_block(const block_visitor& visitor) const {
    for (size_type i = 0; i < buf_.size(); ++i) {
        visitor(i, buf_[i]);
    }
}

void dynamic_bitset::expand_if_smaller_than(size_type new_bit_cnt) {
    if (size() < new_bit_cnt) {
        size_type new_blk_cnt = block_cnt_from_bit_cnt(new_bit_cnt);
        buf_.resize(new_blk_cnt, block_type(0));
        bit_cnt_ = new_bit_cnt;
    }
}

dynamic_bitset::size_type dynamic_bitset::block_cnt_from_bit_cnt(
  size_type bit_cnt) {
    return (bit_cnt + bits_per_block - 1) / bits_per_block;
}

dynamic_bitset::size_type dynamic_bitset::bit_pos_to_block_index(
  size_type pos) {
    return pos / bits_per_block;
}

dynamic_bitset::size_type dynamic_bitset::bit_pos_to_bit_index(size_type pos) {
    return pos % bits_per_block;
}

}  // namespace dts
