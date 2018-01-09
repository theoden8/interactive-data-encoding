#ifndef CODINGMETA_HPP
#define CODINGMETA_HPP

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include <DynamicBitset.hpp>

namespace coding {

class CodingMeta {
  std::string alphabet_;
  std::vector <float> probabilities_;
 public:
  CodingMeta(std::string &alphabet, std::vector <float> &probabilities):
    alphabet_(alphabet), probabilities_(probabilities)
  {
    if(alphabet.length() != probabilities.size()) {
      throw std::runtime_error("alphabet length must match the number of probabilities");
    }
    auto sum = .0f;
    for(auto &p : probabilities) {
      sum += p;
    }
    if(std::abs(1.f-sum) > 1e-2) {
      throw std::runtime_error("invalid probabilities: must add up to 1, cur value " + std::to_string(sum));
    }
    for(auto &p : probabilities) {
      p /= 1.f/sum;
    }
  }
  const std::string &alphabet() const { return alphabet_; }
  const std::vector<float> &probabilities() const { return probabilities_; }
  size_t size() const { return alphabet_.length(); }
  char get_char(size_t i) const { return alphabet_[i]; }
  size_t find_char(char c) const { return alphabet_.find(c); }
  float get_prob(size_t i) const { return probabilities_[i]; }
};

} // namespace coding


#endif /* end of include guard: CODINGMETA_HPP */
