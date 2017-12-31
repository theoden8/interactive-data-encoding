#ifndef BITHACKS_HPP
#define BITHACKS_HPP

#include <cstdint>
#include <climits>

using int_t = int64_t;

namespace BitHacks {
  // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
  template <typename T>
  constexpr bool is_exp2(T v) {
    return !(v & (v - 1));
  }

  template <typename T>
  constexpr int_t weight(T v) {
    v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
    v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
    v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
    return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
  }

  template <typename T>
  constexpr int_t hamming_distance(T x, T y) {
    return weight<T>(x ^ y);
  }

  namespace test {
    template <typename T>
    void test() {
      static_assert(weight<T>(0xff) == 8, "");
      static_assert(weight<T>(0x1) == 1, "");
      static_assert(weight<T>(~T(0)) == sizeof(T)*CHAR_BIT, "");
      static_assert(hamming_distance<T>(T(0xff)<<3, T(0xff)) == 6, "");
    }

    void test_types() {
      test<uint16_t>();
      test<uint32_t>();
      test<uint64_t>();
    }
  } // testing stuff
} // namespace BitHacks

#endif /* end of include guard: BITHACKS_HPP */
