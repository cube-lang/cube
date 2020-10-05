all: build/ build/parser.cpp build/tokens.cpp

build/:
	mkdir -p $@
	cp -v src/* $@

build/parser.cpp:
	cd build/ &&  bison -d -o parser.cpp parser.y

build/tokens.cpp:
	cd build/ && lex -o tokens.cpp tokens.l
