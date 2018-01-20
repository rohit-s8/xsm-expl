#include <string.h>

static void ext_rem(char* filename){
	char* temp = filename;
	int len = strlen(temp);
	while(--len && temp[len]!='/' && temp[len]!='.')
		;
	if(temp[len]=='.')
		temp[len]='\0';
}

static char* ext_rep(char* filename, char* ext){
	char* temp = strdup(filename);
	ext_rem(temp);
	strcat(temp,ext);
	return temp;
}

char* obfile(char* filename){
	return ext_rep(filename,".xo");
}

char* exefile(char* filename){
	return ext_rep(filename,".xsm");
}
