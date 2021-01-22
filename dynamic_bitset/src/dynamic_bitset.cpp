#include "dynamic_bitset.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace dts {

namespace detail {

template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
T bit_mask(std::size_t a, std::size_t b) {
    // [a,b)
    static constexpr std::size_t type_width = sizeof(T) * bits_per_byte;
    static constexpr T one = static_cast<T>(1);
    static constexpr T all_ones = static_cast<T>(~0);
    assert(a <= b && b <= type_width);
    return (b - a == type_width) ? all_ones : (((one << (b - a)) - 1) << a);
}

template<typename T>
T n_lsb(T val, std::size_t n) {
    // n least significant bits.
    return (val & bit_mask<T>(0, n));
}

template<typename T>
T n_msb(T val, std::size_t n) {
    // n most significant bits.
    static constexpr std::size_t type_width = sizeof(T) * bits_per_byte;
    return ((val & bit_mask<T>(type_width - n, type_width)) >>
            (type_width - n));
}

static const char* hex_char_to_bin(char hex_char) {
    switch (toupper(hex_char)) {
    case '0':
        return "0000";
    case '1':
        return "0001";
    case '2':
        return "0010";
    case '3':
        return "0011";
    case '4':
        return "0100";
    case '5':
        return "0101";
    case '6':
        return "0110";
    case '7':
        return "0111";
    case '8':
        return "1000";
    case '9':
        return "1001";
    case 'A':
        return "1010";
    case 'B':
        return "1011";
    case 'C':
        return "1100";
    case 'D':
        return "1101";
    case 'E':
        return "1110";
    case 'F':
        return "1111";
    default:
        assert(false);
    }
}

}  // namespace detail

