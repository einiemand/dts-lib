#pragma once

#include <memory>

namespace dts {

template<typename T, typename Alloc = std::allocator<T>>
class vector {
public:
    static_assert(!std::is_reference_v<T>, "value_type can't be reference");
    static_assert(std::is_same_v<T, std::remove_cv_t<T>>,
                  "value_type must be non-const and non-volatile");
    static_assert(
      std::is_same_v<T, typename Alloc::value_type>,
      "vector's value_type must be the same as allocator's value_type");

    using value_type = T;
    using allocator_type = Alloc;
    using size_type = typename allocator_type::size_type;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    vector();

    explicit vector(const allocator_type& alloc);

    vector(size_type count, const value_type& value,
           const allocator_type& alloc = allocator_type());

    explicit vector(size_type count,
                    const allocator_type& alloc = allocator_type());

    template<typename InputIt>
    vector(InputIt first, InputIt last,
           allocator_type alloc = allocator_type());

    vector(const vector& other);

    vector(vector&& other) noexcept;

    vector(std::initializer_list<value_type> ilist,
           const allocator_type& alloc = allocator_type());

    ~vector();

    vector& operator=(const vector& other);

    vector& operator=(vector&& other) noexcept;

    allocator_type& get_allocator() noexcept;

    const allocator_type& get_allocator() const noexcept;

    reference at(size_type pos);

    const_reference at(size_type pos) const;

    reference operator[](size_type pos);

    const_reference operator[](size_type pos) const;

    reference front();

    const_reference front() const;

    reference back();

    const_reference back() const;

    pointer data() noexcept;

    const_pointer data() const noexcept;

    iterator begin() noexcept;

    const_iterator begin() const noexcept;

    const_iterator cbegin() const noexcept;

    iterator end() noexcept;

    const_iterator end() const noexcept;

    const_iterator cend() noexcept;

    iterator rbegin() noexcept;

    const_iterator rbegin() const noexcept;

    const_iterator crbegin() const noexcept;

    iterator rend() noexcept;

    const_iterator rend() const noexcept;

    const_iterator crend() const noexcept;

    bool empty() const noexcept;

    size_type size() const noexcept;

    void reserve(size_type new_cap);

    size_type capacity() const noexcept;

    void shrink_to_fit();

    void clear();

    iterator insert(const_iterator pos, const value_type& value);

    iterator insert(const_iterator pos, value_type&& value);

    iterator insert(const_iterator pos, size_type count,
                    const value_type& value);

    template<typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last);

    iterator insert(const_iterator pos,
                    std::initializer_list<value_type> ilist);

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args);

    iterator erase(const_iterator pos);

    iterator erase(const_iterator first, const_iterator last);

    void push_back(const value_type& value);

    void push_back(value_type&& value);

    template<typename... Args>
    reference emplace_back(Args&&... args);

    void pop_back();

    void resize(size_type count);

    void resize(size_type count, const value_type& value);

    void swap(vector& other) noexcept;

private:
    pointer start_ = {};
    pointer finish_ = {};
    pointer end_of_storage_ = {};
    allocator_type alloc_ = {};
};

template<typename T, typename Alloc>
bool operator==(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs);

template<typename T, typename Alloc>
bool operator!=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs);

template<typename T, typename Alloc>
bool operator<(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs);

template<typename T, typename Alloc>
bool operator<=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs);

template<typename T, typename Alloc>
bool operator>(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs);

template<typename T, typename Alloc>
bool operator>=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs);

}  // namespace dts

namespace std {
template<typename T, typename Alloc>
void swap(dts::vector<T, Alloc>& lhs, dts::vector<T, Alloc>& rhs);
}
