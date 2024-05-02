/*
 * Autor: Rubens Laszlo
 * Disciplina: Sistemas Distribuídos
 * Data da última alteração: 08/04/2024
 * Tarefa 3: Cada processo mantém localmente o vetor state e atualiza seus valores ao testar o processo (só atualizada entrada corresponde ao processo em que testou)
 */
#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Vamos definir os EVENTOS
#define test 1
#define fault 2
#define recovery 3

#define ESTADO_CORRETO 0
#define ESTADO_DESCONHECIDO -1
#define ESTADO_FALHO 1

// Vamos definir o descritor do processo

typedef struct
{
	int id; // identificador de facility do SMPL
			// outras variáveis locais dos processos sÃ£o declaradas aqui!
	int *state;
} TipoProcesso;

TipoProcesso *processo;

typedef struct
{
	int size;	  // slots used so far
	int capacity; // total available slots
	int *data;	  // array of integers we're storing
} failVector;

// Imprime na tela vetor status
void printStatusVec(TipoProcesso *processo, int n, int tokenID)
{
	int it;
	int stat;
	printf("[");

	for (it = 0; it < n; ++it)
	{
		stat = processo[tokenID].state[it];
		if (stat == ESTADO_DESCONHECIDO)
		{
			printf("%d ", stat);
		}
		if (stat == ESTADO_CORRETO)
		{
			printf("%d ", stat);
		}

		if (stat == ESTADO_FALHO)
		{
			printf("%d ", stat);
		}
	}
	printf(" ]\n");
}

void getStatus(TipoProcesso *processos, int N, int tester, int toTest, int *failV, int qtdfailnodes)
{
	int i = 0;
	for (i = 0; i < N; ++i)
	{
		if (i != tester && i != toTest)
		{
			processo[tester].state[i] = processo[toTest].state[i];
		}
	}

	for (i = 0; i < qtdfailnodes; ++i)
	{
		processo[tester].state[failV[i]] = ESTADO_FALHO;
	}
}

// Método realiza o teste dos processos considerando uma estrutura em anel
void testProcesses(TipoProcesso *processos, int n, int tokenID)
{
	int antecessor = tokenID;
	int next = (antecessor + 1) % n;
	int nextStatus = ESTADO_DESCONHECIDO;
	int failVector[n];
	int failnodes = 0;

	while (nextStatus != ESTADO_CORRETO)
	{
		nextStatus = status(processos[next].id);
		if (nextStatus == ESTADO_FALHO)
		{ // próximo nó possui falha
			printf("Processo %02d testa Processo %02d -> Processo %02d está falho no tempo: %4.1f.\n", tokenID, next, next, time());
			// atualiza state

			processo[tokenID].state[next] = ESTADO_FALHO;
			failVector[failnodes] = next;
			failnodes++;

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
		// atualiza state
		getStatus(processos, n, tokenID, next, failVector, failnodes);
		processo[tokenID].state[next] = ESTADO_CORRETO;
		printf("Processo %02d testa Processo %02d no tempo %4.1f-> Processo %02d vetor de estados: ", tokenID, next, time(), tokenID);
		printStatusVec(processo, n, tokenID);
	}
}

int main(int argc, char *argv[])
{

	static int N, // número de processos
		token,	  // indica o processo que estÃ¡ executando
		event, r, i, j,
		MaxTempoSimulac = 100;

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

		// Inicialização do vetor de estados
		processo[i].state = malloc(N * sizeof(int));
		for (j = 0; j < N; ++j)
		{
			processo[i].state[j] = ESTADO_DESCONHECIDO;
		}
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

	schedule(fault, 31.0, 3);
	schedule(fault, 31.0, 4);
	schedule(fault, 40.0, 5);
	schedule(fault, 45.0, 0);
	schedule(recovery, 70, 4);
	schedule(recovery, 61.0, 3);
	schedule(recovery, 62.0, 0);
	schedule(recovery, 80.0, 5);

	// agora vem o loop principal do simulador

	puts("===============================================================");
	puts("           Sistemas Distribuídos Prof. Elias");
	puts("          LOG do Trabalho Prático 0, Tarefa 4");
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
	}	  // while
} // tempo.c