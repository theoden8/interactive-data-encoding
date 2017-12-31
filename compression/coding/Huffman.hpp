#ifndef CODINGHUFFMAN_HPP
#define CODINGHUFFMAN_HPP

#include <limits>
#include <array>

#include <utility>
#include <type_traits>

#include <coding/Base.hpp>

namespace coding {

template <typename... Ts> void unroll(Ts...){}

// N is arity
template <size_t N>
struct HuffmanNode {
  HuffmanNode<N> *parent = nullptr;
  std::array<HuffmanNode *, N> subnodes_;
private:
  int index_ = -1;
  int childno_;
  int counter_;
public:

private:
  // variadic template unrolling
  template <size_t I>
  int setnull() noexcept {
    subnodes_[I] = nullptr;
    return 0;
  }
  template <size_t... Is>
  void setnull_each(std::index_sequence<Is...>) noexcept {
    unroll(setnull<Is>()...);
  }

public:
  HuffmanNode():
    parent(nullptr),
    subnodes_(),
    counter_(0),
    childno_(-1)
  {
    setnull_each(std::make_index_sequence<N>());
    if(parent != nullptr) {
      add_parent(*parent);
    }
  }

public:
  bool is_leaf() const noexcept {
    return !counter_;
  }

  bool is_root() const noexcept {
    return parent == nullptr;
  }

  size_t get_count() const noexcept {
    return counter_;
  }

  HuffmanNode<N> *child(size_t i) const {
    if(i > get_count()) {
      throw std::runtime_error("no child corresponding to the index");
    }
    return subnodes_[i];
  }

  void set_index(int index) noexcept {
    index_ = index;
  }

  int index() const noexcept {
    return index_;
  }

  void add_child(HuffmanNode<N> &child) {
    // set child
    if(get_count() == N) {
      throw std::runtime_error("unable to add child: tree capacity exceeded");
    }
    subnodes_[counter_++] = &child;
    child.childno_ = get_count() - 1;
    // set parent
    if(child.parent != nullptr) {
      throw std::runtime_error("unable to add child: tree capacity exceeded");
    }
    child.parent = this;
  }

  template <typename... Ts>
  void add_children(Ts&... children) {
    unroll(
      [&](auto &c) {
        add_child(c);
        return 0;
      }(children)...
    );
  }

  void add_parent(HuffmanNode<N> &parent) {
    parent.add_child(*this);
  }

  template <typename = std::enable_if<N == 2>>
  void get_code(DynamicBitset &bset) const noexcept {
    if(parent == nullptr) {
      return;
    }
    parent->get_code(bset);
    bset.append_bit(childno_);
  }

  template <typename = std::enable_if<N == 2>>
  DynamicBitset get_code() const noexcept {
    DynamicBitset bset;
    if(is_root()) {
      bset.append_bit(0);
    } else {
      get_code(bset);
    }
    return bset;
  }
};

struct Huffman {
  using number_t = uint32_t;

  template <typename T>
  static void swap(T &x, T &y) {
    if(x == y) return;
    T t = x; x = y; y = t;
  }

  template <typename X, typename Y>
  static void sort(X &x, Y &y) {
    for(int i = 0; i < x.size() - 1; ++i) {
      for(int j = i + 1; j < x.size(); ++j) {
        if(x[i] > x[j]) {
          swap(x[i], x[j]);
          swap(y[i], y[j]);
        }
      }
    }
  }

  template <typename T>
  static constexpr int ceil_log2(T x) {
    auto t = x, r = 0;
    while(t>>=1)r+=1;
    return r-((1<<r==x)?1:0);
  }

  const CodingMeta &meta;
  HuffmanNode<2> *huffman_tree;
  // huffman tree stored altogether
  std::vector<HuffmanNode<2>> leaves_;
  std::vector<HuffmanNode<2>> nodes_;

  Huffman(const CodingMeta &meta):
    meta(meta),
    huffman_tree(nullptr)
  {}

  DynamicBitset encode(const std::string &text) {
    DynamicBitset bset;
    // sort this using bucket sort, for example
    auto len = meta.size();
    auto a = meta.alphabet();
    auto p = meta.probabilities();
    sort(p, a);
    // huffman coding
    int i = 0, j = 0;
    std::vector<float> q(len, 1.0f);
    leaves_ = std::vector<HuffmanNode<2>>(len);
    nodes_ = std::vector<HuffmanNode<2>>(len);
    // extract min in constant time N-1 times
    for(int i = 0; i < len; ++i) {
      leaves_[i].set_index(i);
      nodes_[i].set_index(len + i);
    }
    for(int k = 0; k < len - 1; ++k) {
      if(i + 1 < len && p[i] + p[i + 1] <= p[i] + q[j] && p[i] + p[i + 1] <= q[j] + q[j + 1]) {
        q[k] = p[i] + p[i + 1];
        nodes_[k].add_children(leaves_[i], leaves_[i + 1]);
        i += 2;
      } else if(((i + 1 < len && p[i] + q[j] <= p[i] + p[i + 1]) || i + 1 >= len) && (i < len && p[i] + q[j] <= q[j] + q[j + 1])) {
        q[k] = p[i] + q[j];
        nodes_[k].add_children(leaves_[i], nodes_[j]);
        ++i, ++j;
      } else {
        q[k] = q[j] + q[j + 1];
        nodes_[k].add_children(nodes_[j], nodes_[j + 1]);
        j += 2;
      }
    }
    // encode
    std::vector<DynamicBitset> dict(256);
    for(int i = 0; i < len; ++i) {
      dict[uint8_t(a[i])] = leaves_[i].get_code();
    }
    for(auto &ch : text) {
      bset.append(dict[uint8_t(ch)]);
    }
    // set the attribute
    huffman_tree = (len > 1) ? &nodes_[len - 2] : &leaves_[0];
    return bset;
  }

  double average_length() {
    double avglen = 0.;
    auto a = meta.alphabet();
    auto p = meta.probabilities();
    sort(p, a);
    std::vector<int> lengths(meta.size(), 0);
    for(int i = 0; i < meta.size(); ++i) {
      avglen += p[i] * leaves_[i].get_code().size();
    }
    return avglen;
  }

  // take a tree visitor and follow the code, emit on leaves
  std::string decode(const DynamicBitset &bset) {
    auto len = meta.size();
    auto a = meta.alphabet();
    auto p = meta.probabilities();
    sort(p, a);
    // decode
    auto vis = huffman_tree;
    std::string s = "";
    DynamicBitset cur;
    for(int i = 0; i < bset.size(); ++i) {
      if(!vis->is_leaf()) {
        vis = vis->child(bset[i]);
      }
      if(vis->is_leaf()) {
        s += a[vis->index()];
        vis = huffman_tree;
      }
    }
    if(!vis->is_root()) {
      throw std::domain_error("unable to fully decode the text");
    }
    return s;
  }
};

} // namespace coding

#endif /* end of include guard: CODINGHUFFMAN_HPP */
