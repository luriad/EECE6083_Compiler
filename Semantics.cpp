/*EECE6083: SEMANTICS
 * Programmer: David Luria
 * Date updated: 4/23/2019
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <list>
#include <algorithm>
#include <unordered_map>
using namespace std;

//File to compile
FILE* file = fopen("F:\\Users\\David\\Luria_EECE6083_CompilerProject\\testPgms\\correct\\test1.src", "r");

//Line number
int lineNumber = 1;

//Current Scope (0 = global)
int scope = 0;

//Token Types
enum tokenType {
	SEMICOLON = ';', LPAREN = '(', RPAREN = ')', COLON = ':', PLUS = '+', MINUS = '-', STAR = '*', SLASH = '/', LBRACKET = '[', RBRACKET = ']', LBRACE = '{', RBRACE = '}', OR = '|', AND = '&', NOT = '!', COMMA = ',', MODULUS = '%',
	LESSTHAN = '<', GREATERTHAN = '>', SINGLEQUOTE = '\'', DOUBLEQUOTE = '"', PERIOD = '.', ASSIGN = 257, EQUAL, LESSTHANEQUAL, GREATERTHANEQUAL, NOTEQUAL, RETURN, WHILE, IF, THEN, ELSE, FOR, PROGRAM, BEGIN, END, IN, OUT, INOUT, GLOBAL,
	IS, TRUE, FALSE, PROCEDURE, INTEGER, FLOAT, CHAR, STRING, BOOL, IDENTIFIER, INTVAL, FLOATVAL, STRINGVAL, UNKNOWN, ENDFILE
};

//Token structure
struct token {
	tokenType type;
	string name;
};

//Symbol table type is unordered_map, which stores values as a hash table. Here, the key is the token name (string), and the value stored is the token itself
typedef std::unordered_map<std::string, token> symbol_table;

//Symbol Table List (separated by scope)
std::list<symbol_table> symbolTables = {};

//Global Symbol Table (Hash Table)
symbol_table globalSymbolTable = { {"return", {RETURN,"return"}}, {"while",{WHILE,"while"}} , {"if",{IF,"if"}} , {"then",{THEN,"then"}} , {"else",{ELSE,"else"}} , {"for",{FOR,"for"}} , {"program",{PROGRAM,"program"}} , {"begin",{BEGIN,"begin"}},
{"end", {END,"end"}}, {"in",{IN,"in"}}, {"out",{OUT,"out"}}, {"inout",{INOUT,"inout"}}, {"global",{GLOBAL,"global"}}, {"is",{IS,"is"}}, {"true",{TRUE,"true"}}, {"false",{FALSE,"false"}}, {"procedure",{PROCEDURE,"procedure"}}, {"integer",{INTEGER,"integer"}}, 
{"float",{FLOAT,"float"}}, {"char",{CHAR,"char"}}, {"string",{STRING,"string"} }, {"bool",{BOOL,"bool"}} };

//Nonterminals
enum nonTerminal {
	PROGRAM_MAIN, PROGRAM_HEADER, PROGRAM_BODY, DECLARATION, PROCEDURE_DECLARATION, PROCEDURE_HEADER, PARAMETER_LIST, PARAMETER, PROCEDURE_BODY, VARIABLE_DECLARATION, TYPE_DECLARATION, TYPE_MARK, BOUND, STATEMENT, PROCEDURE_CALL, 
	ASSIGNMENT_STATEMENT, DESTINATION, IF_STATEMENT, LOOP_STATEMENT, RETURN_STATEMENT, EXPRESSION, EXPRESSIONEXT, ARITHOP, ARITHOPEXT, RELATION, RELATIONEXT, TERM, TERMEXT, FACTOR, NAME, ARGUMENT_LIST
};

//Scanner Method
token ScanOneToken()
{
	//Load in the character
	token outToken;
	string tokenName = "";
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

	tokenName.push_back(currentChar);

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
			tokenName.push_back(nextChar);
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
			tokenName.push_back(nextChar);
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
			tokenName.push_back(nextChar);
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
			tokenName.push_back(nextChar);
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
			tokenName.push_back(nextChar);
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
			tokenName.push_back(currentChar);
		} while (currentChar != '\'');
		outToken.type = STRINGVAL;
		break;
		//Double quote string case
	case '"':
		//Keep scanning until next double quote reached
		do
		{
			currentChar = getc(file);
			tokenName.push_back(currentChar);
		} while (currentChar != '"');
		outToken.type = STRINGVAL;
		break;
	case EOF:
		outToken.type = ENDFILE;
		tokenName = "EOF";
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
			tokenName.push_back(currentChar);
		}
		//Put back last character
		ungetc(currentChar, file);
		tokenName.pop_back();
		//Convert to lowercase (compiler is case insensitive)
		std::transform(tokenName.begin(), tokenName.end(), tokenName.begin(), ::tolower);
		//Check if token is in symbol tables. If not, add it as an identifier
		bool inASymTable = false;
		for (symbol_table symbolTable : symbolTables)
		{
			if (symbolTable.find(tokenName) != symbolTable.end())
			{
				outToken = symbolTable[tokenName];
				inASymTable = true;
				break;
			}
		}
		if (!inASymTable)
		{
			outToken.type = IDENTIFIER;
			outToken.name = tokenName;
			//If in global scope, add to global symbol table (bottom of stack)
			if (scope == 0)
			{
				symbolTables.front()[tokenName] = outToken;
			}
			//If not in global scope, add to current scope symbol table (top of stack)
			else 
			{
				symbolTables.back()[tokenName] = outToken;
			}
		}
	}
	//Number case
	else if (currentChar >= '0' & currentChar <= '9')
	{
		//Keep scanning while numbers are read
		while (currentChar >= '0' & currentChar <= '9')
		{
			currentChar = getc(file);
			tokenName.push_back(currentChar);
		}
		//If a decimal is encountered, recognize as float (NOTE: ADD double TYPE)
		if (currentChar == '.')
		{
			do
			{
				currentChar = getc(file);
				tokenName.push_back(currentChar);
			} while (currentChar >= '0' & currentChar <= '9');
			outToken.type = FLOATVAL;
		}
		//If no decimal encountered, recognize as integer value
		else
		{
			outToken.type = INTVAL;
		}
		ungetc(currentChar, file);
		tokenName.pop_back();
	}
	outToken.name = tokenName;
	return outToken;
}

//Unget Token
void UngetToken(string token) {
	for (std::string::reverse_iterator rit = token.rbegin(); rit!=token.rend(); rit++) {
		ungetc(*rit, file);
	}
}

//Parser Method
bool parse(token currentToken, nonTerminal nonTerminal) {
	bool isParsed = false;
	string expectedToken;
	token nextToken;
	cout << "Parsing " << nonTerminal << " at line " << lineNumber <<endl;
	switch (nonTerminal) {

	//PROGRAM PARSE
	case PROGRAM_MAIN:
		if (!parse(currentToken, PROGRAM_HEADER)) {
			break;
		}
		scope++;
		symbolTables.push_back({});
		currentToken = ScanOneToken();
		if (!parse(currentToken, PROGRAM_BODY)) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != PERIOD) {
			expectedToken = ".";
			break;
		}
		isParsed = true;
		expectedToken = "Program main";
		symbolTables.pop_back();
		scope--;
		break;
	case PROGRAM_HEADER:
		if (currentToken.type != PROGRAM) {
			expectedToken = "program";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "identifier";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != IS) {
			expectedToken = "is";
			break;
		}
		isParsed = true;
		expectedToken = "Program header";
		break;
	case PROGRAM_BODY:
		while (currentToken.type != BEGIN & currentToken.type != ENDFILE) {
			if (!parse(currentToken, DECLARATION)) {
				break;
			}
			currentToken = ScanOneToken();
		}
		if (currentToken.type != BEGIN) {
			expectedToken = "begin";
			break;
		}
		currentToken = ScanOneToken();
		while (currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken();
		}
		if (currentToken.type != END) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken();
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
			scope = 0;
			currentToken = ScanOneToken();
		}
		if (parse(currentToken, PROCEDURE_DECLARATION)) {
			cout << "Procedure Declaration" << endl;
		}
		else if (parse(currentToken, VARIABLE_DECLARATION)) {
			cout << "Variable Declaration" << endl;
		}
		else {
			expectedToken = "Declaration";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != SEMICOLON) {
			expectedToken = ";";
			break;
		}
		if (scope == 0) {
			scope = symbolTables.size() - 1;
		}
		isParsed = true;
		expectedToken = "Declaration";
		break;

	//PROCEDURE DECLARATION PARSE
	case PROCEDURE_DECLARATION:
		if (!parse(currentToken, PROCEDURE_HEADER)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, PROCEDURE_BODY)) {
			break;
		}
		isParsed = true;
		expectedToken = "Procedure declaration";
		scope--;
		symbolTables.pop_back();
		break;
	case PROCEDURE_HEADER:
		if (currentToken.type != PROCEDURE) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
			break;
		}
		scope++;
		symbolTables.push_back({});
		currentToken = ScanOneToken();
		if (currentToken.type != LPAREN) {
			expectedToken = "(";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != RPAREN) {
			parse(currentToken, PARAMETER_LIST);
			currentToken = ScanOneToken();
			if (currentToken.type != RPAREN) {
				expectedToken = ")";
				break;
			}
		}
		/*if (!parse(currentToken, PARAMETER_LIST)) {
			break;
		}*/
		isParsed = true;
		expectedToken = "Procedure header";
		break;
	case PROCEDURE_BODY:
		while (currentToken.type != BEGIN & currentToken.type != ENDFILE) {
			if (!parse(currentToken, DECLARATION)) {
				break;
			}
			currentToken = ScanOneToken();
		}
		if (currentToken.type == ENDFILE) {
			expectedToken = "begin";
			break;
		}
		currentToken = ScanOneToken();
		while (currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken();
		}
		if (currentToken.type == ENDFILE) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != PROCEDURE) {
			expectedToken = "procedure";
			break;
		}
		isParsed = true;
		expectedToken = "Procedure body";
		break;

	//PARAMETER LIST PARSE
	case PARAMETER_LIST:
		if (!parse(currentToken, PARAMETER)) {
			break;
		}
		nextToken = ScanOneToken();
		if (nextToken.type == COMMA) {
			currentToken = ScanOneToken();
			if (!parse(currentToken, PARAMETER_LIST)) {
				expectedToken = "More parameters";
				break;
			}
		}
		else {
			UngetToken(nextToken.name);
		}
		isParsed = true;
		expectedToken = "Parameter List";
		break;
	case PARAMETER:
		if (!parse(currentToken, VARIABLE_DECLARATION)) {
			break;
		}
		currentToken = ScanOneToken();
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
		if (!parse(currentToken, TYPE_MARK)) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
			break;
		}
		nextToken = ScanOneToken();
		if (nextToken.type == LBRACKET) {
			currentToken = ScanOneToken();
			if (!parse(currentToken, BOUND)) {
				break;
			}
			currentToken = ScanOneToken();
			if (currentToken.type != RBRACKET) {
				expectedToken = "]";
				break;
			}
		}
		else {
			UngetToken(nextToken.name);
		}
		isParsed = true;
		expectedToken = "Variable Declaration";
		break;

	//TYPE MARK PARSE
	case TYPE_MARK:
		if (currentToken.type == INTEGER) {
			cout << "Integer type mark" << endl;
		}
		else if (currentToken.type == FLOAT) {
			cout << "Float type mark" << endl;
		}
		else if (currentToken.type == STRING) {
			cout << "String type mark" << endl;
		}
		else if (currentToken.type == BOOL) {
			cout << "Bool type mark" << endl;
		}
		else if (currentToken.type == CHAR) {
			cout << "Char type mark" << endl;
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
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != COLON) {
			expectedToken = ":";
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		isParsed = true;
		expectedToken = "Bound";
		break;

	//STATEMENT PARSE
	case STATEMENT:
		if (currentToken.type == IDENTIFIER) {
			currentToken = ScanOneToken();
			parse(currentToken, NAME);
			currentToken = ScanOneToken();
			if (parse(currentToken, ASSIGNMENT_STATEMENT)) {
				cout << "Assignment Statement" << endl;
			}
			else if (parse(currentToken, PROCEDURE_CALL)) {
				cout << "Expression Statement" << endl;
			}
			else {
				break;
			}
		}
		else if (parse(currentToken, IF_STATEMENT)) {
			cout << "If Statement" << endl;
		}
		else if (parse(currentToken, LOOP_STATEMENT)) {
			cout << "Loop Statement" << endl;
		}
		else if (parse(currentToken, RETURN_STATEMENT)) {
			cout << "Return Statement" << endl;
		}
		else {
			expectedToken = "Statement";
			break;
		}
		currentToken = ScanOneToken();
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
		currentToken = ScanOneToken();
		if (currentToken.type != RPAREN) {
			parse(currentToken, ARGUMENT_LIST);
			currentToken = ScanOneToken();
			if (currentToken.type != RPAREN) {
				expectedToken = ")";
				break;
			}
		}
		isParsed = true;
		expectedToken = "Procedure call";
		break;

	//ASSIGNMENT STATEMENT
	case ASSIGNMENT_STATEMENT:
		if (currentToken.type != ASSIGN) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		isParsed = true;
		expectedToken = "Assignment Statement";
		break;

	//DESTINATION
	case DESTINATION:
		if (currentToken.type == LBRACKET) {
			currentToken = ScanOneToken();
			if (!parse(currentToken, EXPRESSION)) {
				break;
			}
			currentToken = ScanOneToken();
			if (currentToken.type != RBRACKET) {
				expectedToken = "]";
				break;
			}
		}
		else {
			UngetToken(currentToken.name);
		}
		isParsed = true;
		expectedToken = "Destination";
		break;

	//IF PARSE
	case IF_STATEMENT:
		if (currentToken.type != IF) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != LPAREN) {
			expectedToken = "'('";
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != RPAREN) {
			expectedToken = "')'";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != THEN) {
			expectedToken = "then";
			break;
		}
		currentToken = ScanOneToken();
		while (currentToken.type != ELSE & currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken();
		}
		if (currentToken.type == ELSE) {
			currentToken = ScanOneToken();
			while (currentToken.type != END) {
				if (!parse(currentToken, STATEMENT)) {
					break;
				}
				currentToken = ScanOneToken();
			}
		}
		if (currentToken.type != END) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken();
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
		currentToken = ScanOneToken();
		if (currentToken.type != LPAREN) {
			expectedToken = "(";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, ASSIGNMENT_STATEMENT)) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != SEMICOLON) {
			expectedToken = ";";
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type != RPAREN) {
			expectedToken = ")";
			break;
		}
		currentToken = ScanOneToken();
		while (currentToken.type != END & currentToken.type != ENDFILE) {
			if (!parse(currentToken, STATEMENT)) {
				break;
			}
			currentToken = ScanOneToken();
		}
		if (currentToken.type != END) {
			expectedToken = "end";
			break;
		}
		currentToken = ScanOneToken();
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
			currentToken = ScanOneToken();
		}
		if (!parse(currentToken, ARITHOP)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSIONEXT)) {
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
		    UngetToken(currentToken.name);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, ARITHOP)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSIONEXT)) {
			break;
		}
		isParsed = true;
		break;

	//ARITHOP PARSE
	case ARITHOP:
		if (!parse(currentToken, RELATION)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, ARITHOPEXT)) {
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
			UngetToken(currentToken.name);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, RELATION)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, ARITHOPEXT)) {
			break;
		}
		isParsed = true;
		break;

	//RELATION PARSE
	case RELATION:
		if (!parse(currentToken, TERM)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, RELATIONEXT)) {
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
			UngetToken(currentToken.name);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, TERM)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, RELATIONEXT)) {
			break;
		}
		isParsed = true;
		break;

	//TERM PARSE
	case TERM:
		if (!parse(currentToken, FACTOR)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, TERMEXT)) {
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
			UngetToken(currentToken.name);
			isParsed = true;
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, FACTOR)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, TERMEXT)) {
			break;
		}
		isParsed = true;
		break;

	//FACTOR PARSE
	case FACTOR:
		if (currentToken.type == LPAREN) {
			currentToken = ScanOneToken();
			if (!parse(currentToken, EXPRESSION)) {
				break;
			}
			currentToken = ScanOneToken();
			if (currentToken.type != RPAREN) {
				expectedToken = "')'";
				break;
			}
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		if (currentToken.type == IDENTIFIER) {
			currentToken = ScanOneToken();
			if (parse(currentToken, PROCEDURE_CALL)) {
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
			else if (parse(currentToken, NAME)) {
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
		}
		else if (currentToken.type == MINUS) {
			currentToken = ScanOneToken();
			if (currentToken.type == INTVAL | currentToken.type == FLOATVAL) {
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
			else if (currentToken.type == IDENTIFIER) {
				parse(currentToken, NAME);
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
			currentToken = ScanOneToken();
			if (!parse(currentToken, EXPRESSION)) {
				expectedToken = "Expression";
				break;
			}
			currentToken = ScanOneToken();
			if (currentToken.type != RBRACKET) {
				expectedToken = "]";
				break;
			}
		}
		else {
			UngetToken(currentToken.name);
		}
		isParsed = true;
		expectedToken = "Name";
		break;

	//ARGUMENT LIST PARSE
	case ARGUMENT_LIST:
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		nextToken = ScanOneToken();
		if (nextToken.type == COMMA) {
			currentToken = ScanOneToken();
			if (!parse(currentToken, ARGUMENT_LIST)) {
				break;
			}
		}
		else {
			UngetToken(nextToken.name);
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
	if (isParsed & expectedToken != "") {
		cout << expectedToken << " " << currentToken.name << " parsed successfully at line " << lineNumber << endl;
	}
	else if (expectedToken != "") {
		cout << "BLURGHARG *Dies* (Expected " << expectedToken << " got " << currentToken.name << ")" << " at line " << lineNumber << endl;
	}
	return isParsed;
}

//Main
int main()
{
	//Initialization
	cout << "begin" << endl;
	symbolTables.push_back(globalSymbolTable);
	token currentToken = { BEGIN, "test" };
	//Parse program
	currentToken = ScanOneToken();
	parse(currentToken, PROGRAM_MAIN);
	cout << "**End of program**" << endl;

	//List symbols for debugging
	cout << endl << "Global Symbols (name: type): " << endl;
	for (symbol_table symbolTable : symbolTables) 
	{
		symbol_table::iterator itr;
		for (itr = symbolTable.begin(); itr != symbolTable.end(); itr++)
		{
			cout << itr->second.name << ": " << itr->second.type << ",";
		}
	}
	cout << endl << endl << "end!" << endl;
	return 0;
	}


