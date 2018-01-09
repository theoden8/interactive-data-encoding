#ifndef CODINGLZ77_HPP
#define CODINGLZ77_HPP

#include <Base.hpp>

namespace coding {

// adapted from https://github.com/manassra/LZ77-Compressor
struct LZ77 {
  const CodingMeta &meta;

  LZ77(const CodingMeta &meta):
    meta(meta)
  {}

  int window_size = -1;
  int textsize = -1;
  int lookahead_size = -1;

  static std::string repeat(std::string &&s, int times) {
    std::string ret;
    ret.reserve(times * s.length());
    for(int i = 0; i < times; ++i) {
      for(int j = 0; j < s.length(); ++j) {
        ret += s[j];
      }
    }
    return ret;
  }

  // inefficient, but works
  std::pair<int, int> find_longest_match(const std::string &text, int i) {
    auto ret = std::make_pair(0, -1);
    auto bufsize = std::min<int>(i + lookahead_size, text.length()) - i;
    for(int end = i + 2; end < i + bufsize; ++end) {
      auto start = std::max<int>(0, i - window_size);
      auto s = text.substr(i, end - i);
      for(int j = start; j < i; ++j) {
        auto rep = s.length() / (i - j);
        auto last = s.length() % (i - j);
        auto s_matched = repeat(text.substr(j, i-j), rep) + text.substr(j, last);
        if(s_matched == s && s.length() > ret.second) {
          ret.first = i - j;
          ret.second = s.length();
        }
      }
    }
    return ret;
  }

  // copied around so dont have to include special utility func
  static constexpr auto ceil_log2(long n) {
    int x = 0;
    while((1 << x) < n)++x;
    if(n==1)x=1;
    return x;
  }

  DynamicBitset encode(const std::string &text) {
    DynamicBitset bset;

    lookahead_size = std::min<int>(text.length(), std::max<int>(10 + std::log(text.length()), 15));
    window_size = std::cbrt(text.length());
    auto bits_sym = ceil_log2(meta.size());
    auto bits_distsize = ceil_log2(window_size);
    auto bits_lookahead = ceil_log2(lookahead_size);
    for(int i = 0; i < text.length();) {
      auto match = find_longest_match(text, i);
      match.first -= i;
      if(match.second > 0) {
      // encode the match
        bset.append_bit(1);
        for(int i = 0; i < bits_distsize+bits_lookahead; ++i) {
          if(i < bits_distsize) {
            auto bit = 1 << (bits_distsize - i - 1);
            bset.append_bit(match.first & bit);
          } else {
            int j = i - bits_distsize;
            auto bit = 1 << (bits_lookahead - j - 1);
            bset.append_bit(match.second & bit);
          }
        }
        i += match.second;
      } else {
      // emit raw symbol
        bset.append_bit(0);
        auto ind = meta.find_char(text[i]);
        for(int i = 0; i < bits_sym; ++i) {
          bset.append_bit(ind & (1 << (bits_sym - i - 1)));
        }
        ++i;
      }
    }
    return bset;
  }

  bool next_bit(const DynamicBitset &bset, int &i) {
    return bset[i++];
  }

  std::string decode(const DynamicBitset &bset) {
    auto bits_sym = ceil_log2(meta.size());
    auto bits_distsize = ceil_log2(window_size);
    auto bits_lookahead = ceil_log2(lookahead_size);
    std::string s;
    for(int i = 0; i < bset.size();) {
      auto flag = next_bit(bset, i);
      if(flag) {
      // decode the match
        auto match = std::make_pair(0, 0);
        for(int j = 0; j < bits_distsize+bits_lookahead; ++j) {
          if(j < bits_distsize) {
            if(next_bit(bset, i)) {
              match.first |= (1 << (bits_distsize - j - 1));
            }
          } else {
            int k = j - bits_distsize;
            if(next_bit(bset, i)) {
              match.second |= (1 << (bits_lookahead - k - 1));
            }
          }
        }
        for(int j = 0; j < match.second; ++j) {
          s += s[s.length() - match.first];
        }
      } else {
      // decode raw symbol
        int ind = 0;
        for(int j = 0; j < bits_sym; ++j) {
          if(next_bit(bset, i)) {
            ind |= (1 << (bits_sym - j - 1));
          }
        }
        s += meta.get_char(ind);
      }
    }
    return s;
  }
};

} // namespace coding

#endif /* end of include guard: CODING_LZ77_HPP */
