putfloat:
FILE* outFile = fopen("./output.txt","w");
char out[255];
out = fprintf(out, "%f", M[SP + 2]);
fputs(out, outFile);
fclose(outFile);
goto returnAddr;