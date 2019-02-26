/*EECE6083: PARSER
 * Programmer: David Luria
 * Date updated: 2/24/2019
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <list>
using namespace std;

//Line number
int lineNumber = 1;

//Token Types
enum tokenType {
	SEMICOLON = ';', LPAREN = '(', RPAREN = ')', COLON = ':', PLUS = '+', MINUS = '-', STAR = '*', SLASH = '/', LBRACKET = '[', RBRACKET = ']', LBRACE = '{', RBRACE = '}', OR = '|', AND = '&', NOT = '!', COMMA = ',', MODULUS = '%',
	LESSTHAN = '<', GREATERTHAN = '>', SINGLEQUOTE = '\'', DOUBLEQUOTE = '"', PERIOD = '.', ASSIGN = 257, EQUAL, LESSTHANEQUAL, GREATERTHANEQUAL, NOTEQUAL, RETURN, WHILE, IF, THEN, ELSE, FOR, PROGRAM, BEGIN, END, IN, OUT, GLOBAL,
	IS, TRUE, FALSE, PROCEDURE, INTEGER, FLOAT, CHAR, STRING, IDENTIFIER, INTVAL, FLOATVAL, STRINGVAL, UNKNOWN, ENDFILE
};

//Token structure
struct token {
	tokenType type;
	string name;
};

//Symbol Table
list<token> symbolTable = { {RETURN, "return"}, {WHILE, "while"} , {IF, "if"} , {THEN, "then"} , {ELSE, "else"} , {FOR, "for"} , {PROGRAM, "program"} , {BEGIN, "begin"} ,
{END, "end"} , {IN, "in"} , {OUT, "out"} , {GLOBAL, "global"} , {IS, "is"} , {TRUE, "true"} , {FALSE, "false"} , {PROCEDURE, "procedure"} , {INTEGER, "integer"}, {FLOAT, "float"}, {CHAR, "char"}, {STRING, "string"} };

//Nonterminals
enum nonTerminal {
	PROGRAM_MAIN, PROGRAM_HEADER, PROGRAM_BODY, DECLARATION, PROCEDURE_DECLARATION, PROCEDURE_HEADER, PARAMETER_LIST, PARAMETER, PROCEDURE_BODY, VARIABLE_DECLARATION, TYPE_DECLARATION, TYPE_MARK, BOUND, STATEMENT, PROCEDURE_CALL, 
	ASSIGNMENT_STATEMENT, DESTINATION, IF_STATEMENT, LOOP_STATEMENT, RETURN_STATEMENT, EXPRESSION, ARITHOP, RELATION, TERM, FACTOR, NAME, ARGUMENT_LIST
};

//Scanner Method
token ScanOneToken(FILE *file)
{
	//Load in the character
	token outToken;
	string tokenName;
	char currentChar = getc(file);
	char nextChar;

	//Skip whitespace
	while (isspace(currentChar))
	{
		if (currentChar == '\n') {
			lineNumber++;
		}
		currentChar = getc(file);
	}

	//Skip comments
	if (currentChar == '/') {
		//Check next char
		nextChar = getc(file);
		//See if in-line comment
		if (nextChar == '/')
		{
			//Keep skipping until next line
			while (currentChar != '\n')
			{
				currentChar = getc(file);
			}
			//Skip remaining whitespace
			while (isspace(currentChar)) {
				currentChar = getc(file);
			}
		}
		//See if multi-line comment
		else if (nextChar == '*')
		{
			//Skip until comment end (*/)
			while ((currentChar != '*') | (nextChar != '/'))
			{
				currentChar = getc(file);
				if (currentChar == '*')
				{
					nextChar = getc(file);
				}
			} 
			currentChar = getc(file);
			//Skip remaining whitespace
			while (isspace(currentChar)) {
				currentChar = getc(file);
			}
		}
		//Otherwise, put the nextChar back
		else
		{
			//ANYTIME nextchar is used, finish with ungetc if the next character is not needed
			ungetc(nextChar, file);
		}
	}

	outToken.name.push_back(currentChar);

	switch (currentChar)
	{
	//Regular single character tokens
	case ';' : case '(': case ')': case '+': case '*': case '/': case '[': case ']': case '{': case '}': case '|': case '&': case ',': case '%': case '.': case '-':
		outToken.type = static_cast<tokenType>(currentChar);
		break;
	case '!':
		nextChar = getc(file);
		//Check if NOTEQUAL
		if (nextChar == '=')
		{
			outToken.type = NOTEQUAL;
			outToken.name.push_back(nextChar);
		}
		//Else, recognize as NOT logic
		else
		{
			ungetc(nextChar, file);
			outToken.type = NOT;
		}
		break;
	case '>':
		nextChar = getc(file);
		//Check if GREATERTHANEQUAL
		if (nextChar == '=')
		{
			outToken.type = GREATERTHANEQUAL;
			outToken.name.push_back(nextChar);
		}
		//Else, recognize as normal GREATERTHAN
		else
		{
			ungetc(nextChar, file);
			outToken.type = GREATERTHAN;
		}
		break;
	case '<':
		nextChar = getc(file);
		//Check if LESSTHANEQUAL
		if (nextChar == '=')
		{
			outToken.type = LESSTHANEQUAL;
			outToken.name.push_back(nextChar);
		}
		//Else, recognize as normal LESSTHAN
		else
		{
			ungetc(nextChar, file);
			outToken.type = LESSTHAN;
		}
		break;
	case ':':
		nextChar = getc(file);
		//Check if assignment token
		if (nextChar == '=')
		{
			outToken.type = ASSIGN;
			outToken.name.push_back(nextChar);
		}
		//Else, recognize as COLON
		else
		{
			ungetc(nextChar, file);
			outToken.type = COLON;
		}
		break;
	case '=':
		nextChar = getc(file);
		//Check if comparison (==)
		if (nextChar == '=')
		{
			outToken.type = EQUAL;
			outToken.name.push_back(nextChar);
		}
		//Else, '=' is not in this language so return UNKNOWN
		else
		{
			ungetc(nextChar, file);
			outToken.type = UNKNOWN;
		}
		break;
	//Single quote case (string)
	case '\'':
		//Keep scanning until next single quote reached
		do
		{
			currentChar = getc(file);
			outToken.name.push_back(currentChar);
		} while (currentChar != '\'');
		outToken.type = STRINGVAL;
		break;
	//Double quote string case
	case '"':
		//Keep scanning until next double quote reached
		do
		{
			currentChar = getc(file);
			outToken.name.push_back(currentChar);
		} while (currentChar != '"');
		outToken.type = STRINGVAL;
		break;
	case EOF:
		outToken.type = ENDFILE;
		outToken.name = "EOF";
		break;
	default:
		outToken.type = UNKNOWN;
		break;
	}
	//Identifier/Reserved word case
	if (((currentChar >= 'A') & (currentChar <= 'Z')) | ((currentChar >= 'a') & (currentChar <= 'z')) | (currentChar == '_'))
	{
		bool inSymTable = false;
		//Keep scanning a-zA-Z0-9_
		while ((currentChar >= 'A' & currentChar <= 'Z') | (currentChar >= 'a' & currentChar <= 'z') | currentChar == '_' | (currentChar <= '0' & currentChar >= '9'))
		{
			//word += currentChar;
			currentChar = getc(file);
			outToken.name.push_back(currentChar);
		}
		//Put back last character
		ungetc(currentChar, file);
		outToken.name.pop_back();
		//Check if reserved word (EDIT: USE A LIST LOOKUP OR SYMBOL TABLE RATHER THAN THIS CRAPPY ALGORITHM)
		list<token>::iterator i;
		for (i = symbolTable.begin(); i != symbolTable.end(); i++)
		{
			if (i->name == outToken.name)
			{
				outToken.type = i->type;
				inSymTable = true;
			}
		}
		//If not reserved, recognize as identifier
		if (!inSymTable)
		{
			outToken.type = IDENTIFIER;
			symbolTable.push_back(outToken);
		}
	}
	//Number case
	else if (currentChar >= '0' & currentChar <= '9')
	{
		//Keep scanning while numbers are read
		while (currentChar >= '0' & currentChar <= '9')
		{
			currentChar = getc(file);
			outToken.name.push_back(currentChar);
		}
		//If a decimal is encountered, recognize as float (NOTE: ADD double TYPE)
		if (currentChar == '.')
		{
			do
			{
				currentChar = getc(file);
				outToken.name.push_back(currentChar);
			} while (currentChar >= '0' & currentChar <= '9');
			outToken.type = FLOATVAL;
		}
		//If no decimal encountered, recognize as integer value
		else
		{
			outToken.type = INTVAL;
		}
		ungetc(currentChar, file);
		outToken.name.pop_back();
	}
	if (outToken.type == UNKNOWN) {
		cout << "ERROR: UNKOWN TOKEN: " << outToken.name << " at line "<< lineNumber << endl;
	}
	return outToken;
}

