all: cube

OBJS = build/parser.o  \
	build/codegen.o \
	build/linker_common.o \
	build/main.o    \
	build/tokens.o  \
	build/core.o  \
	build/nassignment.o \
	build/nbinaryoperator.o \
	build/nblock.o \
	build/nconstdeclaration.o \
	build/ndouble.o \
	build/nexpressionstatement.o \
	build/nfunction.o \
	build/nidentifier.o \
	build/ninteger.o \
	build/ninternaldeclaration.o \
	build/nmethodcall.o \
	build/nprogram.o \
	build/nreturn.o \
	build/nstring.o \
	build/nsuperdeclaration.o \
	build/nthing.o \
	build/nvariabledeclaration.o

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++20 -lstdc++fs -g
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses -rdynamic
LIBS = `$(LLVMCONFIG) --system-libs --libs all` -L./build/ -L.

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


cube: build/ $(COMMON) $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS) $(LDFLAGS) -O3
