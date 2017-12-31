#ifndef HAMMINGCODE_HPP
#define HAMMINGCODE_HPP

#include <cstdio>
#include <cassert>

#include <Bithacks.hpp>
#include <Bitset.hpp>

#if 0
#include <omp.h>
#endif

struct HammingCode {
  int_t r;

  static constexpr int_t get_source_dim(int_t r) {
    return (1 << r) - r - 1;
  }

  static constexpr int_t get_dest_dim(int_t r) {
    return (1 << r) - 1;
  }

  static constexpr int_t min_dist(int_t r) {
    return get_dest_dim(r) - get_source_dim(r);
  }

  static int_t ceil_log2(int_t x) {
    int_t t = x, u = 0;
    while(t>>=1)u+=1;
    return u-((1<<u==x)?1:0);
  }

  HammingCode(int_t r):
    r(r)
  {}

private:

  template <typename T>
  void encode_n(bit_subset<T> &&dest, const bit_subset<T> &&src) {
    const int_t len = src.size();
    /* printf("encoding %s\n", str(src).c_str()); */
    for(int_t i = 0; i < dest.size(); ++i) {
      if(BitHacks::is_exp2(i+1)) {
        dest.set_bool(i, 0);
        continue;
      } else {
        int_t x = (i + 1) - std::log2(i + 1);
        /* printf("%d: %d\n", i+1,x); */
        if(src[x-1]) {
          /* std::cout << "k: " << std::bitset<57>(x) << std::endl; */
          for(int_t b = 1; b <= dest.size(); b <<= 1) {
            if((i+1) & b) {
              /* printf("%d & %d\n", i+1, b); */
              dest.set_bool(b-1, !dest[b-1]);
            }
          }
        }
        /* printf("setting %d %d\n", i, src[x-1]?1:0); */
        dest.set_bool(i, src[x-1]);
      }
    }
    /* printf("enc result: %s\n", str(dest).c_str()); */
  }

  template <typename T>
  void decode_n(bit_subset<T> &&dest, const bit_subset<T> &&src) {
    const int_t len = dest.size();
    int_t err = 0;
    /* printf("decoding %s\n", str(src).c_str()); */
    for(int_t i = 0; i < src.size(); ++i) {
      int_t x = i + 1;
      /* printf("consider %lld\n", x); */
      if(src[i]) {
        if(BitHacks::is_exp2(x)) {
          /* printf("%d ^ %d\n", err, x); */
          err ^= x;
        } else {
          for(int_t b = 1; b <= src.size(); b <<= 1) {
            /* printf("%d & %d = %d\n", x,b,x&b); */
            if(x & b) {
              /* printf("%d ^ %d\n", err, b); */
              err ^= b;
            }
          }
        }
      }
    }
    /* printf("err=%d\n", err); */
    int_t j = 0;
    for(int_t i = 0; i < src.size(); ++i) {
      if(BitHacks::is_exp2(i+1)) {
        continue;
      }
      /* printf("COPY [%d]=%d AS [%d]=%d\n", i, src[i]?1:0); */
      dest.set_bool(j, i + 1 != err ? src[i] : !src[i]);
      ++j;
    }
    /* printf("dec result: %s\n", str(dest).c_str()); */
  }

public:
  int_t srclen = 0;

  template <typename T>
  decltype(auto) encode(const Bitset<T> &msg) {
    const int_t len = srclen = msg.size();
    const int_t
      k = get_source_dim(r),
      n = get_dest_dim(r),
      rem = len % k,
      remenclen = !rem?0:rem+2*ceil_log2(rem)+1,
      enclen = len/k*n+remenclen;
    /* printf("R==%d, K==%d, N==%d\n", r, k, n); */
    /* printf("ENCODING: %lld -> %lld | rem %lld -> %lld\n", len, enclen, rem, remenclen); */
    Bitset<T> bset(enclen);
    #if 0
    if(true && (len + k - 1) / k < 100000) {
    #endif
      for(int_t i = 0; i < (len + k - 1) / k; ++i) {
        encode_n(
          bit_subset<T>(bset, n*i, std::min(enclen, n*(i+1))),
          bit_subset<T>((Bitset<T>&)msg, k*i, std::min(srclen, k*(i+1)))
        );
      }
    #if 0
    } else {
      #pragma omp parallel for
      for(int_t i = 0; i < (len + k - 1) / k; ++i) {
        encode_n(
          bit_subset<T>(bset, n*i, std::min(enclen, n*(i+1))),
          bit_subset<T>((Bitset<T>&)msg, k*i, std::min(srclen, k*(i+1)))
        );
      }
    }
    #endif
    return bset;
  }

  template <typename T>
  decltype(auto) decode(const Bitset<T> &rec) {
    const int_t k = get_source_dim(r);
    const int_t n = get_dest_dim(r);
    const int_t reclen = rec.size();
    const int_t rem = reclen % get_dest_dim(r);
    const int_t remenclen = !rem ? 0 : rem + 2 * ceil_log2(rem) + 1;
    const int_t declen = srclen;
    /* printf("DECODING: %lld -> %lld | rem %lld -> %lld\n", reclen, declen, rem, remenclen); */
    Bitset<T> bset(srclen);
    #if 0
    #pragma omp parallel for
    #endif
    for(int_t i = 0; i < (reclen + n - 1) / n; ++i) {
      decode_n(
        bit_subset<T>(bset, k*i, std::min(declen, k*(i+1))),
        bit_subset<T>((Bitset<T>&)rec, n*i, std::min(reclen, n*(i+1)))
      );
    }
    return bset;
  }
};

#endif /* HAMMINGCODE_HPP */
