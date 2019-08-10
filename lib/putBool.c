putbool:
FILE* outFile = fopen("./output.txt","w");
char out[5];
if(M[SP + 2] == true) out = "true";
else if(M[SP + 2] == false) out = "false";
fputs(out, outFile);
fclose(outFile);
goto returnAddr;