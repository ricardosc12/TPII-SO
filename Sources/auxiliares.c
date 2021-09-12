#include <string.h>

int stringToInt(char *string,int final){
    char *valor_string = malloc(sizeof(string));
    strncpy(valor_string,string,sizeof(string));
    return atoi(valor_string);
}

char** get_Instrucoes(int *size, char* path){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    int tam = 0;

    char** instrucao = (char**)malloc(sizeof(char*));

    char* destino = malloc(sizeof(char)*(strlen("./InputFiles/")+strlen(path)));
    sprintf(destino,"%s%s","./InputFiles/",path);
    fp = fopen(destino, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1) {
        tam = (long int)read;
        instrucao[count] = (char*)malloc(sizeof(char)*tam);
        strncpy(instrucao[count],line,tam-2);
        count++;
        instrucao = (char**)realloc(instrucao,sizeof(char*)*(count+1));
    }
    fclose(fp);
    if (line){
        free(line);
        free(destino);
    }
        
    *size = count;
    return instrucao;
}