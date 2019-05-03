/*EECE6083: SEMANTICS
 * Programmer: David Luria
 * Date updated: 5/3/2019
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <list>
#include <algorithm>
#include <unordered_map>
using namespace std;

//File to compile
FILE* file = fopen("F:\\Users\\David\\Luria_EECE6083_CompilerProject\\testPgms\\correct\\source.src", "r");

//Line number
int lineNumber = 1;

//Token Types
enum tokenType {
	SEMICOLON = ';', LPAREN = '(', RPAREN = ')', COLON = ':', PLUS = '+', MINUS = '-', STAR = '*', SLASH = '/', LBRACKET = '[', RBRACKET = ']', LBRACE = '{', RBRACE = '}', OR = '|', AND = '&', NOT = '!', COMMA = ',', MODULUS = '%',
	LESSTHAN = '<', GREATERTHAN = '>', SINGLEQUOTE = '\'', DOUBLEQUOTE = '"', PERIOD = '.', ASSIGN = 257, EQUAL, LESSTHANEQUAL, GREATERTHANEQUAL, NOTEQUAL, RETURN, WHILE, IF, THEN, ELSE, FOR, PROGRAM, BEGIN, END, IN, OUT, INOUT, GLOBAL,
	IS, TRUE, FALSE, BOOLVAL, PROCEDURE, INTEGER, FLOAT, CHAR, STRING, BOOL, IDENTIFIER, INTVAL, FLOATVAL, STRINGVAL, UNKNOWN, ENDFILE
};

//Token structure
struct token {
	tokenType type;
	string name;
	bool declared = false;
	tokenType variableType = UNKNOWN;
	tokenType procedureOutType = UNKNOWN;
	int lowerBound = 0;
	int upperBound = 0;
	list<tokenType> parameterTypes;
};

//Symbol table type is unordered_map, which stores values as a hash table. Here, the key is the token name (string), and the value stored is the token itself
typedef std::unordered_map<std::string, token> symbol_table;

//Symbol Table List (separated by scope)
std::list<symbol_table> symbolTables = {};

//Global Symbol Table (Hash Table)
symbol_table globalSymbolTable = { {"return", {RETURN,"return"}}, {"while",{WHILE,"while"}} , {"if",{IF,"if"}} , {"then",{THEN,"then"}} , {"else",{ELSE,"else"}} , {"for",{FOR,"for"}} , {"program",{PROGRAM,"program"}} , {"begin",{BEGIN,"begin"}},
{"end", {END,"end"}}, {"in",{IN,"in"}}, {"out",{OUT,"out"}}, {"inout",{INOUT,"inout"}}, {"global",{GLOBAL,"global"}}, {"is",{IS,"is"}}, {"true",{TRUE,"true"}}, {"false",{FALSE,"false"}}, {"procedure",{PROCEDURE,"procedure"}}, {"integer",{INTEGER,"integer"}}, 
{"float",{FLOAT,"float"}}, {"char",{CHAR,"char"}}, {"string",{STRING,"string"} }, {"bool",{BOOL,"bool"}}, {"getbool",{IDENTIFIER,"getbool",true,PROCEDURE,BOOLVAL,0,0,{BOOLVAL}}}, {"getinteger",{IDENTIFIER,"getinteger",true,PROCEDURE,INTVAL,0,0,{INTVAL}}},
{"gefloat",{IDENTIFIER,"getfloat",true,PROCEDURE,FLOATVAL,0,0,{FLOATVAL}}},{"getstring",{IDENTIFIER,"getstring",true,PROCEDURE,STRINGVAL,0,0,{STRINGVAL}}}, {"putbool",{IDENTIFIER,"putbool",true,PROCEDURE,UNKNOWN,0,0,{BOOLVAL}}}, 
{"putinteger",{IDENTIFIER,"putinteger",true,PROCEDURE,UNKNOWN,0,0,{INTVAL}}}, {"putfloat",{IDENTIFIER,"putfloat",true,PROCEDURE,UNKNOWN,0,0,{FLOATVAL}}}, {"putstring",{IDENTIFIER,"putstring",true,PROCEDURE,UNKNOWN,0,0,{STRINGVAL}}},
{"putchar",{IDENTIFIER,"putchar",true,PROCEDURE,UNKNOWN,0,0,{STRINGVAL}}}, {"sqrt",{IDENTIFIER,"sqrt",true,PROCEDURE,FLOATVAL,0,0,{FLOATVAL}}}};

//Nonterminals
enum nonTerminal {
	PROGRAM_MAIN, PROGRAM_HEADER, PROGRAM_BODY, DECLARATION, PROCEDURE_DECLARATION, PROCEDURE_HEADER, PARAMETER_LIST, PARAMETER, PROCEDURE_BODY, VARIABLE_DECLARATION, TYPE_DECLARATION, TYPE_MARK, BOUND, STATEMENT, PROCEDURE_CALL, 
	ASSIGNMENT_STATEMENT, DESTINATION, IF_STATEMENT, LOOP_STATEMENT, RETURN_STATEMENT, EXPRESSION, EXPRESSIONEXT, ARITHOP, ARITHOPEXT, RELATION, RELATIONEXT, TERM, TERMEXT, FACTOR, NAME, ARGUMENT_LIST
};

//Current Scope (0 = global)
int scope = 0;
list<string> procedureNames = {};
tokenType currentVariableType;
string tokenNameToReference;
token tokenToReference;
list<tokenType> parameterTypeList;
bool resyncFinish = false;

//Scanner Method
token ScanOneToken(bool declaration = false, tokenType variableType = UNKNOWN)
{
	//Load in the character
	token outToken;
	outToken.variableType = variableType;
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
					else if (currentChar == EOF) {
						break;
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
		//Convert to lowercase (compiler is case insensitive)
		std::transform(outToken.name.begin(), outToken.name.end(), outToken.name.begin(), ::tolower);
		//Check if token is in local symbol table
		if (symbolTables.back().find(outToken.name) != symbolTables.back().end())
		{
			outToken = symbolTables.back()[outToken.name];
		}
		//If not local, check if token is in global symbol table
		else if (symbolTables.front().find(outToken.name) != symbolTables.front().end())
		{
			outToken = symbolTables.front()[outToken.name];
		}
		//If not in either symbol table, give it the identifier type
		else
		{
			outToken.type = IDENTIFIER;
			outToken.name = outToken.name;
			if (declaration) {
				outToken.declared = true;
				cout << outToken.name << " declared" << endl;
			}
			//If currently in global scope, add to global symbol table (bottom of stack)
			if (scope == 0)
			{
				symbolTables.front()[outToken.name] = outToken;
				//cout << tokenName << " added to global symbol table" << endl;
			}
			//If not currently in global scope, add to local symbol table (top of stack)
			else 
			{
				symbolTables.back()[outToken.name] = outToken;
				//cout << tokenName << " added to local symbol table " << scope << endl;
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
	return outToken;
}

//Parser Declaration
bool parse(token currentToken, nonTerminal nonTerminal);

//Unget Token
void UngetToken(string token) {
	for (std::string::reverse_iterator rit = token.rbegin(); rit!=token.rend(); rit++) {
		ungetc(*rit, file);
	}
}

/*Errors*/

