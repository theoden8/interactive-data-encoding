#ifndef CODINGBASE_HPP
#define CODINGBASE_HPP

#include <climits>

#include <coding/DynamicBitset.hpp>
#include <coding/CodingMeta.hpp>

namespace coding {

struct Base {
  const CodingMeta &meta;

  Base(const CodingMeta &meta):
    meta(meta)
  {}

  DynamicBitset encode(const std::string &text) {
    DynamicBitset bset;
    bset.append(text);
    return bset;
  }

  double average_length() {
    double avglen = 0.;
    for(int i = 0; i < meta.size(); ++i) {
      avglen += meta.get_prob(i) * 8;
    }
    return avglen;
  }

  std::string decode(const DynamicBitset &bset) {
    if(bset.size() & 0x07) {
      throw std::runtime_error("the bitset size must divide 8");
    }
    auto len = bset.size() / CHAR_BIT;
    std::string s;
    for(int i = 0; i < len; ++i) {
      unsigned char c = 0x00;
      for(int j = 0; j < CHAR_BIT; ++j) {
        if(bset[i * CHAR_BIT + CHAR_BIT - j - 1]) {
          c |= 1 << j;
        }
      }
      s += c;
    }
    return s;
  }
};

} // namespace coding


#endif /* end of include guard: CODINGBASE_HPP */
