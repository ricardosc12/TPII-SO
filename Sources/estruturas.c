#include <stdlib.h>
#include <stdio.h>

typedef struct Vetor Vetor;

struct Vetor{
    int size;
    int *lista;
    void (*push)(Vetor *, int);
    void (*imprime)(Vetor);
    int (*remover)(Vetor*,int);
    void (*clear)(Vetor*);
};

void push(Vetor *vetor,int value){
    vetor->lista = realloc(vetor->lista,(vetor->size+1)*(sizeof(int)));
    vetor->lista[vetor->size] = value;
    vetor->size+=1;
}
void imprime(Vetor vetor){
    for(int i=0;i<vetor.size;i++){
        printf("%d ",vetor.lista[i]);
    }
    printf("\n");
}
int getPosV(Vetor *vetor,int value){
    for(int i=0;i<vetor->size;i++){
        if(value == vetor->lista[i]) return i;
    }
    return -1;
} 
int remover(Vetor *vetor, int value){
    int pos = getPosV(vetor,value);
    if(pos == -1) return -1;
    if(vetor->size == 0 || vetor->size <= pos) return -1;
    int removido = vetor->lista[pos];
    int quantidades = vetor->size - pos - 1;
    if(vetor->size == 1){
        vetor->lista = NULL;
        vetor->size-=1;
        return removido;
    }
    for(int i=0;i<quantidades;i++){
        vetor->lista[i+pos] = vetor->lista[i+pos+1];
    }
    vetor->size-=1;
    vetor->lista = realloc(vetor->lista,vetor->size*sizeof(int));
    return removido;
}
void clear(Vetor *vetor){
    vetor->lista = NULL;
}

Vetor createVetor(){
    Vetor vetor;
    vetor.size = 0;
    vetor.lista = NULL;
    vetor.push = &push;
    vetor.imprime = &imprime;
    vetor.remover = &remover;
    vetor.clear = &clear;
    return vetor;
}
typedef struct {
    char** instrucoes;
}Programa;

typedef struct {
    char** instrucoes;
}Dados;

typedef struct {
    int PID;
    int PID_PAI;
    int PC;
    Dados dados;
    int prioridade;
    int estado;
    int temp_inicio;
    int temp_cpu;
    int *fatia_memoria;
}Processo;


typedef struct {
    int quantidade;
    int give_pid;
    Processo processos[100];
}TabelaProcessos;

typedef struct {
    int lista[100];
}Memoria;
void exibeInformacoes(Processo processo,int x,int y, Memoria memoria, char* estado){
    printf("\n\e[1;97m  PID: %d\e[0m\n",processo.PID);
    printf("\e[1;97m  PID_PAI: %d\e[0m\n",processo.PID_PAI);
    printf("\e[1;97m  PC: %d\e[0m\n",processo.PC);
    printf("\e[1;97m  Prioridade: %d\e[0m\n",processo.prioridade);
    printf("\e[1;97m  Estado: %s\e[0m\n",estado);
    printf("\e[1;97m  Temp_inicio: %d\e[0m\n",processo.temp_inicio);
    printf("\e[1;97m  Tempo CPU: %d\e[0m\n",processo.temp_cpu);
    int quantidade = (y-x)+1;
    int count = 0;
    printf("\e[1;97m  Dados: \e[0m\n");
    if(x>-1 && y>-1){
        for(int i=x;i<quantidade+x;i++){
            printf("\e[1;97m   Var %d: %d\e[0m\n",count,memoria.lista[i]);
            count++;
        }
    }else printf("\e[1;97m   Não alocado !\e[0m\n");

    // printf("Exec Instrução: %s",processo.dados.instrucoes[processo.PC]);
    printf("\n\n");
}


typedef struct {
    int fatia_tempo;
    int tempo_usado;
    int PID;
    int PC;
    int *fatia_memoria;
    Memoria memoria;
    Programa programa;
}Cpu;

typedef struct Gerenciador_de_Processos Gerenciador_de_Processos;

