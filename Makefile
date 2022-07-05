frontend: src/ast.h src/frontend/lexer.lex src/frontend/parser.yacc
	cd src/frontend && yacc -d -o parser.cpp parser.yacc -t && lex -o lexer.cpp lexer.lex

test-frontend: frontend
	g++ -c src/frontend/*.cpp -Wall