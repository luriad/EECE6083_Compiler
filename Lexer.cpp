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
string reservedWords[] = {"return", "while", "if", "then", "else", "for", "program", "begin", "end", "in", "out", "global", "is", "true", "false", "procedure", "integer", "float"};

//Scanner Method
tokenType ScanOneToken(FILE *file)
{
	//Load in the character
	char currentChar = getc(file);
	char nextChar;
	tokenType token;

	//Skip whitespace
	while (isspace(currentChar))
	{
		currentChar = getc(file);
	}

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
			token = static_cast<tokenType>(currentChar);
		}
		break;
	//Regular single character tokens
	case ';' : case '(': case ')': case '+': case '*': case '[': case ']': case '{': case '}': case '|': case '&': case ',': case '%': case '.': case '-':
		token = static_cast<tokenType>(currentChar);
		break;
	case '!':
		nextChar = getc(file);
		//Check if NOTEQUAL
		if (nextChar == '=')
		{
			token = NOTEQUAL;
		}
		//Else, recognize as NOT logic
		else
		{
			ungetc(nextChar, file);
			token = NOT;
		}
		break;
	case '>':
		nextChar = getc(file);
		//Check if GREATERTHANEQUAL
		if (nextChar == '=')
		{
			token = GREATERTHANEQUAL;
		}
		//Else, recognize as normal GREATERTHAN
		else
		{
			ungetc(nextChar, file);
			token = GREATERTHAN;
		}
		break;
	case '<':
		nextChar = getc(file);
		//Check if LESSTHANEQUAL
		if (nextChar == '=')
		{
			token = LESSTHANEQAL;
		}
		//Else, recognize as normal LESSTHAN
		else
		{
			ungetc(nextChar, file);
			token = LESSTHAN;
		}
		break;
	case ':':
		nextChar = getc(file);
		//Check if assignment token
		if (nextChar == '=')
		{
			token = ASSIGN;
		}
		//Else, recognize as COLON
		else
		{
			ungetc(nextChar, file);
			token = COLON;
		}
		break;
	case '=':
		nextChar = getc(file);
		//Check if comparison (==)
		if (nextChar == '=')
		{
			token = EQUAL;
		}
		//Else, '=' is not in this language so return UNKNOWN
		else
		{
			ungetc(nextChar, file);
			token = UNKNOWN;
		}
		break;
	//Single quote case (string)
	case '\'':
		//Keep scanning until next single quote reached
		do
		{
			currentChar = getc(file);
		} while (currentChar != '\'');
		token = STRING;
		break;
	//Double quote string case
	case '"':
		//Keep scanning until next double quote reached
		do
		{
			currentChar = getc(file);
		} while (currentChar != '"');
		token = STRING;
		break;
	case EOF:
		token = ENDFILE;
		break;
	default:
		token = UNKNOWN;
		break;
	}
	//Identifier/Reserved word case
	if (((currentChar >= 'A') & (currentChar <= 'Z')) | ((currentChar >= 'a') & (currentChar <= 'z')) | (currentChar == '_'))
	{
		std::string word;
		bool isReserved = false;
		//Keep scanning a-zA-Z0-9_
		while ((currentChar >= 'A' & currentChar <= 'Z') | (currentChar >= 'a' & currentChar <= 'z') | currentChar == '_' | (currentChar <= '0' & currentChar >= '9'))
		{
			word += currentChar;
			currentChar = getc(file);
		}
		//Check if reserved word (EDIT: USE A LIST LOOKUP OR SYMBOL TABLE RATHER THAN THIS CRAPPY ALGORITHM)
		for (int i = 0; i < sizeof(reservedWords); i++)
		{
			if (reservedWords[i] == word)
			{
				isReserved = true;
				token = static_cast<tokenType>(i + RETURN);
			}
		}
		//If not reserved, recognize as identifier
		if (!isReserved)
		{
			token = IDENTIFIER;
		}
		ungetc(currentChar, file);
	}
	//Number case
	else if (currentChar >= '0' & currentChar <= '9')
	{
		//Keep scanning while numbers are read
		while (currentChar >= '0' & currentChar <= '9')
		{
			currentChar = getc(file);
		}
		//If a decimal is encountered, recognize as float (NOTE: ADD double TYPE)
		if (currentChar == '.')
		{
			do
			{
				currentChar = getc(file);
			} while (currentChar >= '0' & currentChar <= '9');
			token = FLOATVAL;
		}
		//If no decimal encountered, recognize as integer value
		else
		{
			token = INTVAL;
		}
		ungetc(currentChar, file);
	}
	return token;
}

int main()
{
	//Initialization
	cout << "begin" << endl;
	tokenType currentToken = BEGIN;
	std::list<string> symbolTable;
	char filename[] = "";
	FILE* programFile;
	//Console input for filename
	cin >> filename;
	programFile = fopen(filename, "r");
	//Keep scanning until EOF
	while(currentToken != ENDFILE)
	{
		//Trap for 'no file found'
		if (programFile == NULL) 
		{
			cout << "No file found" << endl;
			break;
		}
		currentToken = ScanOneToken(programFile);
		cout<<currentToken<<endl;
	}
	cout << "end!";
	return 0;
	}