//Resync method
void resync(token currentToken) {
	while (currentToken.type != BEGIN & currentToken.type != ENDFILE & currentToken.type != END) {
		currentToken = ScanOneToken();
	}
	if (currentToken.type == BEGIN & scope > 1) {
		cout << "Resynced to line " << lineNumber << endl;
		parse(currentToken, PROCEDURE_BODY);
	}
	else if (currentToken.type == BEGIN & scope == 1) {
		cout << "Resynced to line " << lineNumber << endl;
		parse(currentToken, PROGRAM_BODY);
	}
	else if (currentToken.type == END) {
		currentToken = ScanOneToken();
		if (currentToken.type == PROCEDURE) {
			tokenToReference = symbolTables.back()[procedureNames.back()];
			scope--;
			symbolTables.pop_back();
			symbolTables.back()[procedureNames.back()] = tokenToReference;
			procedureNames.pop_back();
		}
		else if (currentToken.type == PROGRAM) {
			currentToken = ScanOneToken();
			if (currentToken.type != PERIOD) {
				cout << "ERROR: Period expected at line " << lineNumber << endl;
			}
			symbolTables.pop_back();
			scope--;
		}
		resync(currentToken);
	}
	else if (currentToken.type == ENDFILE) {
		cout << "No resync point (begin) found" << endl;
	}
	resyncFinish = true;
}

//Scope Decelaration Error Message
void ScopeDeclarationError(string tokenName) {
	cout << endl << "ERROR: Identifier " << tokenName << " not defined in this scope at line " << lineNumber << endl << endl;
}

