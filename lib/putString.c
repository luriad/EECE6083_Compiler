putstring:
FILE* outFile = fopen("./output.txt","w");
char out[255];
R[0] = M[SP + 2];
R[1] = M[R[0]];
int i = 0;
while (R[1] != '\0';){
	out[i] = R[1];
	i = i + 1;
	R[1] = M[R[0] + i];
}
fputs(out, outFile);
fclose(outFile);
goto returnAddr;