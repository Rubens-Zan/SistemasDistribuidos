/*
 * Autor: Rubens Laszlo
 * Disciplina: Sistemas Distribuídos
 * Data da última alteração: 01/05/2024
 * Tarefa 2: Cada processo sem falha testas sequencialmente até encontrar outro processo sem-falha.
 */
#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Vamos definir os EVENTOS
#define test 1
#define fault 2
#define recovery 3

// Vamos definir o descritor do processo

typedef struct
{
   int id; // identificador de facility do SMPL
   // outras variáveis locais dos processos são declaradas aqui!
} TipoProcesso;

TipoProcesso *processo;

#define ESTADO_CORRETO 0
#define ESTADO_DESCONHECIDO -1
#define ESTADO_FALHO 1

// Método realiza o teste dos processos considerando uma estrutura em anel
void testProcesses(TipoProcesso *processos, int n, int tokenID)
{
   // printf("tempo %4.1f - %d\n", time(),tokenID);
   int antecessor = tokenID;
   int next = (antecessor + 1) % n;
   int nextStatus = -1;

   // Testa-se o próximo nodo enquanto encontra processos com falhas.
   while (nextStatus != ESTADO_CORRETO)
   {
      nextStatus = status(processos[next].id);
      if (nextStatus == ESTADO_FALHO)
      { // próximo nó possui falha
         printf("Processo %02d testa Processo %02d -> Processo %02d está falho no tempo: %4.1f.\n", tokenID, next, next, time());
         antecessor++;
         next = (antecessor + 1) % n;
         nextStatus = status(processos[next].id);
      }
   }

   if (nextStatus == ESTADO_CORRETO && next == tokenID)
   { // próximo nó sem falha é ele mesmo
      printf("Processo %02d é o único correto no tempo %4.1f.\033[0m\n", tokenID, time());
   }
   else if (nextStatus == ESTADO_CORRETO)
   { // próximo nó sem falha
      printf("time: \Processo %02d testa processo %02d -> Processo %02d está correto no tempo: %4.1f \n", tokenID, next, next, time());
   }
}

int main(int argc, char *argv[])
{

   static int N, // número de processos
       token,    // indica o processo que estÃ¡ executando
       event, r, i,
       MaxTempoSimulac = 55;

   static char fa_name[5];

   if (argc != 2)
   {
      puts("Uso correto: tempo <número de processos>");
      exit(1);
   }

   N = atoi(argv[1]);

   smpl(0, "Um Exemplo de Simulação");
   reset();
   stream(1);

   // inicializar processos

   processo = (TipoProcesso *)malloc(sizeof(TipoProcesso) * N);

   for (i = 0; i < N; i++)
   {
      memset(fa_name, '\0', 5);
      sprintf(fa_name, "%d", i);
      processo[i].id = facility(fa_name, 1);
   }
   // vamos fazer o escalonamento inicial de eventos

   // nossos processos vão executar testes em intervalos de testes
   // o intervalo de testes vai ser de 30 unidades de tempo
   // a simulação começa no tempo 0 (zero) e vamos escalonar o primeiro teste de todos os
   //       processos para o tempo 30.0

   for (i = 0; i < N; i++)
   {
      schedule(test, 30.0, i);
   }
   schedule(fault, 20.0, 1);
   schedule(fault, 25.0, 2);
   schedule(fault, 28.0, 4);
   schedule(recovery, 39.0, 1);

   // agora vem o loop principal do simulador

   puts("===============================================================");
   puts("           Sistemas Distribuídos Prof. Elias");
   puts("          LOG do Trabalho Prático 0, Tarefa 3");
   puts("      Digitar, compilar e executar o programa tempo.c");
   printf("   Este programa foi executado para: N=%d processos.\n", N);
   printf("           Tempo Total de Simulação = %d\n", MaxTempoSimulac);
   puts("===============================================================");

   while (time() < MaxTempoSimulac)
   {
      cause(&event, &token);
      switch (event)
      {
      case test:
         if (status(processo[token].id) != 0)
            break; // se o processo está falho, não testa!
         printf("************************\n");
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
   }    // while
} // tempo.c