//Parser Method
bool parse(token currentToken, FILE *file, nonTerminal nonTerminal) {
	bool isParsed = false;
	string expectedToken;
	switch (nonTerminal) {

	//IF PARSE
	case IF_STATEMENT:
		if (currentToken.type != IF) {
			expectedToken = "if";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != LPAREN) {
			expectedToken = "'('";
			break;
		}
		currentToken = ScanOneToken(file);
		parse(currentToken, file, EXPRESSION);
		currentToken = ScanOneToken(file);
		if (currentToken.type != RPAREN) {
			expectedToken = "')'";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != THEN) {
			expectedToken = "then";
			break;
		}
		currentToken = ScanOneToken(file);
		while (currentToken.type != ELSE & currentToken.type != END) {
			parse(currentToken, file, STATEMENT);
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type == ELSE) {
			currentToken = ScanOneToken(file);
			while (currentToken.type != END) {
				parse(currentToken, file, STATEMENT);
				currentToken = ScanOneToken(file);
			}
		}
		if (currentToken.type != END) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != IF) {
			expectedToken = "if";
			break;
		}
		isParsed = true;
		break;

	//EXPRESSION PARSE
	case EXPRESSION:
		if (currentToken.type == NOT) {
			currentToken = ScanOneToken(file);
		}
		if (parse(currentToken, file, ARITHOP)) {
			isParsed = true;
			break;
		}
		parse(currentToken, file, EXPRESSION);
		currentToken = ScanOneToken(file);
		if (currentToken.type == AND) {
			cout << "and expression" << endl;
		}
		else if (currentToken.type == OR) {
			cout << "or expression" << endl;
		}
		else {
			expectedToken = "and/or";
			break;
		}
		currentToken = ScanOneToken(file);
		parse(currentToken, file, ARITHOP);
		isParsed = true;
		break;

	//ARITHOP PARSE
	case ARITHOP:
		if (parse(currentToken, file, RELATION)) {
			isParsed = true;
			break;
		}
		parse(currentToken, file, ARITHOP);
		currentToken = ScanOneToken(file);
		if (currentToken.type == PLUS) {
			cout << "+ arithOp" << endl;
		}
		else if (currentToken.type == MINUS) {
			cout << "- arithOp" << endl;
		}
		else {
			expectedToken = "+/-";
			break;
		}
		currentToken = ScanOneToken(file);
		parse(currentToken, file, RELATION);
		isParsed = true;
		break;

	//RELATION PARSE
	case RELATION:
		if (parse(currentToken, file, TERM)) {
			isParsed = true;
			break;
		}
		parse(currentToken, file, RELATION);
		currentToken = ScanOneToken(file);
		if (currentToken.type == LESSTHAN) {
			cout << "< relation" << endl;
		}
		else if (currentToken.type == LESSTHANEQUAL) {
			cout << "<= relation" << endl;
		}
		else if (currentToken.type == GREATERTHAN) {
			cout << "> relation" << endl;
		}
		else if (currentToken.type == GREATERTHANEQUAL) {
			cout << ">= relation" << endl;
		}
		else if (currentToken.type == EQUAL) {
			cout << "== relation" << endl;
		}
		else if (currentToken.type == NOTEQUAL) {
			cout << "!= relation" << endl;
		}
		else {
			expectedToken = "compare";
			break;
		}
		currentToken = ScanOneToken(file);
		parse(currentToken, file, TERM);
		isParsed = true;
		break;

	//TERM PARSE
	case TERM:
		if (parse(currentToken, file, FACTOR)) {
			isParsed = true;
			break;
		}
		parse(currentToken, file, TERM);
		currentToken = ScanOneToken(file);
		if (currentToken.type == STAR) {
			cout << "* term" << endl;
		}
		else if (currentToken.type == SLASH) {
			cout << "/ term" << endl;
		}
		else {
			expectedToken = "* or /";
			break;
		}
		currentToken = ScanOneToken(file);
		parse(currentToken, file, FACTOR);
		isParsed = true;
		break;

	//FACTOR PARSE
	case FACTOR:
		if (currentToken.type == IDENTIFIER) {
			isParsed = true;
			break;
		}

	//NO PARSE FOUND
	default:
		cout << "Error: No parse for " << nonTerminal << " at line " << lineNumber << endl;
		expectedToken = "parsable nonterminal";
		break;
	}
	if (isParsed) {
		cout << nonTerminal << " parsed successfully" << " at line " << lineNumber << endl;
	}
	else {
		cout << "BLURGHARG *Dies* (Expected " << expectedToken << ")" << " at line " << lineNumber << endl;
	}
	return isParsed;
}

