all: cube

OBJS = build/parser.o  \
	build/codegen.o \
	build/main.o    \
	build/tokens.o  \
	build/core.o  \

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++20 -lstdc++fs
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses
LIBS = `$(LLVMCONFIG) --system-libs --libs all`

.PHONY: clean
clean:
	$(RM) -rfv build/

build/:
	mkdir -vp $@
	cp -v src/* $@

build/parser.cpp: build/parser.y
	bison -d -o $@ $^

build/parser.hpp: build/parser.cpp

build/tokens.cpp: build/tokens.l build/parser.hpp
	flex -o $@ $^

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $<


cube: build/ $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
