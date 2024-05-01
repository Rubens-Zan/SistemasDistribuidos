/*
 * Autor: Rubens Laszlo
 * Disciplina: Sistemas Distribuídos
 * Tarefa 1: Cada processos correto testa o seguinte no anel.
 * Data da última alteração: 1/05/2024
 */
#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Vamos definir os EVENTOS 
#define test 1
#define fault 2
#define recovery 3

// Vamos definir o descritor do processo

typedef struct{ 
   int id;            // identificador de facility do SMPL
   // outras variáveis locais dos processos são declaradas aqui!
} TipoProcesso;

TipoProcesso *processo;

//Método realiza o teste dos processos considerando uma estrutura em anel
void testProcesses(TipoProcesso *processos, int n, int tokenID){
// test next process
int next = (tokenID	+ 1) % n; //armazena id do nodo a ser testado
	int nextStatus = status(processos[next].id); // realiza o teste

	if(nextStatus == 0){ //próximo nó faulty-free
		printf("time: %4.1f\tNode %02d performs test in Node %02d\t\033[0;34;107mNode %02d is fault-free\033[0m.\n",time(), tokenID, next, next);
	}
	if(nextStatus == 1){//próximo nó faulty
		printf("time: %4.1f\tNode %02d performs test in Node %02d\t\033[0;31;107mNode %02d is faulty\033[0m.\n", time(), tokenID, next, next);
	}


}

int main (int argc, char *argv[]) {
   
   static int N,   // número de processos
            token,  // indica o processo que está executando
            event, r, i,
            MaxTempoSimulac = 150;

   static char fa_name[5];

   if (argc != 2) {
       puts("Uso correto: tempo <número de processos>");
       exit(1);
    }

   N = atoi(argv[1]);

   smpl(0, "Um Exemplo de Simulação");
   reset();
   stream(1);

   // inicializar processos

   processo = (TipoProcesso *) malloc(sizeof(TipoProcesso)*N);

   for (i=0; i<N; i++) {
      memset(fa_name, '\0', 5);
      sprintf(fa_name, "%d", i);
      processo[i].id = facility(fa_name,1);
   }
    // vamos fazer o escalonamento inicial de eventos

    // nossos processos vão executar testes em intervalos de testes
    // o intervalo de testes vai ser de 30 unidades de tempo
    // a simulação começa no tempo 0 (zero) e vamos escalonar o primeiro teste de todos os    
    //       processos para o tempo 30.0

    for (i=0; i<N; i++) {
       schedule(test, 30.0, i); 
    }
    schedule(fault, 31.0, 1);
    schedule(recovery, 61.0, 1);

    // agora vem o loop principal do simulador

    puts("===============================================================");
    puts("           Sistemas Distribuídos Prof. Elias");
    puts("          LOG do Trabalho Prático 0, Tarefa 1");
    puts("      Digitar, compilar e executar o programa tempo.c");
    printf("   Este programa foi executado para: N=%d processos.\n", N); 
    printf("           Tempo Total de Simulação = %d\n", MaxTempoSimulac);
    puts("===============================================================");

    while(time() < 150.0) {
         cause(&event, &token);
         switch(event) {
           case test: 
               if (status(processo[token].id) !=0) break; // se o processo está falho, não testa!
               printf("Processo %d: estou testando no tempo %4.1f\n", token, time());
               testProcesses(processo, N, token);

               schedule(test, 30.0, token);
               break;
           case fault:
               r = request(processo[token].id, token, 0);
               printf("Socooorro!!! Sou o processo %d  e estou falhando no tempo %4.1f\n", token, time());
               break;
           case recovery:
               release(processo[token].id, token);
               printf("Viva!!! Sou o processo %d e acabo de recuperar no tempo %4.1f\n", token, time());
               schedule(test, 1.0, token);
               break;
          } // switch
    } // while
} // tempo.c