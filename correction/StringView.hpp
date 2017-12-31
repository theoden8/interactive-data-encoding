#ifndef STRINGVIEW_HPP
#define STRINGVIEW_HPP

#include <string>

#include <Bitset.hpp>

template <typename ContT>
struct StringView {
  const ContT &v;
  constexpr StringView(ContT &v):v(v){}
  constexpr size_t size() const { return v.size(); }
  constexpr char operator[](size_t i) const { return v[i]?'1':'0'; }
  std::string substr(size_t start, size_t len) const {
    std::string s;
    for(int_t i = 0; i < len; ++i) {
      s += (*this)[start + i];
    }
    return s;
  }
  std::string str() const {
    std::string s; s.reserve(size());
    for(int i = 0; i < size(); ++i) {
      s += operator[](i);
    }
    return s;
  }
};

using BitsetStringView = StringView<bitset_t>;

template <typename T>
class TextBox {
  T charMap;
  std::string box;
public:
  const std::string &lines = box;
  int_t w, h;

public:
  constexpr TextBox(T charMap, int_t width, int_t height):
    charMap(charMap), w(width), h(height)
  {}

  constexpr int_t no_lines() const {
    return (charMap.size() + w - 1) / w;
  }

  size_t size() const {
    return lines.size();
  }

  void set_pos_f(float part) {
    int_t pos = int_t(part * no_lines() * w);
    set_pos(pos);
  }

  void set_pos(int_t pos) {
    pos = pos / w * w;
    for(int_t i = 0; i < h; ++i) {
      box = charMap.substr(
        std::min<int_t>(charMap.size(), pos + i*w),
        std::min<int_t>(charMap.size(), w)
      ) + '\n';
    }
  }
};

#endif /* end of include guard: STRINGVIEW_HPP */
