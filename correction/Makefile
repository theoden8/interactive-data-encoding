GUIHEADERS = GLUtil.hpp StringView.hpp
CODINGHEADERS = Bitset.hpp Random.hpp Bithacks.hpp Coding.hpp HammingCode.hpp RepititionCode.hpp HadamardCode.hpp

# OPTFLAGS = -g3
OPTFLAGS = -Ofast -fPIC -m64 -march=native
CFLAGS = -std=c++1y -I. $(OPTFLAGS)
LDFLAGS = $(shell ./link_graphics) -lgmp -lgmpxx

all:; $(MAKE) -j$(shell getconf _NPROCESSORS_ONLN) _all
_all: correction

correction: main.cpp $(GUIHEADERS) $(CODINGHEADERS)
		$(CXX) $(LDFLAGS) $(CFLAGS) main.cpp -o correction

test_codings: test_codings.cpp $(CODINGHEADERS)
		$(CXX) $(CFLAGS) test_codings.cpp -o test_codings

test: test_codings
		./test_codings

clean :
		rm -vf correction test_codings
		rm -rvf *.dSYM
