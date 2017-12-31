#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdint>
#include <cstdlib>

namespace Random {
  // wang random number generator
  static constexpr uint64_t wang_rng(uint64_t key) {
    key = (~key) + (key << 21);
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8);
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
  }

  template <typename T>
  T get_random() {
    return wang_rng(rand() * ~rand());
  }
} // namespace Random


#endif /* end of include guard: RANDOM_HPP */