/*
void exprParse(token currentToken, FILE *file) {
	if (currentToken.type == NOT) {
		currentToken = ScanOneToken(file);
	}
	//Call arithOp Parse

}

void ifParse(token currentToken, FILE *file) {
	if (currentToken.type == IF) {
		currentToken = ScanOneToken(file);
		if (currentToken.type == LPAREN) {
			currentToken = ScanOneToken(file);
			//Call Expression Parse
		}
		else {
			pukeNDie("(");
		}
	}
	else {
		pukeNDie("if");
	}
}
*/

int main()
{
	//Initialization
	cout << "begin" << endl;
	token currentToken = { BEGIN, "test" };
	FILE* programFile = fopen("F:\\Users\\David\\Luria_EECE6083_CompilerProject\\testPgms\\correct\\logicals.src", "r");
	//Keep scanning until EOF
	//parse(currentToken, programFile, PROGRAM_MAIN);
	bool seen = false;
	while(currentToken.type != ENDFILE)
	{
		//Trap for 'no file found'
		if (programFile == NULL) 
		{
			cout << "No file found" << endl;
			break;
		}
		currentToken = ScanOneToken(programFile);
		if (currentToken.type == IF && !seen) {
			parse(currentToken, programFile, IF_STATEMENT);
			seen = true;
		}
		//cout << currentToken.type << "  " << currentToken.name << endl;
		//if (currentToken.type == UNKNOWN) {
		//	cout << "ERROR: UNKNOWN TOKEN: " + currentToken.name << endl;
		//}
	}
	//List symbols for debugging
	cout << endl << "Symbols (name: type): ";
	list<token>::iterator i;
	for (i = symbolTable.begin(); i != symbolTable.end(); i++)
	{
		cout << i->name << ": " << i->type << ", ";
	}
	cout << endl << endl << "end!";
	return 0;
	}