struct Gerenciador_de_Processos{
    Cpu cpu;
    int tempo;
    TabelaProcessos tabela_processos;
    TabelaProcessos tabela_concluidos;
    Vetor EstadoExec;  
    Vetor EstadoPronto;
    Vetor EstadoBloq;
    int (*adiciona)(Gerenciador_de_Processos*,Processo processo);
    void (*imprimeG)(Gerenciador_de_Processos);
    int (*getPos)(Gerenciador_de_Processos*,int);
    Processo* (*removerG)(Gerenciador_de_Processos*,int);
};

int adiciona(Gerenciador_de_Processos *gerenciador,Processo processo){
    int *give_pid = malloc(sizeof(int));
    int tam = gerenciador->tabela_processos.quantidade;
    give_pid[0] = gerenciador->tabela_processos.give_pid;
    gerenciador->tabela_processos.processos[tam] = processo;
    gerenciador->tabela_processos.processos[tam].PID = give_pid[0];
    gerenciador->tabela_processos.quantidade++;
    gerenciador->tabela_processos.give_pid++;
    return give_pid[0];
}

int getPos(Gerenciador_de_Processos *gerenciador,int pid){
    for(int i=0;i<gerenciador->tabela_processos.quantidade;i++){
        if(gerenciador->tabela_processos.processos[i].PID == pid){
            return i;
        }
    }
    return -1;
}

void imprimeG(Gerenciador_de_Processos gerenciador){
    printf(" Quantidade: %d",gerenciador.tabela_processos.quantidade);
    for(int i=0;i<gerenciador.tabela_processos.quantidade;i++){
        exibeInformacoes(gerenciador.tabela_processos.processos[i],
        gerenciador.tabela_processos.processos[i].fatia_memoria[0],
        gerenciador.tabela_processos.processos[i].fatia_memoria[1],
        gerenciador.cpu.memoria,
        "Concluído !");
    }
    printf("  -------------------\n");
}

void imprimeProntos(Gerenciador_de_Processos gerenciador){
    printf("\e[1;93m Processo em estado pronto - Quantidade: %d \e[0m\n",gerenciador.EstadoPronto.size);
    for(int i=0;i<gerenciador.EstadoPronto.size;i++){
        int pos = getPos(&gerenciador,gerenciador.EstadoPronto.lista[i]);
        exibeInformacoes(gerenciador.tabela_processos.processos[pos],
        gerenciador.tabela_processos.processos->fatia_memoria[0],
        gerenciador.tabela_processos.processos->fatia_memoria[1],
        gerenciador.cpu.memoria,
        "Pronto !");
    }
    printf("  -------------------\n");
}

void imprimeBloqueados(Gerenciador_de_Processos gerenciador){
    printf("\e[0;91m Processo em estado bloqueado - Quantidade: %d\e[0m\n",gerenciador.EstadoBloq.size);
    for(int i=0;i<gerenciador.EstadoBloq.size;i++){
        int pos = getPos(&gerenciador,gerenciador.EstadoBloq.lista[i]);
        exibeInformacoes(gerenciador.tabela_processos.processos[pos],
        gerenciador.tabela_processos.processos->fatia_memoria[0],
        gerenciador.tabela_processos.processos->fatia_memoria[1],
        gerenciador.cpu.memoria,
        "Bloqueado !");
    }
    printf("  -------------------\n");
}

void imprimeConcluidos(Gerenciador_de_Processos gerenciador){
    printf("\e[1;94m Processo Concluídos - Quantidade: %d\e[0m\n",gerenciador.tabela_concluidos.quantidade);
    for(int i=0;i<gerenciador.tabela_concluidos.quantidade;i++){

        exibeInformacoes(gerenciador.tabela_concluidos.processos[i],
        gerenciador.tabela_concluidos.processos[i].fatia_memoria[0],
        gerenciador.tabela_concluidos.processos[i].fatia_memoria[1],
        gerenciador.cpu.memoria,
        "Concluído !");
    }
    printf("  -------------------\n");
}

