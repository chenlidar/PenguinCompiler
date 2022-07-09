frontend: src/structure/ast.h src/frontend/lexer.lex src/frontend/parser.yacc
	cd src/frontend && yacc -d -o parser.cpp parser.yacc -t -r all && lex -o lexer.cpp lexer.lex

test-frontend: frontend
	clang++-10 -c src/frontend/*.cpp -Wall

compiler:
	clang++-10 