#ifndef CODING_HPP
#define CODING_HPP

#include <cstdlib>
#include <string>
#include <iostream>

enum class DecodeStatus : size_t { NO_ERR, DETECTED, CORRECTED, NO_DECODE_STATUSES };

#include <Bitset.hpp>

template <typename T, template <typename> class BITSET>
static std::string str(const BITSET<T> &v){std::string s;for(int i=0;i<v.size();++i)s+='0'+(v[i]?1:0);return s;}

#include <HammingCode.hpp>
#include <RepititionCode.hpp>
#include <HadamardCode.hpp>

#endif /* end of include guard: CODING_HPP */
