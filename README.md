# EECE6083_Compiler
Compiler Project for EECE6083
David Luria
Updated: 8/10/2019

**File structure:**
* /src contains the source code of the project and /bin contains the binaries to run the compiler
* Each binary includes one required argument: the location of the program to be compiled. So it should be run as such:

  ./bin_name [file location]

* You will find two versions of the compiler: standard and verbose. The standard binary will create CodeGen.c, which is the IR of the compiled program in reduced C form. It will print any errors detected to the console. Verbose does the same, except it also prints the state of the parser throughout the compiling proccess. It also produces a commented CodeGen.c to give more information on the code generated.
* Generated code is in reduced assembly-like C
* /lib contains standard I/O library functions, like putInteger() and getString(). These are statically linked into the IR C program.

**Description:**
This compiler project was build in c++ using a one-pass process. The scanner is a method that simply looks through the input code and produce
"tokens". Tokens are pieces of code that represent pieces of logic that can be used to build the entire program. These are things like identifiers,
line ends, numbers, etc. Tokens are fed from the scanner to the parser. The parser makes sense of the string of tokens using grammar rules.
Grammar rules are syntactic and simply check to make sure that the code follows the structure of the source language. I have also embedded
more samantic-focused rules in the structure of the parser code. These rules include things such as type-checking (i.e. to make sure variables
are assigned the correct type) and scoping information. The code includes symbol table management, which collect unique identifiers and phrases
and stores them in a hash table. Several symbol tables are managed, one per scope. So, when a procedure is defined, a new symbol table is created
that manages the variables of that procedure's scope. There is also a global symbol table that all scopes can access. The global table includes 
symbols that are expected to be used globally, such as "variable", "integer", "procedure", "end", etc., as well as any globally-defined variables. The compiler generates code to a file called CodeGen.c. This code is an intermediate representation in the form of a reduced assembly-like C program. The generated code can then be compiled using a C compiler (like gcc). The compiler also statically links standard library functions (in /lib) as needed.

This compiler proved to be a very difficult coding project for me. 
