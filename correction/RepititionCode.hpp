#ifndef REPITITIONCODE_HPP
#define REPITITIONCODE_HPP

#include <Bitset.hpp>

struct RepititionCode {
  int_t e;

  static constexpr int_t get_source_dim(int_t e) {
    return 1;
  }

  static constexpr int_t get_dest_dim(int_t e) {
    return 2*e+1;
  }

  static constexpr int_t min_dist(int_t e) {
    return 2*e + 1;
  }

  RepititionCode(int_t e):
    e(e)
  {}

  int_t srclen = 0;

  template <typename T>
  Bitset<T> encode(const Bitset<T> &msg) {
    const int_t len = srclen = msg.size();
    const int_t min_d = 2*e + 1;
    Bitset<T> bset(len*min_d);
    for(int_t i = 0; i < len; ++i) {
      for(int_t j = 0; j < min_d; ++j) {
        bset.set_bool(i*min_d+j, msg[i]);
      }
    }
    return bset;
  }

  template <typename T>
  Bitset<T> decode(const Bitset<T> &rec) {
    const int_t len = srclen;
    const int_t min_d = 2*e + 1;
    Bitset<T> bset(srclen);
    for(int_t i = 0; i < len; ++i) {
      int_t counts[2] = {0,0};
      for(int_t j = 0; j < min_d; ++j) {
        ++counts[rec[i*min_d+j]?1:0];
      }
      bset.set_bool(i, counts[0] > counts[1] ? 0 : 1);
    }
    return bset;
  }
};


#endif /* end of include guard: REPITITIONCODE_HPP */
