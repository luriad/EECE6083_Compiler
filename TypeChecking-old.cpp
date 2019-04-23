/* EECE6083: TYPE CHECKING
 * Programmer: David Luria
 * Date updated: 3/22/2019
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
	LESSTHAN = '<', GREATERTHAN = '>', SINGLEQUOTE = '\'', DOUBLEQUOTE = '"', PERIOD = '.', ASSIGN = 257, EQUAL, LESSTHANEQUAL, GREATERTHANEQUAL, NOTEQUAL, RETURN, WHILE, IF, THEN, ELSE, FOR, PROGRAM, BEGIN, END, IN, OUT, INOUT, GLOBAL,
	IS, TRUE, FALSE, PROCEDURE, INTEGER, FLOAT, CHAR, STRING, BOOL, IDENTIFIER, INTVAL, FLOATVAL, STRINGVAL, BOOLVAL, UNKNOWN, ENDFILE
};

//Token structure
struct token {
	tokenType type;
	tokenType identifierType;
	string name;
};

//Symbol Table
list<token> symbolTable = { {RETURN, "return"}, {WHILE, "while"} , {IF, "if"} , {THEN, "then"} , {ELSE, "else"} , {FOR, "for"} , {PROGRAM, "program"} , {BEGIN, "begin"} ,
{END, "end"} , {IN, "in"} , {OUT, "out"} , {INOUT, "inout"} , {GLOBAL, "global"} , {IS, "is"} , {TRUE, "true"} , {FALSE, "false"} , {PROCEDURE, "procedure"} , {INTEGER, "integer"}, {FLOAT, "float"}, {CHAR, "char"}, {STRING, "string"}, {BOOL, "bool"} };

//Nonterminals
enum nonTerminal {
	PROGRAM_MAIN, PROGRAM_HEADER, PROGRAM_BODY, DECLARATION, PROCEDURE_DECLARATION, PROCEDURE_HEADER, PARAMETER_LIST, PARAMETER, PROCEDURE_BODY, VARIABLE_DECLARATION, TYPE_DECLARATION, TYPE_MARK, BOUND, STATEMENT, PROCEDURE_CALL, 
	ASSIGNMENT_STATEMENT, DESTINATION, IF_STATEMENT, LOOP_STATEMENT, RETURN_STATEMENT, EXPRESSION, EXPRESSIONEXT, ARITHOP, ARITHOPEXT, RELATION, RELATIONEXT, TERM, TERMEXT, FACTOR, NAME, ARGUMENT_LIST
};

tokenType currentTypeCheck;

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
		while (currentChar == '/') {
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
				lineNumber++;
				while (isspace(currentChar)) {
					currentChar = getc(file);
				}
			}
			//See if multi-line comment
			else if (nextChar == '*')
			{
				int nestLevel = 1;
				//Skip until comment end (*/)
				while (nestLevel > 0)
				{
					currentChar = getc(file);
					if (currentChar == '\n') {
						lineNumber++;
					}
					else if (currentChar == '/') {
						currentChar = getc(file);
						if (currentChar == '*') {
							nestLevel++;
						}
					}
					else if (currentChar == '*') {
						currentChar = getc(file);
						if (currentChar == '/') {
							nestLevel--;
						}
					}
				}
				currentChar = getc(file);
				//Skip remaining whitespace
				while (isspace(currentChar)) {
					if (currentChar == '\n') {
						lineNumber++;
					}
					currentChar = getc(file);
				}
			}
			//Otherwise, put the nextChar back
			else
			{
				//ANYTIME nextchar is used, finish with ungetc if the next character is not needed
				ungetc(nextChar, file);
				break;
			}
		}
	}

	outToken.name.push_back(currentChar);

	switch (currentChar)
	{
		//Regular single character tokens
	case ';': case '(': case ')': case '+': case '*': case '/': case '[': case ']': case '{': case '}': case '|': case '&': case ',': case '%': case '.': case '-':
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
		while ((currentChar >= 'A' & currentChar <= 'Z') | (currentChar >= 'a' & currentChar <= 'z') | currentChar == '_' | (currentChar >= '0' & currentChar <= '9'))
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
		cout << "ERROR: Unknown token " << outToken.name << " at line " << lineNumber << endl;
	}
	return outToken;
}

