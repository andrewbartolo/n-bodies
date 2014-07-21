FILES = n-bodies.cc universe.cc body.cc pheap.c
OUT_BIN = n-bodies
CPPFLAGS = -Wall -pedantic -std=c++11
LIBS = 

build: $(FILES)
				$(CXX) $(CPPFLAGS) -O2 -o $(OUT_BIN) $(FILES) $(LIBS)

clean:
				rm -f *.o n-bodies backing_file

rebuild: clean build

debug: $(FILES)
				$(CXX) $(CPPFLAGS) -O0 -g -o $(OUT_BIN) $(FILES) $(LIBS)