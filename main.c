/*
*   Programa: Gerenciamento de processos
*   Autor: Roniel Nunes Barbosa , Thaís, Ricardo e Vínicios.
*   Matéria: Sistemas Operacionais
*   Professor: Daniel Mendes Barbosa
*/


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "./Sources/api.c"
// #include "./Sources/auxiliares.c"
// Libs para utilizar o comando fork(), esse comando cria um processo filho a partir do main.c(tbm é um processo). 
// Esse processo filho será igual ao processo pai, mas as ações realizadas em cada um são diferentes. Uma variável no 
// main pode ter um valor x, e no processo filho receber outro valor y.
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    system("clear");
    pid_t pid;
    int i;
    int p[2];

    if (pipe(p) < 0) exit(1);

    pid = fork();

    if (pid<0) exit(1);

    if(pid == 0){
        // printf("pid do Filho: %d\n", getpid());
        Gerenciador(p[0]);
    }

    else{
        // printf("pid do Pai: %d\n", getpid());
        Controle(p[1]);

    }
    exit(0);


    // int size;
    // char** instrucao = get_Instrucoes(&size);
    // printf("tam %d %s",size,instrucao[0]);

}

