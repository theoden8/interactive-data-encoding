#ifndef BITSET_HPP
#define BITSET_HPP

#include <cmath>
#include <climits>
#include <cstdlib>
#include <cstdint>

#include <array>
#include <vector>

#include <Random.hpp>
#include <Bithacks.hpp>

#if 0
#include <omp.h> */
#endif

template <typename T> struct bit_subset;
template <typename T> struct Bitset;

using bitset_t = Bitset<uint32_t>;

constexpr int log2_for_T(int x) {
  return (x == 1) ? 0 : 1 + log2_for_T(x >> 1);
}

using int_t = int64_t;

template <typename T = uint64_t>
struct Bitset {
  static constexpr int_t BLOCK = sizeof(T) * CHAR_BIT;
  static constexpr int_t BLOCK_P = log2_for_T(BLOCK);

  std::vector<T> data;
  size_t size_ = 0;

  Bitset(int_t size=0):
    size_(size), data((size + BLOCK - 1) / BLOCK)
  {}

  Bitset(int_t size, bool val):
    size_(size), data((size + BLOCK - 1) / BLOCK, val ? MASK_CONTIG_1 : 0x00)
  {}

  constexpr size_t size() const {
    return size_;
  }

  constexpr size_t get_index(size_t i) const {
    return i / BLOCK;
  }

  constexpr size_t get_bit(size_t i) const {
    return i % BLOCK;
  }

  void append(bool b) {
    if(get_bit(size()) == 0) {
      data.push_back(b ? 0x01 : 0x00);
      ++size_;
    } else {
      ++size_;
      set_bool(size() - 1, b);
    }
  }

  bool pop() {
    bool x = data[data.size() - 1];
    if(get_bit(size()) == 0) {
      data.pop_back();
    }
    --size_;
    return x;
  }

  void resize(int_t new_size) {
    data.resize((new_size + BLOCK - 1) / BLOCK);
    size_ = new_size;
  }

public:
  void set_random(const int_t start, const int_t end) {
    uint64_t key_component = rand();
    for(int_t i = get_index(start); i <= get_index(end-1); ++i) {
      uint64_t key = Random::wang_rng(key_component ^ ~i);
      T block = Random::wang_rng(key);
      T mask = MASK_CONTIG_1;
      if(i == get_index(start)) {
        T begin_mask = MASK_CONTIG_1 << get_bit(start);
        mask &= begin_mask;
      }
      if(i == get_index(end-1)) {
        T end_mask = MASK_CONTIG_1 >> get_bit(end);
        mask &= end_mask;
      }
      auto &d = data[i];
      d = (d & ~mask) | (block & mask);
    }
  }

  int_t set_errors(float p) {
    std::array<T, BLOCK> beets;
    for(int i = 0; i < beets.size(); ++i) {
      beets[i] = 1 << i;
    }
    const std::array<T, BLOCK> bits = beets;
    const T prob = p * MASK_CONTIG_1;
    const uint64_t key_component = Random::get_random<uint64_t>();
    int_t err = 0;
    for(int i = 0; i < data.size(); ++i) {
      for(int j = 0; j < BLOCK; ++j) {
        if(i * BLOCK + j > size())break;
        auto &b = bits[j];
        T r = Random::wang_rng(key_component + ~(i*BLOCK+j));
        if(r < prob) {
          data[i] ^= b;
          ++err;
        }
      }
    }
    return err;
  }

  static int_t hamming_distance(const Bitset<T> &a, const Bitset<T> &b) {
    int_t res = 0;
    for(int_t i = 0; i < std::min(a.data.size(), b.data.size()); ++i) {
      res += BitHacks::hamming_distance<T>(a.get_masked(i), b.get_masked(i));
    }
    return res;
  }

  template <typename U>
  bool operator==(const Bitset<U> &other) {
    if(size() != other.size()) return false;
    for(int_t i = 0; i < size(); ++i) if(operator[](i) != other[i]) return false;
    return true;
  }

  static constexpr T MASK_CONTIG_1 = ~0;
  constexpr T get_masked(int_t i) const {
    if(i == data.size() - 1) {
      return data[i] & (MASK_CONTIG_1 >> (BLOCK - get_bit(size())));
    }
    return data[i];
  }
  bool xor_positions(int_t p) const {
    return bit_subset<T>((Bitset<T>&)(*this), 0, size()).xor_positions(p);
  }

  constexpr bool operator[](int_t i) const {
    return data[get_index(i)] & (1 << get_bit(i));
  }

