#include <stdio.h>

int main(int argc, char** argv) {
	
	FILE *fp = fopen("checkertest.txt", "r");
	yyin = fp;

	//lex through the input
	yylex();
	return 0;
}
