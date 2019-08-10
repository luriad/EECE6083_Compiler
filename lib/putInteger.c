putinteger:
FILE* outFile = fopen("./output.txt","w");
char out[255];
sprintf(out, %d, M[SP + 2]);
fputs(out, outFile);
fclose(outFile);
goto returnAddr;