  bool set_bool(int_t i, bool x) {
    if(x) {
      data[get_index(i)] |= (1 << get_bit(i));
    } else {
      data[get_index(i)] &= ~(1 << get_bit(i));
    }
    return x;
  }

  bool empty() {
    return size() == 0;
  }

  void clear() {
    size_ = 0;
    data.clear();
  }
};

template <typename T>
struct bit_subset {
  static constexpr int_t BLOCK = sizeof(T) * CHAR_BIT;
  static constexpr int_t BLOCK_P = log2_for_T(BLOCK);

  Bitset<T> &b;

  int_t start, stop;
  int_t trim_begin, trim_end;

  constexpr bit_subset(Bitset<T> &b, int_t start=0, int_t end=-1):
    b(b), start(start), stop((end==-1)?b.size():end),
    trim_begin(BLOCK - (start % BLOCK)), trim_end(end / BLOCK * BLOCK)
  {}
  constexpr int_t size() const {
    return stop - start;
  }
  bool operator[](int_t i) const {
    return b[start+i];
  }
  bool set_bool(int_t i, bool x) {
    return b.set_bool(start+i, x);
  }

  constexpr bit_subset<T> subset(int from, int len) const {
    return bit_subset(b, start + from, start + from + len);
  }

  static constexpr T MASK_CONTIG_1 = ~0;
  static constexpr T MASK_CONTIG_2 = 0xAAAAAAAAAAAAAAAA;
  static constexpr T MASK_CONTIG_4 = 0xCCCCCCCCCCCCCCCC;
  static constexpr T MASK_CONTIG_8 = 0xF0F0F0F0F0F0F0F0;
  static constexpr T MASK_CONTIG_16 = 0xFF00FF00FF00FF00;
  static constexpr T MASK_CONTIG_32 = 0xFFFFFFFF00000000;
  constexpr T get_masked(int_t i) const {
    T x = b.data[i];
    if(i < trim_begin) {
      x &= (MASK_CONTIG_1 << trim_begin);
    }
    if(i >= trim_end) {
      x &= (MASK_CONTIG_1 >> (stop % BLOCK));
    }
    return x;
  }
  constexpr bool xor_positions(int_t p) const {
    int_t res = 0;
    auto &bset = *this;
    /* printf("p==%lld\n", p); */
    for(int_t i = 0; i < size(); ++i) {
      if(!(i & (1 << p))) {
        /* printf("xor-ing %lld\n", i); */
        if(bset[i]) {
          res ^= 1;
        }
      }
    }
    return res;
#if 0
    T res = 0x00;
    if(p <= BLOCK_P) {
      if(p == 1) {
        for(int_t i = start / BLOCK * BLOCK; i <= (stop - 1) / BLOCK * BLOCK; ++i) {
          res ^= get_masked(i) & (f ? MASK_CONTIG_2 : ~MASK_CONTIG_2);
        }
      } else if(p == 2) {
        for(int_t i = start / BLOCK * BLOCK; i <= (stop - 1) / BLOCK * BLOCK; ++i) {
          res ^= get_masked(i) & (f ? MASK_CONTIG_4 : ~MASK_CONTIG_4);
        }
      } else if(p == 3) {
        for(int_t i = start / BLOCK * BLOCK; i <= (stop - 1) / BLOCK * BLOCK; ++i) {
          res ^= get_masked(i) & (f ? MASK_CONTIG_8 : ~MASK_CONTIG_8);
        }
      } else if(p == 4) {
        for(int_t i = start / BLOCK * BLOCK; i <= (stop - 1) / BLOCK * BLOCK; ++i) {
          res ^= get_masked(i) & (f ? MASK_CONTIG_16 : ~MASK_CONTIG_16);
        }
      } else if(p == 5) {
        for(int_t i = start / BLOCK * BLOCK; i <= (stop - 1) / BLOCK * BLOCK; ++i) {
          res ^= get_masked(i) & (f ? MASK_CONTIG_32 : ~MASK_CONTIG_32);
        }
      }
    } else {
      const int_t contig = 1 << (p - BLOCK_P);
      int_t c = 0;
      for(int_t i = start / BLOCK * BLOCK; i <= (stop - 1) / BLOCK * BLOCK; ++i, ++c) {
        if(c == contig * 2) {
          c = 0;
        }
        if((f?1:0) == ((c < contig)?1:0)) {
          res ^= get_masked(c);
        }
      }
    }
    for(int_t i = 0; i < BLOCK; ++i) {
      if(!res) return false;
      if(res == 0x01) return true;
      if(res | 1) res = (res >> 1) ^ 1; else res >>= 1;
    }
    return false;
#endif
  }
};


#endif /* end of include guard: BITSET_HPP */
