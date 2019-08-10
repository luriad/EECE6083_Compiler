getfloat:
FILE* inFile = fopen("./input.txt","r");
float out;
char read[255];
fgets(read, 255, (FILE*)inFile);
out = strtod(read,NULL);
M[SP] = out;
fclose(inFile);
goto returnAddr;
