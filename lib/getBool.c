getbool:
FILE* inFile = fopen("./input.txt","r");
bool out;
char read[5];
fgets(read, 5, (FILE*)inFile);
if(read == "true") out = true;
else if (read == "false") out = false;
M[SP] = out;
fclose(inFile);
goto returnAddr;