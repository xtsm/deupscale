CXX = g++
EMCXX = em++
CXXFLAGS = -O2 -std=c++20

all: deupscale deupscale.js

clean:
	rm -f *.o deupscale deupscale.js deupscale.wasm

test: deupscale
	./test.sh

deupscale: deupscale.o deupscale_file.o main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

deupscale.js: deupscale.cpp deupscale_file.cpp deupscale.h deupscale_file.h
	$(EMCXX) $(CXXFLAGS) -o $@ -sEXPORTED_FUNCTIONS=_DeupscaleFile,_free -sEXPORTED_RUNTIME_METHODS=stringToNewUTF8 deupscale.cpp deupscale_file.cpp

deupscale.o: deupscale.cpp deupscale.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

deupscale_file.o: deupscale_file.cpp deupscale_file.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
