#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "auxiliares.c"
#include "estruturas.c"

void exibeInformacoes(Processo processo, int x,int y,Memoria,char *estado);
Processo moveToProcessTable(Gerenciador_de_Processos *gerenciador, int pid);

int getQuantum(int prioridade){
    if(prioridade == 0) return 1;
    if(prioridade == 1) return 2;
    if(prioridade == 2) return 4;
    if(prioridade == 3) return 8;
}

int retornaMaiorPrioridade(Gerenciador_de_Processos *gerenciador){
    int maior = 99;
    int pid = -1;
    for(int i=0;i<gerenciador->EstadoPronto.size;i++){
        int pos = gerenciador->getPos(gerenciador,gerenciador->EstadoPronto.lista[i]);
        if(gerenciador->tabela_processos.processos[pos].prioridade < maior){
            maior = gerenciador->tabela_processos.processos[pos].prioridade;
            pid = gerenciador->tabela_processos.processos[pos].PID;
        }
    }
    return pid;
}

int escalona(Gerenciador_de_Processos *gerenciador,int pid){

    if(gerenciador->EstadoExec.size == 0 || gerenciador->cpu.PID == -1){
        return retornaMaiorPrioridade(gerenciador);
    }
    if(gerenciador->EstadoPronto.size == 0 && gerenciador->EstadoExec.size > 0) return gerenciador->cpu.PID;
    
    if(gerenciador->EstadoPronto.size == 0 && gerenciador->EstadoExec.size == 0) return -1;
    
    int pos = gerenciador->getPos(gerenciador,gerenciador->cpu.PID);
    int quantum = getQuantum(gerenciador->tabela_processos.processos[pos].prioridade);
    if(gerenciador->cpu.tempo_usado >= quantum){
        if(gerenciador->tabela_processos.processos[pos].prioridade < 3){
            gerenciador->tabela_processos.processos[pos].prioridade++;
        }
        int prox_pid = retornaMaiorPrioridade(gerenciador);
        if(prox_pid != gerenciador->cpu.PID){
            gerenciador->EstadoExec.remover(&gerenciador->EstadoExec,gerenciador->cpu.PID);
            gerenciador->EstadoPronto.push(&gerenciador->EstadoPronto,gerenciador->cpu.PID);
            moveToProcessTable(gerenciador,gerenciador->cpu.PID);
        }
        return prox_pid;
        
    }
    else{
        return gerenciador->tabela_processos.processos[pos].PID;
    }
}

char* executaInstrucao(Gerenciador_de_Processos *gerenciador,int pid);
Processo criaProcesso(int pid_pai, int pc, Dados dados, 
    int prioridade, int estado,int temp_inicio, int temp_cpu);

char** splitString(char *string, char separator){
    char** string_splited = (char**)malloc(1*sizeof(char*));
    int qt = 0;

    int corte = 0;
    for(int i=0;i<strlen(string);i++){
        if(string[i] == separator && string[i-1] != separator){
            if(i==0) {
                corte = i+1;
            }
            else{
                string_splited[qt] = (char*)malloc((i-corte)*sizeof(char));
                strncpy(string_splited[qt],string+corte,i-corte);
                qt++;
                string_splited = realloc(string_splited,((qt+1)*sizeof(char*)));
                corte = i+1;
            }
        }
        else if(i == strlen(string)-1){
            if(i==0) string_splited[qt] = (char*)malloc((i+1-corte)*sizeof(char));
            else string_splited[qt] = (char*)malloc((i-corte)*sizeof(char));
            strncpy(string_splited[qt],string+corte,i-corte+1);
        }
        if(string[i] == separator && string[i-1] == separator){
            corte = i+1;
        }
    }
    return string_splited;
}

void preencheMemoriaLivre(Memoria *memoria, int *fatia){
    for(int i=fatia[0];i<=fatia[1];i++){
        memoria->lista[i] = 0;
    }
}

void imprimeMemoria(Memoria memoria, int *fatia){
    for(int i=fatia[0];i<=fatia[1];i++){
        printf("%d: %d",i,memoria.lista[i]);
    }
}

