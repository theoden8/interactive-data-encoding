#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include <bitset>
#include <Coding.hpp>

using bitset_t = Bitset<uint32_t>;

constexpr int ceil_log2(int x) {
  auto t = x, r = 0;
  while(t>>=1)r+=1;
  return r-((1<<r==x)?1:0);
}

template <typename T>
Bitset<T> genmsg(int n) {
  Bitset<T> v(n);
  v.set_random(0, n);
  return v;
}

template <typename T>
Bitset<T> transmit(const Bitset<T> &v, int e=1) {
  if(v.size() == 0) return v;
  e = rand() % (e + 1);
  auto v2 = v;
  for(int i = 0; i < e; ++i) {
    int ind = rand() % v.size();
    v2.set_bool(ind, !v[ind]);
  }
  return v2;
}

template <typename CoderT>
void test_random_case(CoderT &&coder, int_t len, int e) {
  bitset_t msg = genmsg<uint32_t>(len);
  auto enc = coder.encode(msg);
  bitset_t rec = transmit(enc, e);
  auto dec = coder.decode(rec);
  if(str(dec) != str(msg)) {
    /* printf("MISMATCH: [%s] != [%s]\n", str(dec).c_str(), str(msg).c_str()); */
    throw std::logic_error("did not correct <= " + std::to_string(e) + " errors");
  }
}

#ifndef NO_TESTS
#define NO_TESTS 1000
#endif /* ifndef NO_TESTS */
int main() {
  srand(time(NULL));
  /* srand(0); */
  for(int r = 2; r <= 15; ++r) {
    int k = HammingCode::get_source_dim(r);
    printf("Hamming code: r == %d\n", r);
    for(int i = 0; i < NO_TESTS; ++i) {
      test_random_case(HammingCode(r), k, 1);
    }
  }
  for(int e = 1; e <= 20; ++e) {
    printf("Repitition code: e == %d\n", e);
    for(int i = 0; i < NO_TESTS; ++i) {
      int len = rand() % 10000 + 1;
      test_random_case(RepititionCode(e), len, e);
    }
  }
  for(int k = 3; k <= 15; ++k) {
    const int_t e = ((1 << (k-1)) - 1) / 2;
    printf("Hadamard code: k == %d (e == %lld)\n", k, e);
    for(int i = 0; i < NO_TESTS; ++i) {
      test_random_case(HadamardCode(k), k, e);
    }
  }
}