//Unget Token
void UngetToken(string token, FILE *file) {
	for (std::string::reverse_iterator rit = token.rbegin(); rit!=token.rend(); rit++) {
		ungetc(*rit, file);
	}
}

//Parser Method
bool parse(token currentToken, FILE *file, nonTerminal nonTerminal) {
	bool isParsed = false;
	string expectedToken;
	token nextToken;
	//cout << "Parsing " << nonTerminal << " at line " << lineNumber <<endl;
	switch (nonTerminal) {

	//PROGRAM PARSE
	case PROGRAM_MAIN:
		if (!parse(currentToken, file, PROGRAM_HEADER)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, PROGRAM_BODY)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != PERIOD) {
			expectedToken = ".";
			break;
		}
		isParsed = true;
		expectedToken = "Program main";
		break;
	case PROGRAM_HEADER:
		if (currentToken.type != PROGRAM) {
			expectedToken = "program";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "identifier";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != IS) {
			expectedToken = "is";
			break;
		}
		isParsed = true;
		expectedToken = "Program header";
		break;
	case PROGRAM_BODY:
		while (currentToken.type != BEGIN & currentToken.type != ENDFILE) {
			if (!parse(currentToken, file, DECLARATION)) {
				break;
			}
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type != BEGIN) {
			expectedToken = "begin";
			break;
		}
		currentToken = ScanOneToken(file);
		while (currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, file, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type != END) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != PROGRAM) {
			expectedToken = "program";
			break;
		}
		isParsed = true;
		expectedToken = "Program body";
		break;

	//DECLARATION PARSE
	case DECLARATION:
		if (currentToken.type == GLOBAL) {
			currentToken = ScanOneToken(file);
		}
		if (parse(currentToken, file, PROCEDURE_DECLARATION)) {
			cout << "Procedure Declaration" << endl;
		}
		else if (parse(currentToken, file, VARIABLE_DECLARATION)) {
			cout << "Variable Declaration" << endl;
		}
		else {
			expectedToken = "Declaration";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != SEMICOLON) {
			expectedToken = ";";
			break;
		}
		isParsed = true;
		expectedToken = "Declaration";
		break;

	//PROCEDURE DECLARATION PARSE
	case PROCEDURE_DECLARATION:
		if (!parse(currentToken, file, PROCEDURE_HEADER)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, PROCEDURE_BODY)) {
			break;
		}
		isParsed = true;
		expectedToken = "Procedure declaration";
		break;
	case PROCEDURE_HEADER:
		if (currentToken.type != PROCEDURE) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != LPAREN) {
			expectedToken = "(";
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, PARAMETER_LIST)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != RPAREN) {
			expectedToken = ")";
			break;
		}
		isParsed = true;
		expectedToken = "Procedure header";
		break;
	case PROCEDURE_BODY:
		while (currentToken.type != BEGIN & currentToken.type != ENDFILE) {
			if (!parse(currentToken, file, DECLARATION)) {
				break;
			}
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type == ENDFILE) {
			expectedToken = "begin";
			break;
		}
		currentToken = ScanOneToken(file);
		while (currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, file, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type == ENDFILE) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != PROCEDURE) {
			expectedToken = "procedure";
			break;
		}
		isParsed = true;
		expectedToken = "Procedure body";
		break;

	//PARAMETER LIST PARSE
	case PARAMETER_LIST:
		if (!parse(currentToken, file, PARAMETER)) {
			break;
		}
		nextToken = ScanOneToken(file);
		if (nextToken.type == COMMA) {
			currentToken = ScanOneToken(file);
			if (!parse(currentToken, file, PARAMETER_LIST)) {
				expectedToken = "More parameters";
				break;
			}
		}
		else {
			UngetToken(nextToken.name, file);
		}
		isParsed = true;
		expectedToken = "Parameter List";
		break;
	case PARAMETER:
		if (!parse(currentToken, file, VARIABLE_DECLARATION)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type == IN) {
			cout << "In parameter" << endl;
		}
		else if (currentToken.type == OUT) {
			cout << "Out parameter" << endl;
		}
		else if (currentToken.type == INOUT) {
			cout << "Inout parameter" << endl;
		}
		else {
			expectedToken = "In/out";
			break;
		}
		isParsed = true;
		expectedToken = "Parameter";
		break;

	//VARIABLE DECLARATION PARSE
	case VARIABLE_DECLARATION:
		if (!parse(currentToken, file, TYPE_MARK)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
			break;
		}
		currentToken.identifierType = currentTypeCheck;
		nextToken = ScanOneToken(file);
		if (nextToken.type == LBRACKET) {
			currentToken = ScanOneToken(file);
			if (!parse(currentToken, file, BOUND)) {
				break;
			}
			currentToken = ScanOneToken(file);
			if (currentToken.type != RBRACKET) {
				expectedToken = "]";
				break;
			}
		}
		else {
			UngetToken(nextToken.name, file);
		}
		isParsed = true;
		expectedToken = "Variable Declaration";
		break;

	//TYPE MARK PARSE
	case TYPE_MARK:
		if (currentToken.type == INTEGER) {
			cout << "Integer type mark" << endl;
			currentTypeCheck = INTVAL;
		}
		else if (currentToken.type == FLOAT) {
			cout << "Float type mark" << endl;
			currentTypeCheck = FLOATVAL;
		}
		else if (currentToken.type == STRING) {
			cout << "String type mark" << endl;
			currentTypeCheck = STRINGVAL;
		}
		else if (currentToken.type == BOOL) {
			cout << "Bool type mark" << endl;
			currentTypeCheck = BOOLVAL;
		}
		else if (currentToken.type == CHAR) {
			cout << "Char type mark" << endl;
			currentTypeCheck = STRINGVAL;
		}
		else {
			expectedToken = "Type mark";
			break;
		}
		isParsed = true;
		expectedToken = "Type mark";
		break;

	//BOUND PARSE
	case BOUND:
		if (!parse(currentToken, file, EXPRESSION)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != COLON) {
			expectedToken = ":";
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, EXPRESSION)) {
			break;
		}
		isParsed = true;
		expectedToken = "Bound";
		break;

	//STATEMENT PARSE
	case STATEMENT:
		if (currentToken.type == IDENTIFIER) {
			currentToken = ScanOneToken(file);
			parse(currentToken, file, NAME);
			currentToken = ScanOneToken(file);
			if (parse(currentToken, file, ASSIGNMENT_STATEMENT)) {
				cout << "Assignment Statement" << endl;
			}
			else if (parse(currentToken, file, PROCEDURE_CALL)) {
				cout << "Expression Statement" << endl;
			}
			else {
				break;
			}
		}
		else if (parse(currentToken, file, IF_STATEMENT)) {
			cout << "If Statement" << endl;
		}
		else if (parse(currentToken, file, LOOP_STATEMENT)) {
			cout << "Loop Statement" << endl;
		}
		else if (parse(currentToken, file, RETURN_STATEMENT)) {
			cout << "Return Statement" << endl;
		}
		else {
			expectedToken = "Statement";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != SEMICOLON) {
			expectedToken = ";";
			break;
		}
		isParsed = true;
		expectedToken = "Statement";
		break;

	//PROCEDURE CALL PARSE
	case PROCEDURE_CALL:
		if (currentToken.type != LPAREN) {
			break;
		}
		currentToken = ScanOneToken(file);
		parse(currentToken, file, ARGUMENT_LIST);
		currentToken = ScanOneToken(file);
		if (currentToken.type != RPAREN) {
			expectedToken = ")";
			break;
		}
		isParsed = true;
		expectedToken = "Procedure call";
		break;

	//ASSIGNMENT STATEMENT
	case ASSIGNMENT_STATEMENT:
		if (currentToken.type != ASSIGN) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, EXPRESSION)) {
			break;
		}
		isParsed = true;
		expectedToken = "Assignment Statement";
		break;

	//DESTINATION
	case DESTINATION:
		if (currentToken.type == LBRACKET) {
			currentToken = ScanOneToken(file);
			if (!parse(currentToken, file, EXPRESSION)) {
				break;
			}
			currentToken = ScanOneToken(file);
			if (currentToken.type != RBRACKET) {
				expectedToken = "]";
				break;
			}
		}
		else {
			UngetToken(currentToken.name, file);
		}
		isParsed = true;
		expectedToken = "Destination";
		break;

	//IF PARSE
	case IF_STATEMENT:
		if (currentToken.type != IF) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != LPAREN) {
			expectedToken = "'('";
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, EXPRESSION)) {
			break;
		}
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
		while (currentToken.type != ELSE & currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, file, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type == ELSE) {
			currentToken = ScanOneToken(file);
			while (currentToken.type != END) {
				if (!parse(currentToken, file, STATEMENT)) {
					break;
				}
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
		expectedToken = "If statement";
		break;

	//LOOP STATEMENT PARSE
	case LOOP_STATEMENT:
		if (currentToken.type != FOR) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != LPAREN) {
			expectedToken = "(";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, ASSIGNMENT_STATEMENT)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != SEMICOLON) {
			expectedToken = ";";
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, EXPRESSION)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != RPAREN) {
			expectedToken = ")";
			break;
		}
		currentToken = ScanOneToken(file);
		while (currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, file, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken(file);
		}
		if (currentToken.type != END) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken(file);
		if (currentToken.type != FOR) {
			expectedToken = "for";
			break;
		}
		isParsed = true;
		expectedToken = "Loop statement";
		break;

	//RETURN STATEMENT PARSE
	case RETURN_STATEMENT:
		if (currentToken.type != RETURN) {
			break;
		}
		isParsed = true;
		expectedToken = "Return statement";
		break;

	//EXPRESSION PARSE
	case EXPRESSION:
		if (currentToken.type == NOT) {
			currentToken = ScanOneToken(file);
		}
		if (!parse(currentToken, file, ARITHOP)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, EXPRESSIONEXT)) {
			break;
		}
		isParsed = true;
		expectedToken = "Expression";
		break;
	case EXPRESSIONEXT:
		if (currentToken.type == AND) {
			cout << "and expression" << endl;
		}
		else if (currentToken.type == OR) {
			cout << "or expression" << endl;
		}
		else {
		    UngetToken(currentToken.name, file);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, ARITHOP)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, EXPRESSIONEXT)) {
			break;
		}
		isParsed = true;
		break;

	//ARITHOP PARSE
	case ARITHOP:
		if (!parse(currentToken, file, RELATION)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, ARITHOPEXT)) {
			break;
		}
		isParsed = true;
		expectedToken = "Arithmetic operation";
		break;

	case ARITHOPEXT:
		if (currentToken.type == PLUS) {
			cout << "+ arithOp" << endl;
		}
		else if (currentToken.type == MINUS) {
			cout << "- arithOp" << endl;
		}
		else {
			UngetToken(currentToken.name, file);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, RELATION)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, ARITHOPEXT)) {
			break;
		}
		isParsed = true;
		break;

	//RELATION PARSE
	case RELATION:
		if (!parse(currentToken, file, TERM)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, RELATIONEXT)) {
			break;
		}
		isParsed = true;
		expectedToken = "Relation";
		break;
	case RELATIONEXT:
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
			UngetToken(currentToken.name, file);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, TERM)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, RELATIONEXT)) {
			break;
		}
		isParsed = true;
		break;

	//TERM PARSE
	case TERM:
		if (!parse(currentToken, file, FACTOR)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, TERMEXT)) {
			break;
		}
		isParsed = true;
		expectedToken = "Term";
		break;
	case TERMEXT:
		if (currentToken.type == STAR) {
			cout << "* term" << endl;
		}
		else if (currentToken.type == SLASH) {
			cout << "/ term" << endl;
		}
		else {
			UngetToken(currentToken.name, file);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, FACTOR)) {
			break;
		}
		currentToken = ScanOneToken(file);
		if (!parse(currentToken, file, TERMEXT)) {
			break;
		}
		isParsed = true;
		break;

	//FACTOR PARSE
	case FACTOR:
		if (currentToken.type == LPAREN) {
			currentToken = ScanOneToken(file);
			if (!parse(currentToken, file, EXPRESSION)) {
				break;
			}
			currentToken = ScanOneToken(file);
			if (currentToken.type != RPAREN) {
				expectedToken = "')'";
				break;
			}
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		if (currentToken.type == IDENTIFIER) {
			currentToken = ScanOneToken(file);
			if (parse(currentToken, file, PROCEDURE_CALL)) {
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
			else if (parse(currentToken, file, NAME)) {
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
		}
		else if (currentToken.type == MINUS) {
			currentToken = ScanOneToken(file);
			if (currentToken.type == INTVAL | currentToken.type == FLOATVAL) {
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
			else if (currentToken.type == IDENTIFIER) {
				parse(currentToken, file, NAME);
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
			else {
				expectedToken = "number";
				break;
			}
		}
		else if (currentToken.type == INTVAL | currentToken.type == FLOATVAL) {
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		else if (currentToken.type == STRINGVAL) {
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		else if (currentToken.type == TRUE) {
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		else if (currentToken.type == FALSE) {
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		break;

	//NAME PARSE
	case NAME:
		if (currentToken.type == LBRACKET) {
			currentToken = ScanOneToken(file);
			if (!parse(currentToken, file, EXPRESSION)) {
				expectedToken = "Expression";
				break;
			}
			currentToken = ScanOneToken(file);
			if (currentToken.type != RBRACKET) {
				expectedToken = "]";
				break;
			}
		}
		else {
			UngetToken(currentToken.name, file);
		}
		isParsed = true;
		expectedToken = "Name";
		break;

	//ARGUMENT LIST PARSE
	case ARGUMENT_LIST:
		if (!parse(currentToken, file, EXPRESSION)) {
			break;
		}
		nextToken = ScanOneToken(file);
		if (nextToken.type == COMMA) {
			currentToken = ScanOneToken(file);
			if (!parse(currentToken, file, ARGUMENT_LIST)) {
				break;
			}
		}
		else {
			UngetToken(nextToken.name, file);
		}
		isParsed = true;
		expectedToken = "Argument list";
		break;

	//NO PARSE FOUND
	default:
		cout << "Error: No parse for " << nonTerminal << " at line " << lineNumber << endl;
		expectedToken = "parsable nonterminal";
		break;
	}
	/*if (isParsed & expectedToken != "") {
		cout << expectedToken << " " << currentToken.name << " parsed successfully at line " << lineNumber << endl;
	}*/
	if (expectedToken != "") {
		cout << "BLURGHARG *Dies* (Expected " << expectedToken << " got " << currentToken.name << ")" << " at line " << lineNumber << endl;
	}
	return isParsed;
}

//Type Checking
void CheckType(tokenType correctType, tokenType tokenToCheck) {
	if ((correctType == INTVAL | correctType == FLOATVAL) & (tokenToCheck == INTVAL | tokenToCheck == FLOATVAL)) {
		return; //Good
	}
	else if (correctType == tokenToCheck) {
		return; //Good
	}
	else {
		cout << "ERROR: Unexpected " << tokenToCheck << " at line " << lineNumber << endl;
	}
}

int main()
{
	//Initialization
	cout << "begin" << endl;
	token currentToken = { BEGIN, "test" };
	FILE* programFile = fopen("F:\\Users\\David\\Luria_EECE6083_CompilerProject\\testPgms\\correct\\source.src", "r");

	//Parse program
	currentToken = ScanOneToken(programFile);
	parse(currentToken, programFile, PROGRAM_MAIN);
	cout << "**End of program**" << endl;

	//List symbols for debugging
	cout << endl << "Symbols (name: type): ";
	list<token>::iterator i;
	for (i = symbolTable.begin(); i != symbolTable.end(); i++)
	{
		cout << i->name << ": " << i->type << ", ";
	}
	cout << endl << endl << "end!" << endl;
	return 0;
}


