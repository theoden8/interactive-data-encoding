 #ifndef CODINGSHANNON_HPP
 #define CODINGSHANNON_HPP

#include <Base.hpp>

namespace coding {

namespace detail {

} // namespace detail

struct Shannon {
  const CodingMeta &meta;

  Shannon(const CodingMeta &meta):
    meta(meta)
  {}

  template <typename T>
  static void swap(T &x, T &y) {
    if(x == y) return;
    T t = x; x = y; y = t;
  }

  template <typename X, typename Y>
  static void sort(X &x, Y &y) {
    for(int i = 0; i < x.size() - 1; ++i) {
      for(int j = i + 1; j < x.size(); ++j) {
        if(x[i] < x[j]) {
          swap(x[i], x[j]);
          swap(y[i], y[j]);
        }
      }
    }
  }

  std::vector<DynamicBitset> dict;

  DynamicBitset encode(const std::string &text) {
    DynamicBitset bset;
    auto len = meta.size();
    auto alph = meta.alphabet();
    auto probs = meta.probabilities();
    // sort alphabet and probabilities by probabilities
    sort(probs, alph);
    std::vector<float> cum_probs;
    cum_probs.push_back(0);
    for(int i = 1; i < len; ++i) {
      cum_probs.push_back(cum_probs[i - 1] + probs[i - 1]);
    }
    // fill the dictionary
    dict = std::vector<DynamicBitset>(len);
    for(int i = 0; i < len; ++i) {
      if(probs[i] == 0.) {
        continue;
      }
      const int L = std::max<int>(1, std::ceil(-std::log2(probs[i])));
      using uint = uint32_t;
      uint x = uint(cum_probs[i] * (uint(1) << L)) & ((uint(1) << L) - 1);
      for(int j = 0; j < L; ++j) {
        dict[i].append_bit(x & (uint32_t(1) << (L - j - 1)));
      }
    }
    // encode text
    for(auto c : text) {
      bset.append(dict[alph.find(c)]);
    }
    return bset;
  }

  double average_length() {
    auto a = meta.alphabet();
    auto p = meta.probabilities();
    sort(p, a);

    double avglen = 0;
    for(int i = 0; i < meta.size(); ++i) {
      avglen += p[i] * dict[i].size();
    }
    return avglen;
  }

  std::string decode(const DynamicBitset &bset) {
    std::string s;
    auto len = meta.size();
    auto alph = meta.alphabet();
    auto probs = meta.probabilities();
    sort(probs, alph);
    // decoding
    int counter = len;
    std::vector<bool> states(len, 1);
    for(int i = 0, j = 0; i < bset.size(); ++i, ++j) {
      for(int k = 0; k < len; ++k) {
        if(counter == 0) {
          throw std::runtime_error("unable to decode");
        }
        if(states[k]) {
          if(dict[k].size() <= j || bset[i] != dict[k][j]) {
            // reject a state
            --counter;
            states[k] = 0;
          }
        }
      }
      if(counter == 1) {
        for(int k = 0; k < len; ++k) {
          if(states[k]) {
            if(dict[k].size() == j + 1) {
              s += alph[k];
              for(int t = 0; t < len; ++t) {
                states[t] = 1;
              }
              counter = len;
              j = -1;
            }
            break;
          }
        }
      }
    }
    return s;
  }
};

} // namespace coding


 #endif /* end of include guard: CODINGSHANNON_HPP */
