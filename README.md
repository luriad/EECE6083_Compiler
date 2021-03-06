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
* Use input.txt to pass inputs into programs, and read outputs from output.txt

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

The compiler will work for some of the simple example programs, such as iterativeFib.src and logicals.src. However, it does not always generate working code. The lexer and parser are fully working, and the correct errors will be generated when they're present. The code generation is still very buggy, but is close to completion, but does not work in all cases. For example, I had a hard time handling strings in the code generation. So, programs with lots of strings likely won't generate good code. I would consider my code generation mostly done, but not completely. I do have the runtime environment set up. I statically link the necessary library functions in the code generation step. This allows the programs that my code generation *can* handle (such as iterativeFib.src) to fully compile and execute. Overall, this proved to be a very difficult programming project. Building a parser was unlike anything I had done before and taught me a lot about how languages are read by the machine. Code generation allowed me to apply my knowledge of load-store architectures, which was a nice refresher of assembly code. Esepecially considering my background (physics!), I'm glad that I was able to get some of the example programs to compile. This project was certainly a challenge and I feel like I did learn a lot in the proccess, even if I didn't end up with a 100% fully functional compiler.
