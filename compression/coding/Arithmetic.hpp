#ifndef CODINGARITHMETIC_HPP
#define CODINGARITHMETIC_HPP

#include <cassert>
#include <cstdint>
#include <vector>

#include <coding/Base.hpp>

namespace coding {

template <typename T>
struct interval {
  T l, r;
  interval(T _l, T _r): l(_l), r(_r) {}

  template <typename U>
  interval(interval<U> t): l(t.l), r(t.r) {}

  constexpr T left() const { return l; }
  constexpr T right() const { return r; }
  T &left() noexcept { return l; }
  T &right() noexcept { return r; }

  auto operator+(T p) const { return interval(r + p, r + p); }
  void operator+=(T p) { l+=r;r+=p; }
  auto operator+(interval t) { return interval(l+t.l, r+t.r); }
  void operator+=(interval t) { l+=t.l,r+=t.r; }

  auto operator*(T p) const { return interval(l * p, r * p); }
  void operator*=(T p) { l*=p,r*=p; }
  auto operator*(interval t) { return interval(l*t.l, r*t.r); }
  void operator*=(interval t) { l*=t.l,r*=t.r; }

  T diff() const { return r - l; }
  bool has_value(T x) { return l <= x && x <= r; }
};

template <typename T>
struct multi_interval {
  std::vector<T> psums;

  template <typename U>
  multi_interval(const std::vector<U> &p):
    psums()
  {
    psums.push_back(U(0));
    for(int i = 1; i < p.size(); ++i) {
      psums.push_back(psums[i - 1] + T(p[i - 1]));
    }
    psums.push_back(U(1));
  }

  size_t find(T value) const {
    for(int i = 0; i < psums.size(); ++i) {
      if(value <= psums[i]) {
        return i - 1;
      }
    }
    if(std::fabs(value - 1.) <= 1e-6) {
      return size() - 1;
    }
    throw std::runtime_error("implementation error");
  }

  size_t size() const {
    return psums.size() - 1;
  }

  decltype(auto) operator[](int i) const {
    return interval<const T &>(psums[i], psums[i + 1]);
  }
};

struct Arithmetic {
  const CodingMeta &meta;

  using mask_t = uint64_t;
  #define NUM_BITS 53
  static constexpr char END_OF_TEXT = EOF;
  constexpr static mask_t fix_mask = ~mask_t(0) >> (sizeof(mask_t) * CHAR_BIT - NUM_BITS);

  static constexpr bool msb(mask_t x) {
    return x & (mask_t(1) << (NUM_BITS - 1));
  }

  static constexpr bool msb2(mask_t x) {
    return x & (mask_t(1) << (NUM_BITS - 2));
  }

  static constexpr mask_t set_msb(mask_t x) {
    return x | (mask_t(1) << (NUM_BITS - 1));
  }

  static constexpr mask_t unset_msb(mask_t x) {
    return ((x << 1) & fix_mask) >> 1;
  }

  static constexpr mask_t lshift(mask_t x) {
    return (x << 1) & fix_mask;
  }

  int pending_bits = 0;

  Arithmetic(const CodingMeta &meta):
    meta(meta)
  {}

  void rescale_a(interval<mask_t> &lu) {
    lu = {
      lshift(lu.l),
      lshift(lu.r) | 1
    };
  }

  void rescale_b(interval<mask_t> &lu) {
    lu = {
      unset_msb(lshift(lu.l)),
      set_msb(lshift(lu.r) | 1)
    };
  }

  void output_bits(DynamicBitset &bset, bool bit, int n) {
    while(n > 32) {
      bset.append<uint32_t>(bit ? ~uint32_t(0) : 0);
      n -= 32;
    }
    while(n > 16) {
      bset.append<uint16_t>(bit ? ~uint16_t(0) : 0);
      n -= 16;
    }
    while(n > 8) {
      bset.append<uint8_t>(bit ? ~uint8_t(0) : 0);
      n -= 8;
    }
    while(n) {
      bset.append_bit(bit);
      --n;
    }
  }

  DynamicBitset encode(const std::string &text) {
    DynamicBitset bset;
    if(meta.alphabet().find(END_OF_TEXT) == std::string::npos) {
      throw std::domain_error("unable to find end-of-text symbol");
    }
    auto len = meta.size();

    pending_bits = 0;
    interval<mask_t> lu(0, fix_mask);
    multi_interval<double> lr(meta.probabilities());

    auto &l = lu.left(), &u = lu.right();

    for(int i = 0; i < text.length(); ++i) {
      auto ind = meta.find_char(text[i]);
      interval<const double &> &&p = lr[ind];
      auto diff = (lu.diff() + 1);
      lu = {
        l + mask_t(p.l * diff),
        l + mask_t(p.r * diff) - 1
      };
      while(msb(l) == msb(u) || (msb2(l) && !msb2(u))) {
        if(msb(l) == msb(u)) {
          // rescale_a
          auto b = msb(l);
          bset.append_bit(b);
          output_bits(bset, !b, pending_bits);
          pending_bits = 0;
          rescale_a(lu);
        } else if(msb2(l) && !msb2(u)) {
          // rescale_b
          rescale_b(lu);
          ++pending_bits;
        }
      }
    }
    auto b = msb(l);
    bset.append_bit(b);
    output_bits(bset, !b, pending_bits);
    bset.append(std::bitset<NUM_BITS-1>(l));
    for(int i = 0; i < NUM_BITS - 1; ++i) {
      mask_t bit = mask_t(1) << (NUM_BITS - i - 2);
      bset.append_bit(l & bit);
    }
    while(!bset[bset.size() - 1]) {
      bset.pop();
    }
    return bset;
  }

  bool get_bit(const DynamicBitset &bset, int i) {
    return bset[i] ? 1 : 0;
  }

  mask_t push(mask_t v, bool bit) {
    return lshift(v) | (bit ? 1 : 0);
  }

  std::string decode(const DynamicBitset &bset) {
    std::string s;

    interval<mask_t> lu(0, fix_mask);
    multi_interval<double> lr(meta.probabilities());
    mask_t v = 0x0;
    auto str_bit = [=](mask_t val) {
      std::string s = "";
      for(int i = 0; i < NUM_BITS; ++i) {
        s += (val & (mask_t(1) << (NUM_BITS - i - 1))) ? '1' : '0';
      }
      return s;
    };

    int i;
    for(i = 0; i < NUM_BITS; ++i) {
      v = push(v, get_bit(bset, i));
    }
    while(1) {
      if(lu.l > lu.r) {
        throw std::runtime_error("wtf?!");
      }
      auto diff = lu.right() - lu.left() + 1;
      auto p_v = double(v - lu.l + 1) / diff;
      auto id = lr.find(p_v);
      auto &&p = lr[id];
      lu = {
        lu.l + mask_t(p.l * diff),
        lu.l + mask_t(p.r * diff) - 1
      };
      auto c = meta.get_char(id);
      s += c;
      if(c == END_OF_TEXT) {
        break;
      }
      while(msb(lu.l) == msb(lu.r) || (msb2(lu.l) && !msb2(lu.r))) {
        if(i >= bset.size()) {
          break;
        }
        if(msb(lu.l) == msb(lu.r)) {
          rescale_a(lu);
          v = push(v, get_bit(bset, i++));
        } else {
          rescale_b(lu);
          v = push(v, get_bit(bset, i++)) ^ (mask_t(1) << (NUM_BITS - 1));
        }
      }
    }

    return s;
  }

  #undef NUM_BITS
};

} // namespace coding

#endif /* end of include guard: CODINGARITHMETIC_HPP */
