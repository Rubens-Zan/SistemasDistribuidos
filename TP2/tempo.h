#ifndef __TEMPO__
#define __TEMPO__

#include <stdbool.h>
#include <stdio.h>


#define simulationTime 109.0 // tempo de duração da simulação
#define interval 10.0		 // intervalo de testes

// Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3
#define SPANNING_TREE 7

#define FAULT_TYPE 4
#define REPAIR_TYPE 5
#define MIN_SPANNING_TREE_TYPE 6

#define UNKNOWN_STATE -1
#define FAIL_STATE 1
#define CORRECT_STATE 0

/* descritor do processo */
typedef struct
{
	int id;
	int *stateVec;
} TipoProcesso;

TipoProcesso *processo;

// utilizado para armanezenar informações de um evento e ser possível imprimir na tela.
typedef struct
{
	bool novoEvento;
	bool detectado;
	float initTime;
	bool notFirstDetection;
	int processoIdentificador;
	int processoIdentificado;
	int TestRound;
	int newState;
	bool detected;
	int testNumberDetected;
} TipoEvento;

// armazena eventos que devem ocorrer
typedef struct
{
	int type;	 // Inteiro do tipo do evento
	real time;	 // momento em que ocorre evento.
	int process; // processo que sofre event0.
} TipoAgendaEvento;

// Declaração de métodos
void executeTest(TipoProcesso *processos, int n, int tokenID, bool started, int roundTest, TipoEvento *newEv);
void getDetection(TipoProcesso *processos, int n, int i, int j);
void getTestingList(TipoProcesso *processos, int n, int i, int s, int *nList);
bool checkEventFullDetection(TipoProcesso *processos, int n, int eventProcess, int timestamp);
bool checkProcessStartDetection(TipoProcesso *processos, int n);
void resetEvent(TipoEvento *newEv);

#endif