dynamic_bitset::dynamic_bitset(size_type bit_cnt, uint64_t value)
    : buf_(bit_cnt_to_block_cnt(bit_cnt)),
      bit_cnt_(bit_cnt) {
    if (bit_cnt < uint64_width) {
        value = detail::n_lsb(value, bit_cnt);
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
    if (offset >= size()) {
        reset();
    }
    else if (offset > 0) {
        const size_type blk_idx_diff = bit_pos_to_block_index(offset);
        const size_type bit_idx_diff = bit_pos_to_bit_index(offset);
        for (size_type high_blk_idx = num_blocks() - 1,
                       low_blk_idx = high_blk_idx - blk_idx_diff;
             low_blk_idx > 0; --high_blk_idx, --low_blk_idx)
        {
            buf_[high_blk_idx] =
              (buf_[low_blk_idx] << bit_idx_diff) |
              detail::n_msb(buf_[low_blk_idx - 1], bit_idx_diff);
        }
        buf_[blk_idx_diff] = (buf_[0] << bit_idx_diff);
        set(0, offset, 0);
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::operator>>=(size_type offset) {
    if (offset >= size()) {
        reset();
    }
    else if (offset > 0) {
        const size_type blk_idx_diff = bit_pos_to_block_index(offset);
        const size_type bit_idx_diff = bit_pos_to_bit_index(offset);
        for (size_type low_blk_idx = 0, high_blk_idx = blk_idx_diff;
             high_blk_idx + 1 < num_blocks(); ++low_blk_idx, ++high_blk_idx)
        {
            buf_[low_blk_idx] =
              (buf_[high_blk_idx] >> bit_idx_diff) |
              detail::n_lsb(buf_[high_blk_idx + 1], bit_idx_diff);
        }
        buf_[num_blocks() - blk_idx_diff - 1] = (buf_.back() >> bit_idx_diff);
        set(size() - offset, offset, 0);
    }
    return *this;
}

dynamic_bitset dynamic_bitset::operator<<(size_type offset) const {
    dynamic_bitset copy(*this);
    return (copy <<= offset);
}

dynamic_bitset dynamic_bitset::operator>>(size_type offset) const {
    dynamic_bitset copy(*this);
    return (copy >>= offset);
}

dynamic_bitset dynamic_bitset::operator~() const {
    dynamic_bitset copy(*this);
    return copy.flip();
}

dynamic_bitset& dynamic_bitset::set(size_type pos, size_type len, bool val) {
    const size_type last_pos = pos + len;
    if (pos >= size() || last_pos > size()) {
        throw std::out_of_range("dynamic_bitset::set() out of range");
    }

    const size_type first_blk_idx = bit_pos_to_block_index(pos);
    const size_type first_bit_idx = bit_pos_to_bit_index(pos);
    const size_type last_blk_idx = bit_pos_to_block_index(last_pos);
    const size_type last_bit_idx = bit_pos_to_bit_index(last_pos);

    const block_type new_bits = (val ? ones : zeros);

    for (size_type blk_idx = first_blk_idx + 1; blk_idx < last_blk_idx;
         ++blk_idx) {
        buf_[blk_idx] = new_bits;
    }
    block_type& first_blk = buf_[first_blk_idx];
    block_type& last_blk = buf_[last_blk_idx];
    if (first_blk_idx < last_blk_idx) {
        first_blk = (detail::n_msb(new_bits, bits_per_block - first_bit_idx)
                     << first_bit_idx) |
                    detail::n_lsb(first_blk, first_bit_idx);
        last_blk = (detail::n_msb(last_blk, bits_per_block - last_bit_idx)
                    << last_bit_idx) |
                   detail::n_lsb(new_bits, last_bit_idx);
    }
    else {
        first_blk = (detail::n_msb(first_blk, bits_per_block - last_bit_idx)
                     << last_bit_idx) |
                    (new_bits & detail::bit_mask<block_type>(first_bit_idx,
                                                             last_bit_idx)) |
                    detail::n_lsb(first_blk, first_bit_idx);
    }

    return *this;
}

dynamic_bitset& dynamic_bitset::set(size_type pos, bool val) {
    if (pos >= size()) {
        throw std::out_of_range("dynamic_bitset::set() out of range");
    }
    (*this)[pos] = val;
    return *this;
}

dynamic_bitset& dynamic_bitset::set() {
    return set(0, size(), 1);
}

dynamic_bitset& dynamic_bitset::reset(size_type pos) {
    return set(pos, 0);
}

dynamic_bitset& dynamic_bitset::reset() {
    return set(0, size(), 0);
}

dynamic_bitset& dynamic_bitset::flip(size_type pos) {
    (*this)[pos] ^= 1;
    return *this;
}

dynamic_bitset& dynamic_bitset::flip() {
    if (!empty()) {
        for (size_type blk_idx = 0; blk_idx + 1 < num_blocks(); ++blk_idx) {
            buf_[blk_idx] ^= ones;
        }
        const size_type last_bit_idx = bit_pos_to_bit_index(size() - 1) + 1;
        buf_.back() ^= detail::bit_mask<block_type>(0, last_bit_idx);
    }
    return *this;
}

bool dynamic_bitset::test(size_type pos) const {
    return static_cast<bool>((*this)[pos]);
}

bool dynamic_bitset::any() const {
    return std::any_of(buf_.begin(), buf_.end(), [](block_type blk) {
        return blk != zeros;
    });
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

void dynamic_bitset::push_back(bool bit) {
    expand_if_smaller_than(size() + 1);
    set(size() - 1, 1, bit);
}

void dynamic_bitset::swap(dynamic_bitset& other) noexcept {
    buf_.swap(other.buf_);
    std::swap(bit_cnt_, other.bit_cnt_);
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
    static constexpr size_type hexes_per_block = bits_per_block / bits_per_hex;
    static constexpr char default_zero = '0';
    static constexpr char default_one = '1';

    std::string rep;
    if (!empty()) {
        std::stringstream hex_stream;
        for (size_type i = num_blocks(); i > 0; --i) {
            size_type blk_idx = i - 1;
            hex_stream << std::setw(hexes_per_block)
                       << std::setfill(default_zero) << std::hex
                       << buf_[blk_idx];
        }
        std::stringstream bin_stream;
        std::transform(std::istream_iterator<char>(hex_stream),
                       std::istream_iterator<char>(),
                       std::ostream_iterator<const char*>(bin_stream),
                       detail::hex_char_to_bin);

        size_type last_bit_idx = bit_pos_to_bit_index(size() - 1);
        size_type last_blk_bit_cnt = last_bit_idx + 1;
        size_type num_bits_to_drop = bits_per_block - last_blk_bit_cnt;
        bin_stream.seekg(num_bits_to_drop);

        bin_stream >> rep;
        std::replace(rep.begin(), rep.end(), default_zero, zero);
        std::replace(rep.begin(), rep.end(), default_one, one);
    }
    return rep;
}

void dynamic_bitset::expand_if_smaller_than(size_type new_bit_cnt) {
    if (size() < new_bit_cnt) {
        size_type new_blk_cnt = bit_cnt_to_block_cnt(new_bit_cnt);
        buf_.resize(new_blk_cnt, zeros);
        bit_cnt_ = new_bit_cnt;
    }
}

dynamic_bitset::size_type dynamic_bitset::bit_cnt_to_hex_cnt(
  size_type bit_cnt) {
    return (bit_cnt + bits_per_hex - 1) / bits_per_hex;
}

dynamic_bitset::size_type dynamic_bitset::bit_cnt_to_block_cnt(
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
