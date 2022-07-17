VPATH=src/frontend:src/structure:src/util:src/backend
frontend: src/structure/ast.h src/frontend/lexer.lex src/frontend/parser.yacc
	cd src/frontend && yacc -d -o parser.cpp parser.yacc -t -r all && lex -o lexer.cpp lexer.lex

test-frontend: frontend
	clang++-10 -c src/frontend/*.cpp -Wall

test-ir: frontend treeIR.cpp treeIR.hpp ast.cpp ast.h table.hpp canon.cpp canon.hpp utils.hpp
	clang++-10 -O0 -g \
	src/frontend/parser.cpp \
	src/frontend/lexer.cpp \
	src/structure/ast.cpp \
	src/backend/canon.cpp \
	test/src/test-ir.cpp src/structure/treeIR.cpp \
	src/util/temptemp.cpp src/util/templabel.cpp \
	-I./src -o build/test-ir

test-asm: frontend
	clang++-10 -O2 -g \
	src/**/*.cpp \
	src/main.cpp \
	-I./src -o build/test-asm

compiler:
	clang++-10 

.PHONY: test-asm
