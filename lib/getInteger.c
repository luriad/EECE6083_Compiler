getinteger:
FILE* inFile = fopen("./input.txt","r");
int out;
char read[255];
fgets(read, 255, (FILE*)inFile);
out = strtol(read,NULL,10);
M[SP] = out;
fclose(inFile);
goto returnAddr;