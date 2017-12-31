#ifndef HADAMARDCODE_HPP
#define HADAMARDCODE_HPP

#include <cstdint>
#include <climits>
#include <cassert>

#include <random>

#include <Random.hpp>
#include <Bithacks.hpp>
#include <Bitset.hpp>

struct HadamardCode {
  static constexpr int_t get_source_dim(int_t k) {
    return k;
  }

  static constexpr int_t get_dest_dim(int_t k) {
    return !k ? 0 : (1 << k);
  }

  static constexpr int_t min_dist(int_t k) {
    return 1 << (k - 1);
  }

  int_t k;

  HadamardCode(int_t k):
    k(k)
  {}

  template <typename T>
  void encode_n(bit_subset<T> &&dest, const bit_subset<T> &&src) {
    const int_t len = src.size();
    constexpr size_t BLOCK = sizeof(uint64_t) * CHAR_BIT;
    assert(k <= BLOCK);
    uint64_t w = 0x0000; // encoded vector
    for(uint64_t i = 0; i < len; ++i) {
      if(src[i]) {
        w |= (1 << (len - i - 1));
      }
    }
    const uint64_t mask = ~uint64_t(0) >> (BLOCK - k);
    uint64_t v = 0x0000; //(mask + 1) >> 1; // all possible vectors of size k <= 64
    while(v < mask + 1) {
      /* printf("prod %llu %llu = %lld -> [%lld | %lld]\n", w, v, v & w, BitHacks::weight(v & w), BitHacks::weight(v & w) & 1); */
      dest.set_bool(v, BitHacks::weight(v & w) & 1);
      ++v;
    };
    /* printf("encoded [%s] -> [%s]\n", str(src).c_str(), str(dest).c_str()); */
  }

  template <typename T>
  void decode_n(bit_subset<T> &&dest, const bit_subset<T> &&src) {
    const int_t len = dest.size();
    constexpr size_t BLOCK = sizeof(uint64_t) * CHAR_BIT;
    const uint64_t mask = ~uint64_t(0) >> (BLOCK - len);
    /* printf("decoding [%s]\n", str(src).c_str()); */
    for(uint64_t i = 0; i < len; ++i) {
      /* uint64_t j = Random::get_random<uint64_t>() & mask; */
      /* uint64_t k = j ^ (1 << i); */
      /* dest.set_bool(len - i - 1, (src[j]?1:0) ^ (src[k]?1:0)); */
      /* printf("take %llu ^ %llu = %llu -> x[%llu] = %d ^ %d = %d\n", j, k, 1<<i, i, src[j]?1:0, src[k]?1:0, dest[i]?1:0); */
      uint64_t c[2] = {0,0};
      for(uint64_t j = 0x0000; j <= mask; ++j) {
        uint64_t k = j ^ (1 << i);
        /* printf("take %llu ^ %llu = %llu -> x[%llu] = %d ^ %d = %d\n", j, k, uint64_t(1)<<i, i, src[j]?1:0, src[k]?1:0, dest[i]?1:0); */
        int bit_j = src[j] ? 1 : 0;
        int bit_k = src[k] ? 1 : 0;
        ++c[bit_j ^ bit_k];
      }
      dest.set_bool(len - i - 1, (c[0]>c[1])?0:1);
    }
    /* printf("decoded [%s] -> [%s]\n", str(src).c_str(), str(dest).c_str()); */
  }

  template <typename T>
  Bitset<T> encode(const Bitset<T> &msg) {
    const int_t len = msg.size();
    const int_t n = get_dest_dim(k);
    const int_t rem = len % k;
    const int_t remenclen = get_dest_dim(rem);
    const int_t enclen = len / k * n + remenclen;
    Bitset<T> bset(enclen);
    for(int_t i = 0; i < (len + k - 1) / k; ++i) {
      encode_n(
        bit_subset<T>(bset, i*n, std::min((i+1)*n, enclen)),
        bit_subset<T>((Bitset<T>&)msg, i*k, std::min((i+1)*k, len))
      );
    }
    return bset;
  }

  template <typename T>
  Bitset<T> decode(const Bitset<T> &rec) {
    const int_t reclen = rec.size();
    const int_t n = get_dest_dim(k);
    const int_t remenclen = reclen % n;
    const int_t rem = !remenclen ? 0 : std::log2(remenclen);
    const int_t len = reclen / n * k + rem;
    Bitset<T> bset(len);
    for(int_t i = 0; i < (reclen + n - 1) / n; ++i) {
      decode_n(
        bit_subset<T>(bset, i*k, std::min((i+1)*k, len)),
        bit_subset<T>((Bitset<T>&)rec, i*n, std::min((i+1)*n, reclen))
      );
    }
    return bset;
  }
};


#endif /* end of include guard: HADAMARDCODE_HPP */
