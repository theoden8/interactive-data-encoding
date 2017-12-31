#ifndef CODINGBLOCK_HPP
#define CODINGBLOCK_HPP

#include <climits>
#include <cstdint>
#include <iostream>

#include <coding/Base.hpp>

namespace coding {

struct Block {
  using block_t = uint32_t;
  static constexpr int MAX_BLOCK_SIZE_BITS = 5;

  const CodingMeta &meta;
  block_t block_size = 0x00;

  Block(const CodingMeta &meta):
    meta(meta)
  {}

  DynamicBitset encode(const std::string &text) {
    DynamicBitset bset;
    int n = meta.size();
    int blqsize = 0;
    while((1 << blqsize) < n)++blqsize;
    if(n==1)blqsize=1;
    // encode
    for(char c : text) {
      int pos = meta.find_char(c);
      for(int i = 0; i < blqsize; ++i) {
        bset.append_bit(pos & 1 << (blqsize - i - 1));
      }
    }
    // set the attributes
    block_size = blqsize;
    return bset;
  }

  double average_length() {
    double avglen = 0.;
    for(int i = 0; i < meta.size(); ++i) {
      avglen += meta.get_prob(i) * block_size;
    }
    return avglen;
  }

  std::string decode(const DynamicBitset &bset) {
    auto len = bset.size() / block_size;
    std::string s;
    s.reserve(len);
    for(int i = 0; i < len; ++i) {
      int index = i * block_size;
      int pos = 0x00;
      for(int j = 0; j < block_size; ++j) {
        pos += bset[index + j] << (block_size - j - 1);
      }
      s += meta.get_char(pos);
    }
    return s;
  }
};

} // namespace coding

#endif /* end of include guard: CODINGBLOCK_HPP */