void defineValorMemoria(Cpu *cpu, int pos, int valor){
    cpu->memoria.lista[cpu->fatia_memoria[0]+pos] = valor;
}
void somaValorMemoria(Cpu *cpu, int pos, int valor){
    cpu->memoria.lista[cpu->fatia_memoria[0]+pos]+=valor;
}
void subtraiValorMemoria(Cpu *cpu, int pos, int valor){
    cpu->memoria.lista[cpu->fatia_memoria[0]+pos]-=valor;
}

int* fatiaMemoriaLivre(Memoria *memoria,int fatia){
    int space = 0;
    int *inicial_final = (int*)malloc(sizeof(int)*2);
    inicial_final[0] = -1;
    inicial_final[1] = -1;

    int set_pos = 0;
    for(int i=0;i<100;i++){
        if(memoria->lista[i] != -1){
            inicial_final[0] = -1;
            inicial_final[1] = -1;
            space=0;
            set_pos = 0;
        }
        else if(memoria->lista[i] == -1 && space < fatia && !set_pos){
            inicial_final[0]=i;
            set_pos = 1;
            space++;
        }
        else if(memoria->lista[i] == -1 && space < fatia && set_pos){
            space++;
        }
        if(memoria->lista[i] == -1 && space == fatia){
            inicial_final[1]=i;
            break;
        }
    }
    return inicial_final;
}

void moveToCpu(Gerenciador_de_Processos *gerenciador, int pid){
    int pos = gerenciador->getPos(gerenciador,pid);
    Processo *processo = &(gerenciador->tabela_processos.processos[pos]);
    gerenciador->cpu.PC = processo->PC;
    gerenciador->cpu.PID = pid;
    gerenciador->cpu.fatia_memoria = processo->fatia_memoria;
    gerenciador->cpu.tempo_usado = 0;
    gerenciador->cpu.programa.instrucoes = processo->dados.instrucoes;
    gerenciador->EstadoExec.push(&gerenciador->EstadoExec,pid);
    gerenciador->EstadoPronto.remover(&gerenciador->EstadoPronto,pid);
    if(processo->temp_cpu == 0) processo->temp_inicio = gerenciador->cpu.fatia_tempo;
}

Processo moveToProcessTable(Gerenciador_de_Processos *gerenciador, int pid){
    int pos = gerenciador->getPos(gerenciador,pid);
    Processo *processo = &(gerenciador->tabela_processos.processos[pos]);
    processo->PC = gerenciador->cpu.PC;
    processo->dados.instrucoes = gerenciador->cpu.programa.instrucoes;
    processo->temp_cpu += gerenciador->cpu.tempo_usado;
    if(gerenciador->cpu.fatia_memoria != NULL){
        processo->fatia_memoria[0] = gerenciador->cpu.fatia_memoria[0];
        processo->fatia_memoria[1] = gerenciador->cpu.fatia_memoria[1];
    }else processo->fatia_memoria=NULL;
    return *processo;
}

void moveToProcessComplete(Gerenciador_de_Processos *gerenciador, int pid){
    Processo processo = moveToProcessTable(gerenciador,pid);
    int *give_pid = malloc(sizeof(int));
    int tam = gerenciador->tabela_concluidos.quantidade;
    give_pid[0] = gerenciador->tabela_concluidos.give_pid;
    gerenciador->tabela_concluidos.processos[tam] = processo;
    gerenciador->tabela_concluidos.processos[tam].PID = give_pid[0];
    gerenciador->tabela_concluidos.quantidade++;
    gerenciador->tabela_concluidos.give_pid++;

    // Aqui devemos limpar a cpu para outro processo
}

