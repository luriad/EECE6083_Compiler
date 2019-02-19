/*EECE6083: LEXER
 * Programmer: David Luria
 * Date updated: 2/3/2019
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <list>
using namespace std;

//Token Types
enum tokenType {
	SEMICOLON = ';', LPAREN = '(', RPAREN = ')', COLON = ':', PLUS = '+', MINUS = '-', STAR = '*', SLASH = '/', LBRACKET = '[', RBRACKET = ']', LBRACE = '{', RBRACE = '}', OR = '|', AND = '&', NOT = '!', COMMA = ',', MODULUS = '%',
	LESSTHAN = '<', GREATERTHAN = '>', SINGLEQUOTE = '\'', DOUBLEQUOTE = '"', PERIOD = '.', ASSIGN = 257, EQUAL, LESSTHANEQAL, GREATERTHANEQUAL, NOTEQUAL, RETURN, WHILE, IF, THEN, ELSE, FOR, PROGRAM, BEGIN, END, IN, OUT, GLOBAL,
	IS, TRUE, FALSE, PROCEDURE, INTEGER, FLOAT, IDENTIFIER, INTVAL, FLOATVAL, STRING, UNKNOWN, ENDFILE
};

//Token structure
struct token {
	tokenType type;
	string name;
};

//Symbol Table
list<token> symbolTable = { {RETURN, "return"}, {WHILE, "while"} , {IF, "if"} , {THEN, "then"} , {ELSE, "else"} , {FOR, "for"} , {PROGRAM, "program"} , {BEGIN, "begin"} ,
{END, "end"} , {IN, "in"} , {OUT, "out"} , {GLOBAL, "global"} , {IS, "is"} , {TRUE, "true"} , {FALSE, "false"} , {PROCEDURE, "procedure"} , {FLOAT, "float"} };

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
		currentChar = getc(file);
	}

	outToken.name.push_back(currentChar);

	switch (currentChar)
	{
	case '/':
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
		}
		//See if multi-line comment
		if (nextChar == '*')
		{
			//Skip until comment end (*/)
			do
			{
				currentChar = getc(file);
				if (currentChar == '*')
				{
					nextChar = getc(file);
				}
			} while ((currentChar != '*') | (nextChar != '/'));
		}
		//Otherwise, recognize as divide operator
		else
		{
			//ANYTIME nextchar is used, finish with ungetc if the next character is not needed
			ungetc(nextChar, file);
			outToken.type = static_cast<tokenType>(currentChar);
		}
		break;
	//Regular single character tokens
	case ';' : case '(': case ')': case '+': case '*': case '[': case ']': case '{': case '}': case '|': case '&': case ',': case '%': case '.': case '-':
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
			outToken.type = LESSTHANEQAL;
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
		outToken.type = STRING;
		break;
	//Double quote string case
	case '"':
		//Keep scanning until next double quote reached
		do
		{
			currentChar = getc(file);
			outToken.name.push_back(currentChar);
		} while (currentChar != '"');
		outToken.type = STRING;
		break;
	case EOF:
		outToken.type = ENDFILE;
		outToken.name = "EOF";
		break;
	default:
		outToken.type = UNKNOWN;
		//outToken.name = "ERROR: UNKNOWN TOKEN: " + outToken.name;
		break;
	}
	//Identifier/Reserved word case
	if (((currentChar >= 'A') & (currentChar <= 'Z')) | ((currentChar >= 'a') & (currentChar <= 'z')) | (currentChar == '_'))
	{
		//std::string word;
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

	return outToken;
}

int main()
{
	//Initialization
	cout << "begin" << endl;
	token currentToken = { BEGIN, "test" };
	FILE* programFile = fopen("F:\\Users\\David\\Luria_EECE6083_CompilerProject\\iterativeFib.src", "r");
	//Keep scanning until EOF
	while(currentToken.type != ENDFILE)
	{
		//Trap for 'no file found'
		if (programFile == NULL) 
		{
			cout << "No file found" << endl;
			break;
		}
		currentToken = ScanOneToken(programFile);
		cout << currentToken.type << "  " << currentToken.name << endl;
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


