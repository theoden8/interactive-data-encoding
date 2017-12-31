#ifndef CODINGLZW_HPP
#define CODINGLZW_HPP

#include <coding/Base.hpp>

#include <unordered_map>
#include <array>

namespace coding {

template <size_t N = 256>
struct Tree {
  std::array<Tree *, N> sub = {nullptr};
  char c;
  uint64_t val;
  Tree(char c, uint64_t val):
    c(c), val(val)
  {}

  void add(unsigned char c, uint64_t v) {
    sub[c] = new Tree<N>(c, v);
  }

  bool has(unsigned char c) const noexcept {
    return sub[c] != nullptr;
  }

  Tree<N> *child(unsigned char c) {
    return sub[c];
  }

  void free() {
    for(int i = 0; i < N; ++i) {
      if(sub[i]) {
        sub[i]->free();
        delete sub[i];
      }
    }
  }
};

struct LZW {
  static constexpr char END_OF_TEXT = EOF;

  const CodingMeta &meta;

  LZW(const CodingMeta &meta):
    meta(meta)
  {}

  static constexpr auto ceil_log2(long n) {
    int x = 0;
    while((1 << x) < n)++x;
    if(n==1)x=1;
    return x;
  }

  int block_size = -1;

  DynamicBitset encode(const std::string &text_) {
    auto text = text_ + END_OF_TEXT;
    DynamicBitset bset;
    Tree<256> dict(0,0);
    int size = 0;
    std::vector<uint64_t> codes;
    for(auto a : meta.alphabet()) {
      dict.add(a, size++);
    }
    Tree<256> *w = &dict;
    for(auto c : text) {
      if(c == END_OF_TEXT) {
        codes.push_back(w->val);
        break;
      } else {
        if(w->has(c)) {
          w = w->child(c);
        } else {
          codes.push_back(w->val);
          w->add(c, size++);
          w = dict.child(c);
        }
      }
    }
    block_size = ceil_log2(size);
    for(auto &x : codes) {
      for(int i = 0; i < block_size; ++i) {
        bset.append_bit(x & (uint64_t(1) << (block_size - i - 1)));
      }
    }
    dict.free();
    return bset;
  }

  uint64_t decode_symbol(const DynamicBitset &bset, int i) {
    uint64_t x = 0;
    for(int j = 0; j < block_size; ++j) {
      if(bset[i + j]) {
        x |= uint64_t(1) << (block_size - j - 1);
      }
    }
    return x;
  }

  std::string decode(const DynamicBitset &bset) {
    std::string s;
    std::unordered_map<uint64_t, std::string> dict;
    for(int i = 0; i < meta.size(); ++i) {
      dict[dict.size()] = std::string() + meta.get_char(i);
    }
    if(bset.size() == 0) {
      return s;
    }
    std::string w;
    for(int i = 0; i < bset.size(); i += block_size) {
      auto x = decode_symbol(bset, i);
      if(!i) {
        w = dict[x];
        s += w;
        continue;
      }
      std::string e;
      if(dict.find(x) != dict.end()) {
        e = dict[x];
      } else if(x == dict.size()) {
        e = w + w[0];
      } else {
        throw std::runtime_error("compression failed");
      }
      s += e;
      dict[dict.size()] = w + e[0];
      w = e;
    }
    return s;
  }
};

} // namespace coding

#endif /* end of include guard: CODINGLZW_HPP */
