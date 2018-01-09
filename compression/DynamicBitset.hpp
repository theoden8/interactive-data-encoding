#ifndef DYNAMICBITSET_HPP
#define DYNAMICBITSET_HPP

#include <cmath>
#include <climits>
#include <vector>
#include <string>
#include <bitset>

#include <utility>
#include <type_traits>

namespace detail {
  template <typename T, bool scalar = std::is_fundamental<T>::value> struct bit_appender;
} // namespace detail

struct DynamicBitset {
  static constexpr const char *repr = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::vector<bool> bitset_;
  DynamicBitset(size_t N = 0, bool value = false):
    bitset_(N, value)
  {}
  bool operator[](size_t i) const { return bitset_[i]; }
  auto operator[](size_t i) -> decltype(bitset_[i]) { return bitset_[i]; }
  size_t size() const noexcept { return bitset_.size(); }
  // special function to append a bit
  void append_bit(bool value) noexcept {
    append<bool>(value);
  }
  // containers by reference
  template <typename T, typename = std::enable_if_t<!std::is_fundamental<std::remove_reference_t<T>>::value>>
  void append(T &value) noexcept {
    detail::bit_appender<T>::append(*this, value);
  }
  template <typename T, typename = std::enable_if_t<!std::is_fundamental<std::remove_reference_t<T>>::value>>
  void append(T &&value) noexcept {
    detail::bit_appender<T>::append(*this, value);
  }
  // scalars by value
  template <typename T, typename = std::enable_if_t<std::is_fundamental<T>::value>>
  void append(T value) noexcept {
    detail::bit_appender<T>::append(*this, value);
  }
  void pop() {
    bitset_.pop_back();
  }
  void reverse() {
    auto &self = *this;
    for(int i = 0; i < size() >> 1; ++i) {
      auto t = self[i];
      self[i] = self[size() - i - 1];
      self[size() - i - 1] = t;
    }
  }
  std::string str() const noexcept {
    std::string s;
    s.reserve(size());
    for(int i = 0; i < size(); ++i) {
      s += repr[(*this)[i]];
    }
    return s;
  }
};

namespace detail {

template <typename T> struct bit_appender<T, true> {
  static void append(DynamicBitset &bset, T value) noexcept {
    auto nbits = sizeof(T) << 3;
    for(int i = 0; i < nbits; ++i) {
      bset.append_bit(value & (1 << (nbits - i - 1)));
    }
  }
};

template <> struct bit_appender<float, true> {
  static void append(DynamicBitset &bset, float value) noexcept {
    bit_appender<long>::append(bset, long(value));
    value -= long(value);
    while(value != std::floor(value)) {
      value *= 2;
    }
    bit_appender<long>::append(bset, long(value));
  }
};

template <> struct bit_appender<double, true> {
  static void append(DynamicBitset &bset, double value) noexcept {
    bit_appender<long long>::append(bset, (long long)(value));
    value -= (long long)(value);
    while(value != std::floor(value)) {
      for(int i = 0; value != std::floor(value) && i < 32; ++i) {
        value *= 2;
      }
      bit_appender<long long>::append(bset, (long long)(value));
    }
  }
};

template <> struct bit_appender<bool> {
  static void append(DynamicBitset &bset, bool value) noexcept {
    bset.bitset_.push_back(value);
  }
};

template <typename T> struct bit_appender<T, false> {
  static void append(DynamicBitset &bset, T &container) noexcept {
    for(auto it : container) {
      bset.append<decltype(it)>(it);
    }
  }
};

template <> struct bit_appender<DynamicBitset> {
  static void append(DynamicBitset &bset, DynamicBitset &other) noexcept {
    for(size_t i = 0; i < other.size(); ++i) {
      bset.append_bit(other[i]);
    }
  }
};

template <size_t N> struct bit_appender<std::bitset<N>, false> {
  static void append(DynamicBitset &bset, std::bitset<N> &other) noexcept {
    for(size_t i = 0; i < other.size(); ++i) {
      bset.append_bit(other[i]);
    }
  }
};

} // namespace detail


#endif /* end of include guard: DYNAMICBITSET_HPP */