char* executaInstrucao(Gerenciador_de_Processos *gerenciador,int pid){
    Cpu *cpu = &(gerenciador->cpu);
    char *instrucao = cpu->programa.instrucoes[cpu->PC];
    char** options = splitString(instrucao,' ');
    int n;
    int x;
    if(strcmp(options[0],"N") == 0) {
        // write(pid, "N !", 64);
        n = atoi(options[1]);
        int *fatiaMen = fatiaMemoriaLivre(&(cpu->memoria),n);
        preencheMemoriaLivre(&(cpu->memoria),fatiaMen);
        cpu->fatia_memoria = fatiaMen;
        cpu->PC++;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }
    else if (strcmp(options[0],"D") == 0) {
        // write(pid, "D !", 64);
        x = atoi(options[1]);
        defineValorMemoria(cpu,x,0);
        cpu->PC++;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }
    else if (strcmp(options[0],"V") == 0) {
        // write(pid, "V !", 64);
        x = atoi(options[1]);
        n = atoi(options[2]);
        defineValorMemoria(cpu,x,n);
        cpu->PC++;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }
    else if (strcmp(options[0],"A") == 0) {
        // write(pid, "A !", 64);
        x = atoi(options[1]);
        n = atoi(options[2]);
        somaValorMemoria(cpu,x,n);
        cpu->PC++;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }
    else if (strcmp(options[0],"S") == 0) {
        // write(pid, "S !", 64);
        x = atoi(options[1]);
        n = atoi(options[2]);
        subtraiValorMemoria(cpu,x,n);
        cpu->PC++; 
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }
    else if (strcmp(options[0],"B") == 0) {
        write(pid, "Bloqueando processo !", 64);
        cpu->PC++;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;

        moveToProcessTable(gerenciador,cpu->PID);
        gerenciador->EstadoExec.remover(&gerenciador->EstadoExec,cpu->PID);
        gerenciador->EstadoBloq.push(&gerenciador->EstadoBloq,cpu->PID);
        cpu->tempo_usado = 0;
        cpu->PC = 0;
        cpu->PID = -1;
        // cpu->fatia_memoria = NULL;
        cpu->programa.instrucoes = NULL;
    }
    else if (strcmp(options[0],"T") == 0) {
        // write(pid, "Finalizado !", 64);
        cpu->PC++;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
        moveToProcessComplete(gerenciador,cpu->PID);
        gerenciador->removerG(gerenciador,cpu->PID);
        int pss = gerenciador->EstadoExec.remover(&gerenciador->EstadoExec,cpu->PID);
        cpu->tempo_usado = 0;
        cpu->PC = 0;
        // cpu->fatia_memoria = NULL;
        cpu->programa.instrucoes = NULL;
    }
    else if (strcmp(options[0],"F") == 0) {
        // write(pid, "Criando !", 64);
        n = atoi(options[1]);
        Dados dados;
        dados.instrucoes = cpu->programa.instrucoes;
        int pos = gerenciador->getPos(gerenciador,cpu->PID);
        int prioridade = gerenciador->tabela_processos.processos[pos].prioridade;
        cpu->PC++;
        Processo processo = criaProcesso(cpu->PID,cpu->PC,dados,prioridade,0,0,0);
        int pid = gerenciador->adiciona(gerenciador,processo);
        gerenciador->EstadoPronto.push(&gerenciador->EstadoPronto,pid);
        cpu->PC+=n;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }
    else if (strcmp(options[0],"R") == 0) {
        // write(pid, "Clone Sub !", 64);
        int size;
        strcat(options[1],".txt");
        char** instrucoes = get_Instrucoes(&size,options[1]);
        cpu->programa.instrucoes = instrucoes;
        cpu->PC = 0;
        cpu->fatia_tempo++;
        cpu->tempo_usado++;
    }

    return instrucao;
}