Processo* removerG(Gerenciador_de_Processos *gerenciador,int pid){
    int quantidade = gerenciador->tabela_processos.quantidade;
    if(quantidade == 0) return NULL;
    int pos = getPos(gerenciador,pid);
    Processo processo = gerenciador->tabela_processos.processos[pos];
    Processo *process_copy = &processo;
    int quantidades = quantidade-pos-1;
    for(int i=0;i<quantidades;i++){
        gerenciador->tabela_processos.processos[i+pos] = gerenciador->tabela_processos.processos[i+pos+1];
    }
    gerenciador->tabela_processos.quantidade--;
    return process_copy;
}

void iniciarMemoria(Memoria *memoria){
    for(int i=0;i<100;i++){
        memoria->lista[i] = -1;
    }
}

Gerenciador_de_Processos iniciaGerenciador(){

    Gerenciador_de_Processos gerenciador;
    gerenciador.tempo=0;
    gerenciador.EstadoBloq = createVetor();
    gerenciador.EstadoExec = createVetor();
    gerenciador.EstadoPronto = createVetor();
    gerenciador.tabela_processos.quantidade = 0;
    gerenciador.tabela_processos.give_pid = 0;
    gerenciador.adiciona = &adiciona;
    gerenciador.getPos = &getPos;
    gerenciador.removerG = &removerG;
    gerenciador.imprimeG = &imprimeG;
    gerenciador.cpu.fatia_memoria = NULL;
    iniciarMemoria(&(gerenciador.cpu.memoria));
    return gerenciador;
}

void imprimeEstados(Gerenciador_de_Processos gerenciador){
    printf("-------------------");
    for(int i=0;i<gerenciador.EstadoExec.size;i++){
        printf("PID: %d\n",gerenciador.EstadoExec.lista[i]);
    }
    printf("-------------------");
}


void teste_vetor(){
    Vetor vetor = createVetor();
    vetor.push(&vetor,1);
    vetor.push(&vetor,2);
    vetor.remover(&vetor,0);
    vetor.imprime(vetor);
    vetor.clear(&vetor);
}

void imprimeProcessoCpu(Gerenciador_de_Processos gerenciador){
    printf("\e[1;92m Processo em execução - Quantidade %d\e[0m\n",gerenciador.EstadoExec.size);
    if(!gerenciador.EstadoExec.size) {
        printf(" -------------------\n");
        return;
    }
    int pos = getPos(&gerenciador,gerenciador.cpu.PID);
    Processo processo = gerenciador.tabela_processos.processos[pos];
    printf("\n\e[1;97m  PID: %d\e[0m\n",processo.PID);
    printf("\e[1;97m  PID_PAI: %d\e[0m\n",processo.PID_PAI);
    printf("\e[1;97m  PC: %d\e[0m\n",gerenciador.cpu.PC);
    printf("\e[1;97m  Prioridade: %d\e[0m\n",processo.prioridade);
    printf("\e[1;97m  Estado: Executando !\e[0m\n");
    printf("\e[1;97m  Temp_inicio: %d\e[0m\n",processo.temp_inicio);
    printf("\e[1;97m  Tempo CPU: %d\e[0m\n",gerenciador.cpu.tempo_usado);
    int x = gerenciador.cpu.fatia_memoria[0];
    int y = gerenciador.cpu.fatia_memoria[1];
    int quantidade = (y-x)+1;
    int count = 0;
    printf("\e[1;97m  Dados: \e[0m\n");
    if(x>-1 && y>-1){
        for(int i=x;i<quantidade+x;i++){
            printf("\e[1;97m   Var %d: %d\e[0m\n",count,gerenciador.cpu.memoria.lista[i]);
            count++;
        }
    }else printf("\e[1;97m   Não alocado !\e[0m\n");

    // printf("Exec Instrução: %s",processo.dados.instrucoes[processo.PC]);
    printf("\n\n  -------------------\n");
}