//Redeclaration Error Message
void redeclarationError(token errorToken) {
	cout << "ERROR: Variable " << errorToken.name << " at line " << lineNumber << " already declared as " << errorToken.variableType << endl;
}

//Non-negatable variable error
void negationError(token errorToken) {
	cout << "ERROR: " << errorToken.name << " cannot be negated at aline " << lineNumber << endl;
}

//Type Error
void typeError(tokenType unexpectedType) {
	cout << "ERROR: Unexpected " << unexpectedType  << " at line " << lineNumber << endl;
}

//Parser Method
bool parse(token currentToken, nonTerminal nonTerminal) {
	tokenType typeToCompare = UNKNOWN;
	bool isParsed = false;
	string expectedToken;
	token nextToken;

	//cout << "Parsing " << nonTerminal << " at line " << lineNumber <<endl;
	switch (nonTerminal) {

	//PROGRAM PARSE
	case PROGRAM_MAIN:
		if (!parse(currentToken, PROGRAM_HEADER)) {
			break;
		}
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
		currentToken = ScanOneToken(true, PROGRAM);
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "identifier";
			break;
		}
		scope++;
		symbolTables.push_back({ {currentToken.name, {currentToken}} });
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
		tokenToReference = symbolTables.back()[procedureNames.back()];
		scope--;
		symbolTables.pop_back();
		symbolTables.back()[procedureNames.back()] = tokenToReference;
		procedureNames.pop_back();
		break;
	case PROCEDURE_HEADER:
		if (currentToken.type != PROCEDURE) {
			break;
		}
		currentToken = ScanOneToken(true, PROCEDURE);
		if (currentToken.variableType != PROCEDURE && scope != 0) {
			redeclarationError(symbolTables.back()[currentToken.name]);
		}
		else if (currentToken.variableType != currentVariableType && scope == 0) {
			redeclarationError(symbolTables.back()[currentToken.name]);
		}
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
			break;
		}
		tokenNameToReference = currentToken.name;
		procedureNames.push_back(currentToken.name);
		scope++;
		symbolTables.push_back({ {currentToken.name, {currentToken}} });
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
		if (scope != 0) {
			symbolTables.back()[tokenNameToReference].parameterTypes.push_back(currentVariableType);
		}
		else {
			symbolTables.front()[tokenNameToReference].parameterTypes.push_back(currentVariableType);
		}
		currentToken = ScanOneToken();
		if (currentToken.type == IN) {
			cout << "In parameter" << endl;
		}
		else if (currentToken.type == OUT) {
			cout << "Out parameter" << endl;
			if (scope != 0) {
				symbolTables.back()[tokenNameToReference].procedureOutType = currentVariableType;
			}
			else {
				symbolTables.front()[tokenNameToReference].procedureOutType = currentVariableType;
			}
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
		currentToken = ScanOneToken(true, currentVariableType);
		if (currentToken.variableType != currentVariableType && scope != 0) {
			redeclarationError(symbolTables.back()[currentToken.name]);
		}
		else if (currentToken.variableType != currentVariableType && scope == 0) {
			redeclarationError(symbolTables.back()[currentToken.name]);
		}
		if (currentToken.type != IDENTIFIER) {
			expectedToken = "Identifier";
			break;
		}
		nextToken = ScanOneToken();
		if (nextToken.type == LBRACKET) {
			tokenNameToReference = currentToken.name;
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
			currentVariableType = INTVAL;
		}
		else if (currentToken.type == FLOAT) {
			cout << "Float type mark" << endl;
			currentVariableType = FLOATVAL;
		}
		else if (currentToken.type == STRING) {
			cout << "String type mark" << endl;
			currentVariableType = STRINGVAL;
		}
		else if (currentToken.type == BOOL) {
			cout << "Bool type mark" << endl;
			currentVariableType = BOOLVAL;
		}
		else if (currentToken.type == CHAR) {
			cout << "Char type mark" << endl;
			currentVariableType = STRINGVAL;
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
		//if (!parse(currentToken, EXPRESSION)) {
		//	break;
		//}
		if (currentToken.type == MINUS) {
			currentToken = ScanOneToken();
			typeToCompare = MINUS;
		}
		if (currentToken.type != INTVAL) {
			expectedToken = "Integer";
			break;
		}
		if (scope != 0) {
			if (typeToCompare != MINUS) {
				symbolTables.back()[tokenNameToReference].lowerBound = stoi(currentToken.name);

			}
			else {
				symbolTables.back()[tokenNameToReference].lowerBound = -stoi(currentToken.name);
			}
		}
		else{
			if (typeToCompare != MINUS) {
				symbolTables.front()[tokenNameToReference].lowerBound = stoi(currentToken.name);
			}
			else {
				symbolTables.front()[tokenNameToReference].lowerBound = -stoi(currentToken.name);
			}
		}
		typeToCompare = UNKNOWN;
		currentToken = ScanOneToken();
		if (currentToken.type != COLON) {
			expectedToken = ":";
			break;
		}
		currentToken = ScanOneToken();
		if (currentToken.type == MINUS) {
			currentToken = ScanOneToken();
			typeToCompare;
		}
		if (currentToken.type != INTVAL) {
			expectedToken = "Integer";
			break;
		}
		if (scope != 0) {
			if (typeToCompare != MINUS) {
				symbolTables.back()[tokenNameToReference].upperBound = stoi(currentToken.name);
			}
			else {
				symbolTables.back()[tokenNameToReference].upperBound = -stoi(currentToken.name);
			}
		}
		else {
			if (typeToCompare != MINUS) {
				symbolTables.front()[tokenNameToReference].upperBound = stoi(currentToken.name);
			}
			else {
				symbolTables.front()[tokenNameToReference].upperBound = -stoi(currentToken.name);
			}
		}
		//if (!parse(currentToken, EXPRESSION)) {
		//	break;
		//}
		isParsed = true;
		expectedToken = "Bound";
		break;

	//STATEMENT PARSE
	case STATEMENT:
		if (currentToken.type == IDENTIFIER) {
			if (!currentToken.declared) {
				ScopeDeclarationError(currentToken.name);
			}
			tokenToReference = currentToken;
			currentVariableType = currentToken.variableType;
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
		if (currentVariableType != PROCEDURE) {
			typeError(currentVariableType);
		}
		currentToken = ScanOneToken();
		parameterTypeList = tokenToReference.parameterTypes;
		if (currentToken.type != RPAREN) {
			parse(currentToken, ARGUMENT_LIST);
			currentToken = ScanOneToken();
			if (currentToken.type != RPAREN) {
				expectedToken = ")";
				break;
			}
		}
		if (parameterTypeList.size() != 0) {
			cout << "ERROR: Not enough arguments at line " << lineNumber << endl;
		}
		currentVariableType = tokenToReference.procedureOutType;
		isParsed = true;
		expectedToken = "Procedure call";
		break;

	//ASSIGNMENT STATEMENT
	case ASSIGNMENT_STATEMENT:
		if (currentToken.type != ASSIGN) {
			break;
		}
		typeToCompare = currentVariableType;
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSION)) {
			break;
		}
		if (typeToCompare != currentVariableType & 
			!(typeToCompare == FLOATVAL & currentVariableType == INTVAL) & !(typeToCompare == INTVAL & currentVariableType == FLOATVAL) & 
			!(typeToCompare == BOOLVAL & currentVariableType == INTVAL) & !(typeToCompare == INTVAL & currentVariableType == BOOLVAL)) {
			typeError(currentVariableType);
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
			if (currentVariableType != INTVAL) {
				typeError(currentVariableType);
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
		if (currentVariableType != BOOLVAL) {
			typeError(currentVariableType);
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
		else if (!currentToken.declared) {
			ScopeDeclarationError(currentToken.name);
		}
		currentVariableType = currentToken.variableType;
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
		if (currentVariableType != BOOLVAL) {
			typeError(currentVariableType);
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
			typeToCompare = BOOLVAL;
		}
		if (!parse(currentToken, ARITHOP)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSIONEXT)) {
			break;
		}
		if (typeToCompare == BOOLVAL & currentVariableType != BOOLVAL & currentVariableType != INTVAL) {
			typeError(currentVariableType);
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
		if (currentVariableType != BOOLVAL & currentVariableType != INTVAL) {
			typeError(currentVariableType);
		}
		typeToCompare = currentVariableType;
		currentToken = ScanOneToken();
		if (!parse(currentToken, ARITHOP)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, EXPRESSIONEXT)) {
			break;
		}
		if (currentVariableType != typeToCompare) {
			typeError(currentVariableType);
		}
		currentVariableType = BOOLVAL;
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
		if (currentVariableType != INTVAL & currentVariableType != FLOATVAL) {
			typeError(currentVariableType);
		}
		typeToCompare = currentVariableType;
		currentToken = ScanOneToken();
		if (!parse(currentToken, RELATION)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, ARITHOPEXT)) {
			break;
		}
		if (currentVariableType != INTVAL & currentVariableType != FLOATVAL) {
			typeError(currentVariableType);
		}
		else if (typeToCompare == INTVAL & currentVariableType == INTVAL) {
			currentVariableType = INTVAL;
		}
		else {
			currentVariableType = FLOATVAL;
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
		if (currentVariableType != BOOLVAL & currentVariableType != INTVAL & currentVariableType != FLOATVAL & currentVariableType != STRINGVAL) {
			typeError(currentVariableType);
		}
		else if (currentVariableType == STRINGVAL & currentToken.type != EQUAL & currentToken.type != NOTEQUAL) {
			typeError(currentVariableType);
		}
		typeToCompare = currentVariableType;
		currentToken = ScanOneToken();
		if (!parse(currentToken, TERM)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, RELATIONEXT)) {
			break;
		}
		if (currentVariableType != BOOLVAL & currentVariableType != INTVAL & currentVariableType != FLOATVAL & currentVariableType != STRINGVAL) {
			typeError(currentVariableType);
		}
		else if (currentVariableType != typeToCompare & (currentVariableType == STRINGVAL | typeToCompare == STRINGVAL)) {
			typeError(currentVariableType);
		}
		currentVariableType = BOOLVAL;
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
		if (currentVariableType != INTVAL & currentVariableType != FLOATVAL) {
			typeError(currentVariableType);	
		}
		typeToCompare = currentVariableType;
		currentToken = ScanOneToken();
		if (!parse(currentToken, FACTOR)) {
			break;
		}
		currentToken = ScanOneToken();
		if (!parse(currentToken, TERMEXT)) {
			break;
		}
		if (currentVariableType != INTVAL & currentVariableType != FLOATVAL) {
			typeError(currentVariableType);
		}
		currentVariableType = FLOATVAL;
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
			if (!currentToken.declared) {
				ScopeDeclarationError(currentToken.name);
			}
			currentVariableType = currentToken.variableType;
			cout << "Current variable type: " << currentToken.variableType << endl;
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
				currentVariableType = currentToken.type;
				isParsed = true;
				expectedToken = "Factor";
				break;
			}
			else if (currentToken.type == IDENTIFIER) {
				if (!currentToken.declared) {
					ScopeDeclarationError(currentToken.name);
				}
				if (currentToken.variableType != INTVAL & currentToken.variableType != FLOATVAL) {
					negationError(currentToken);
				}
				currentVariableType = currentToken.variableType;
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
			currentVariableType = currentToken.type;
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		else if (currentToken.type == STRINGVAL) {
			currentVariableType = STRINGVAL;
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		else if (currentToken.type == TRUE) {
			currentVariableType = BOOLVAL;
			isParsed = true;
			expectedToken = "Factor";
			break;
		}
		else if (currentToken.type == FALSE) {
			currentVariableType = BOOLVAL;
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
		if (parameterTypeList.size() == 0) {
			cout << "ERROR: Too many arguments at line " << lineNumber << endl;
			break;
		}
		if (parameterTypeList.front() != currentVariableType &
		!(parameterTypeList.front() == FLOATVAL & currentVariableType == INTVAL) & !(parameterTypeList.front() == INTVAL & currentVariableType == FLOATVAL) &
		!(parameterTypeList.front() == BOOLVAL & currentVariableType == INTVAL) & !(parameterTypeList.front() == INTVAL & currentVariableType == BOOLVAL)) {
			typeError(currentVariableType);
		}
		parameterTypeList.pop_front();
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
	if (resyncFinish) {
		expectedToken = "";
	}
	if (isParsed & expectedToken != "") {
		//cout << expectedToken << " " << currentToken.name << " parsed successfully at line " << lineNumber << endl;
	}
	else if (expectedToken != "") {
		cout << endl <<  "ERROR: Expected " << expectedToken << ", got " << currentToken.name << " at line " << lineNumber << endl
			<< "Resyncing..." << endl << endl;
		resync(currentToken);
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
	scope = 0;
	cout << endl << "Global Symbols (name: type): " << endl;
	for (symbol_table symbolTable : symbolTables) 
	{
		cout << "Table " << scope << ": ";
		symbol_table::iterator itr;
		for (itr = symbolTable.begin(); itr != symbolTable.end(); itr++)
		{
			cout << itr->second.name << ": " << itr->second.type << ",";
		}
		cout << endl;
		scope++;
	}
	cout << endl << endl << "end!" << endl;
	return 0;
	}