void enviaComando(int pid,char *inbuf, Gerenciador_de_Processos *gerenciador){
    if(strcmp(inbuf,"U") == 0){
        int process_id = escalona(gerenciador,pid);
        if(process_id != gerenciador->cpu.PID || gerenciador->cpu.fatia_tempo == 0 && process_id != -1){
            moveToCpu(gerenciador,process_id);
        }
        if(process_id == -1) {
            write(pid, "  Não há processos para executar", 64);
            return;
        }
        executaInstrucao(gerenciador,pid);
        // gerenciador->imprimeG(*gerenciador);
 
    }

    else if(strcmp(inbuf,"L") == 0){
        if(gerenciador->EstadoBloq.size){
            write(pid, "  Desbloqueando processo !", 64);
            int pid_process = gerenciador->EstadoBloq.lista[0];
            gerenciador->EstadoBloq.remover(&gerenciador->EstadoBloq,pid_process);
            gerenciador->EstadoPronto.push(&gerenciador->EstadoPronto,pid_process);
        }
        
    }
    else if(strcmp(inbuf,"I") == 0){
        write(pid, "Imprimindo ! - Comando I", 64);
        sleep(1);
        printf("\e[1;96m Tempo de CPU: %d\e[0m\n\n",gerenciador->cpu.fatia_tempo);
        imprimeProcessoCpu(*gerenciador);
        imprimeProntos(*gerenciador);
        imprimeBloqueados(*gerenciador);
        imprimeConcluidos(*gerenciador);
    }
    else if(strcmp(inbuf,"M") == 0){
        write(pid, "Imprimindo ! - Comando M", 64);
        sleep(1);
        printf("\e[1;96m Tempo de CPU: %d\e[0m\n\n",gerenciador->cpu.fatia_tempo);
        imprimeProcessoCpu(*gerenciador);
        imprimeProntos(*gerenciador);
        imprimeBloqueados(*gerenciador);
        imprimeConcluidos(*gerenciador);
        exit(0);
    }
}




Processo criaProcesso(int pid_pai, int pc, Dados dados, 
    int prioridade, int estado,int temp_inicio, int temp_cpu)
{
    Processo processo;
    processo.PID_PAI = pid_pai;
    processo.PC = pc;
    processo.dados = dados;
    processo.prioridade = prioridade;
    processo.estado = estado;
    processo.temp_inicio = temp_inicio;
    processo.temp_cpu = temp_cpu;
    processo.fatia_memoria = (int*)malloc(sizeof(int)*2);
    processo.fatia_memoria[0] = -1;
    processo.fatia_memoria[1] = -1;
    return processo;
}

void Impressao(int pid){
    char inbuf[32];
    while(1){
        read(pid, inbuf, 64);
        printf("\n\n ### %s ###\n\n", inbuf);
        sleep(0.1);
    }
}
void Controle(int pid){
    char opcao[10];
    char* msg1 = "Criei o Gerenciador !";
    write(pid, "init", 64);
    while(1){
        printf("  Modo de execução\n");
        printf("  1) Comandos\n");
        printf("  2) Arquivo\n");
        printf("  --> ");
        scanf("%s",opcao);
        if(strcmp(opcao,"1") == 0){
            while(1){
                printf(" Instrução: ");
                scanf("%s",opcao);
                write(pid, opcao, 64);
            }
        }
        if(strcmp(opcao,"2") == 0){
            while(1){
                printf(" Nome do arquivo: ");
                scanf("%s",opcao);
                write(pid, opcao, 64);
                break;
            } 
        }
        sleep(0.1);
    }
}



void Gerenciador(int pid_p){
    char* msg1 = "Criei o Impressao !";
    int p[2];

    if (pipe(p) < 0) exit(1);
    int pid = fork();
    char inbuf[32];

    Gerenciador_de_Processos gerenciador = iniciaGerenciador();

    if(pid==0){
        Impressao(p[0]);
    }
    while(1){
        read(pid_p, inbuf, 64);
        if(strcmp(inbuf,"init") == 0){
            Dados dado;
            int size;
            dado.instrucoes = get_Instrucoes(&size,"init.txt");
            Processo processo = criaProcesso(0,0,dado,0,0,0,0);
            int give_pid = gerenciador.adiciona(&gerenciador,processo);
            gerenciador.EstadoPronto.push(&gerenciador.EstadoPronto,give_pid);

        }
        else if(strcmp(inbuf,"U") == 0 || strcmp(inbuf,"M") == 0 || strcmp(inbuf,"L") == 0 || strcmp(inbuf,"I") == 0){
            enviaComando(p[1],inbuf, &gerenciador);
        }
        else{
            int size_C;
            char **comandos = get_Instrucoes(&size_C,inbuf);
            for(int i=0;i<size_C;i++){
                enviaComando(p[1],comandos[i], &gerenciador);
            }
        }
        sleep(1);
    } 
}

