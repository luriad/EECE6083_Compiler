getstring;
FILE* inFile = fopen("./input.txt","r");
char out[255];
fgets(out, 255, (FILE*)inFile);
M[SP] = out;
fclose(inFile);
goto returnAddr;