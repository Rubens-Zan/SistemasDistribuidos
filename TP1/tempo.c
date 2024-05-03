/*
 * Autor: Rubens Laszlo
 * Disciplina: Sistemas Distribuídos
 * Data da última alteração: 01/05/2024
 * TP1: Implementação do algoritmo VCube2 no ambiente de simulação SMPL
 */

#include <math.h>
#include "smpl.h"
#include <stdbool.h>
#include "cisj.c"

#define simulationTime 201.0 // tempo de duração da simulação
#define interval 10.0		 // intervalo de testes

// Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3

#define FAULT_TYPE 4
#define REPAIR_TYPE 5

#define UNKNOWN_STATE -1
#define FAIL_STATE 1
#define CORRECT_STATE 1


/* descritor do processo */
typedef struct
{
	int id;
	int *timestamp;
} TipoProcesso;

TipoProcesso *processo;

// utilizado para armanezenar informações de um evento e ser possível imprimir na tela.
typedef struct
{
	bool novoEvento;
	bool detectado;
	float initTime;
	bool impresso;
	int processoIdentificador;
	int processoIdentificado;
	int TestRound;
	int novoEstado;
	bool diagnosed;
	int testNumberDiagnosed;
} TipoEvento;

// armazena eventos que devem ocorrer
typedef struct
{
	int type;	 // f para falho e r para recover.
	real time;	 // momento em que ocorre evento.
	int process; // processo que sofre event0.
} TipoAgendaEvento;

int testnumber = 1;		 // armazena número do teste realizado
int eventTestnumber = 0; // armazena o identificado do teste no momento em que um evento é diagnosticado

// Declaração de métodos
void executeTest(TipoProcesso *processos, int n, int tokenID, bool started, int roundTest, TipoEvento *newEv);
void printStatus(TipoProcesso *processos, int n, int tokenID);
void printLogIntro(int n, TipoAgendaEvento *eventos, int numEventos);

void getDiagnosisInfo(TipoProcesso *processos, int n, int i, int j);
void whoIShouldTest(TipoProcesso *processos, int n, int i, int s, int *nList);
bool checkEventDiagnosed(TipoProcesso *processos, int n, int eventProcess, int timestamp);
bool checkProcessStartDiagnosed(TipoProcesso *processos, int n);
void restartEvent(TipoEvento *newEv);

// reinicia/inicia váriavel que armazena informações sobre um evento
void restartEvent(TipoEvento *newEv)
{
	newEv->novoEvento = false;
	newEv->detectado = false;
	newEv->impresso = false;
	newEv->processoIdentificador = UNKNOWN_STATE;
	newEv->processoIdentificado = UNKNOWN_STATE;
	newEv->TestRound = UNKNOWN_STATE;
	newEv->novoEstado = UNKNOWN_STATE;
	newEv->diagnosed = false;
	newEv->testNumberDiagnosed = 0;
	newEv->initTime = time();
}

// verifica se todos processos diagnosticaram a inicialização sem falhas de todos processos do sistema. Apenas após esta verificação devem ocorrer eventos.
bool checkProcessStartDiagnosed(TipoProcesso *processos, int n)
{
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			if (processos[i].timestamp[j] != 0)
			{
				return false;
			}
		}
	}
	return true;
}

// verifica se todos processos do sistema diagnosticam um evento
bool checkEventDiagnosed(TipoProcesso *processos, int n, int eventProcess, int timestamp)
{
	int it;
	for (it = 0; it < n; it++)
	{
		if (status(processos[it].id) == 0)
		{ // considera-se apenas processos livre de falha
			if (processos[it].timestamp[eventProcess] != timestamp)
			{
				return (false);
			}
		}
	}
	return (true);
}

// imprime as informações iniciais de log
void printLogIntro(int n, TipoAgendaEvento *eventos, int numEventos)
{
	printf("******************************************************************************************************************************************\n");
	printf("Log Algoritmo VCube versão 2 \n");
	printf("Autor: Rubens Laszlo \n");
	printf("Tempo de simulação: %4.1f segundos\n", simulationTime);
	printf("Número de processos: %d\n", n);
	printf("Intervalo de testes: %4.1f segundos\n", interval);
	
	printf("\nEstado dos processos: \n");
	printf("Estado Desconhecido: %d \n", UNKNOWN_STATE);
	printf("Estado Desconhecido: %d \n", FAIL_STATE);
	printf("Estado Desconhecido: %d \n", CORRECT_STATE);

	printf("\nEventos agendados: \n");
	int it;
	for (it = 0; it < numEventos; it++)
	{
		if (eventos[it].type == FAULT_TYPE)
		{
			printf("Processo %02d falhar no tempo %4.1f \n", eventos[it].process, eventos[it].time);
		}
		else if (eventos[it].type == REPAIR_TYPE)
		{
			printf("Processo %02d recuperação no tempo %4.1f\n", eventos[it].process, eventos[it].time);
		}
	}
	
	printf("******************************************************************************************************************************************\n\n");
	printf("Tempo \t\tNúmero do teste \tEvento de teste \t\t\t\tStatus do processo \t\t\t\t\t Observação \n");
}

// Método retorna quem o processo i deve testar
//  i = processo testador; s = cluster
void whoIShouldTest(TipoProcesso *processos, int n, int i, int s, int *nList)
{
	int alvoTest;
	int k, it, j, iesimoJ;
	for (it = 0; it < n; it++)
	{
		nList[it] = 0; // é necessário zerar vetor;
	}
	node_set *nodesCij; // armazena processos obtidos a partir da função cisj
	node_set *nodesCjs;
	for (s = 1; s <= log2(n); s++)
	{ // clusters

		// Antes de i executar um teste em j pertencente a Cis, ele checa se é o primeiro livre de erro de 	Cjs
		nodesCij = cis(i, s);
		for (k = 0; k < POW_2(s - 1); k++)
		{ // k -> iteração dos valores de j
			j = nodesCij->nodes[k];
			nodesCjs = cis(j, s);
			// bool firstfaultfree = false;

			for (it = 0; it < POW_2(s - 1); it++)
			{
				iesimoJ = nodesCjs->nodes[it];
				if (processos[i].timestamp[iesimoJ] % 2 == 0 || processos[i].timestamp[iesimoJ] == UNKNOWN_STATE || iesimoJ == i)
				{ // se livre de erro ou desconhecido
					if (iesimoJ == i)
					{
						nList[j] = 1;
					}
					break;
				}
			}
		}
	}
}

// Realiza testes de acordo com o definido pelo algoritmo VCube
void executeTest(TipoProcesso *processos, int n, int tokenID, bool started, int roundTest, TipoEvento *newEv)
{
	int it;
	int i = tokenID;	// indice do processo tester;
	int nodesTotest[n]; // lista de processos que o processo i deve testar
	int j;				// processo a ser testado/
	int jStatus;		// estado de j após test

	int s;											 // define cluster id;
	whoIShouldTest(processos, n, i, s, nodesTotest); // esta função armazena o valor 1 nas posições que indicam necessidade de teste no vetor nodesTotest

	for (j = 0; j < n; j++)
	{
		if (nodesTotest[j] == 1)
		{
			jStatus = status(processos[j].id);
			if (jStatus == 0)
			{ // j é correto
				printf("Tempo:%4.1f\ttn:%03d\t\tProcesso %02d testa processo %02d: CORRETO.", time(), testnumber, i, j);

				if (processos[i].timestamp[j] % 2 == 1)
				{ // Se estado anterior correspondia a falha, atualiza timestamp
					processos[i].timestamp[j]++;
					if (started && newEv->novoEvento == false)
					{
						newEv->novoEvento = true;
						newEv->processoIdentificador = i;
						newEv->processoIdentificado = j;
						newEv->novoEstado = processos[i].timestamp[j];
						newEv->detectado = true;
						newEv->TestRound = roundTest;
						newEv->impresso = false;
						newEv->diagnosed = false;
					}
				}
				testnumber++;
				getDiagnosisInfo(processos, n, i, j); // processo testado possui informações recentes, então atualiza seu vetor de estado (timestamp)
				printf("\tProcess %02d status: ", i);
				printStatus(processos, n, tokenID);
				if (started && newEv->novoEvento && !newEv->impresso)
				{ // se é um novo evento e ainda não foi impresso na tela, deve imprimir
					printf("\tDetectado evento de recuperação detectado pelo primeiro processo.");
					newEv->impresso = true;
				}
			}
			else
			{
				printf("Tempo:%4.1f\ttn:%03d\t\tProcesso %02d testa processo %02d: FALHO.", time(), testnumber, i, j);
				testnumber++;
				printf("\t\tProcesso %02d status: ", i);

				if (processos[i].timestamp[j] % 2 == 0)
				{
					processos[i].timestamp[j]++;
					if (started && newEv->novoEvento == false)
					{
						newEv->novoEvento = true;
						newEv->processoIdentificador = i;
						newEv->processoIdentificado = j;
						newEv->novoEstado = processos[i].timestamp[j];
						newEv->detectado = true;
						newEv->TestRound = roundTest;
						newEv->impresso = false;
						newEv->diagnosed = false;
					}
				}
				printStatus(processos, n, tokenID);
				if (started && newEv->novoEvento && !newEv->impresso)
				{
					printf("\tFailure event detected for the first time.");
					newEv->impresso = true;
				}
			}

			// testa se evento foi diagnosticado
			if (checkEventDiagnosed(processos, n, newEv->processoIdentificado, newEv->novoEstado) && newEv->diagnosed == false)
			{
				printf("\tDiagnosis complete.");
				newEv->diagnosed = true;
				newEv->testNumberDiagnosed = testnumber - 1;
			}

			printf("\n");
		}
	}
}

// Imprime na tela vetor status
void printStatus(TipoProcesso *processos, int n, int tokenID)
{
	int it;
	int stat;
	printf("[");
	for (it = 0; it < n; ++it)
	{
		stat = processo[tokenID].timestamp[it];
		if (stat == UNKNOWN_STATE)
		{
			printf("%3d ", stat);
		}
		else if (stat % 2 == 0)
			printf("%3d ", stat);
		else
			printf("%3d ", stat);
	}
	printf("  ]");
}

// Obtém diagnóstico a partir do processo j correto
void getDiagnosisInfo(TipoProcesso *processos, int n, int i, int j)
{
	int it;
	for (it = 0; it < n; it++)
	{
		if (processos[j].timestamp[it] > processos[i].timestamp[it])
		{ // Se informação obtida ao testar é mais recente, atualiza timestamp
			processos[i].timestamp[it] = processos[j].timestamp[it];
		}
	}
}

int main(int argc, char *argv[])
{
	static int N, // Número de processos
		token, event, r, it, j, tr, t;
	tr = 1;											   // indica a rodada de testes;
	t = UNKNOWN_STATE;											   // utilizada para verificar se houve mudança no tempo de execução e imprimir mensagem de nova rodada de testes
	int maxTestround = (int)simulationTime / interval; // Os testes são executados a cada 10 segundos, portanto é possível obter a quantidade máxima de rodadas de testes em função do tempo de simulação
	TipoEvento newEvento;							   // armazena informaçoes sobre um novo evento detectado
	restartEvent(&newEvento);						   // esta função inicializa/reinicializa strutura de armazenamento de eventos.
	bool nodesStarted = false;
	int qtdTest[maxTestround]; // armazena quantidade de testes por rodada.
	int iTest = UNKNOWN_STATE;
	static char fa_name[5];

	if (argc != 2)
	{
		puts("Wrong input, use: ./tempo <number_of_nodes>");
		exit(1);
	}

	N = atoi(argv[1]);
	smpl(0, "VCube");
	reset();
	stream(1);

	// Inicializacao
	processo = (TipoProcesso *)malloc(sizeof(TipoProcesso) * N);

	for (it = 0; it < N; ++it)
	{
		memset(fa_name, '\0', 5);
		sprintf(fa_name, "%d", it);
		processo[it].id = facility(fa_name, 1);

		// Inicialização do timestamp de cada nó.
		processo[it].timestamp = malloc(N * sizeof(int)); // aloca espaço para vetor que armazena estado dos demais processos;
		for (j = 0; j < N; ++j)
		{
			if (it == j)
			{
				processo[it].timestamp[j] = 0; //
			}
			else
			{
				processo[it].timestamp[j] = UNKNOWN_STATE; //
			}
		}
	}

	// Schedule de eventos
	for (it = 0; it < N; it++)
	{
		schedule(TEST, interval, it);
	}

	/*----------------------------------------------------------------------------------------------------------------------*/
	// Atenção: Aqui devem ser inseridos os eventos que devem ocorrer. Atenção ao inserir número de eventos e indices.
	int numeroDeEventos = 4;

	TipoAgendaEvento *sEvent;
	sEvent = (TipoAgendaEvento *)malloc(sizeof(TipoAgendaEvento) * numeroDeEventos);

	sEvent[0].type = FAULT_TYPE;
	sEvent[0].time = 42.0;
	sEvent[0].process = 0;
	sEvent[1].type = FAULT_TYPE;
	sEvent[1].time = 73.0;
	sEvent[1].process = 3;

	sEvent[2].type = REPAIR_TYPE;
	sEvent[2].time = 105.0;
	sEvent[2].process = 0;
	sEvent[3].type = REPAIR_TYPE;
	sEvent[3].time = 138.0;
	sEvent[3].process = 3;


	for (it = 0; it < numeroDeEventos; it++)
	{
		if (sEvent[it].type == FAULT_TYPE)
		{
			schedule(FAULT, sEvent[it].time, sEvent[it].process);
		}
		else if (sEvent[it].type == REPAIR_TYPE)
		{
			schedule(REPAIR, sEvent[it].time, sEvent[it].process);
		}
	}

	printLogIntro(N, sEvent, numeroDeEventos); // imprime na tela cabeçalho do log


	while (time() < simulationTime)
	{
		cause(&event, &token);
		switch (event)
		{
		case TEST:
			if (token == 0)
			{
				iTest++; // Se o tester equivale ao processo zero, considera-se nova rodada de testes;
			}

			if (time() > simulationTime)
			{
				break;
			}
			if (t < time())
			{
				printf("\n************************************************ Start test round %02d ************************************************\n", tr);
				t = time();
				tr++;
			}
			if (status(processo[token].id) != 0)
			{
				printf("time:%4.1f\t\t\tProcess %02d está falho.\n", time(), token);
			}
			else
			{
				executeTest(processo, N, token, nodesStarted, tr, &newEvento);
			}

			schedule(TEST, interval, token);

			if (token == N - 1)
			{
				printf("**********************************************End of the test round %02d **********************************************\n\n", iTest + 1);
				if (nodesStarted == false)
				{
					nodesStarted = checkProcessStartDiagnosed(processo, N);
				}
				else
				{
					if (newEvento.novoEvento)
					{ // é necessário verificar se evento já foi diagnosticado
						if (checkEventDiagnosed(processo, N, newEvento.processoIdentificado, newEvento.novoEstado))
						{ // se evento foi diagnosticado

							printf("Test round report:\n");
							if (newEvento.novoEstado % 2 == 0)
							{ // evento de recover

								printf("\t\tDiagnosis complete: process %02d recuperado.\n", newEvento.processoIdentificado);
							}
							else
							{
								printf("\t\tDiagnosis complete: process %02d falho.\n", newEvento.processoIdentificado);
							}

							printf("\t\Latência: %02d round(s).\n", tr + 1 - newEvento.TestRound);
							printf("\t\Tempo de detecção: %4.1f s.\n", time() - newEvento.initTime);

							printf("\t\tTotal de testes: %02d.\n", newEvento.testNumberDiagnosed - eventTestnumber);
							printf("\n");

							restartEvent(&newEvento); // reinicia variavel de armazena evento;
						}
						else
						{
						}
					}
				}
			}

			break;

		case FAULT:
			r = request(processo[token].id, token, 0);
			if (r != 0)
			{
				puts("Não foi possível falhar o processo");
				exit(1);
			}
			printf(" Tempo:%4.1f\t\t\tProcesso %02d CORRETO.\n", time(), token);
			eventTestnumber = testnumber - 1;
			break;

		case REPAIR:
			release(processo[token].id, token);
			printf("Tempo: %4.1f\t\t\tProcesso %02d recuperado. \t\t\tProcess %02d status: ", time(), token, token);
			for (it = 0; it < N; it++)
			{
				if (it == token)
				{
					processo[token].timestamp[it] = 0;
				}
				else
				{
					processo[token].timestamp[it] = UNKNOWN_STATE;
				}
			}
			printStatus(processo, N, token);
			eventTestnumber = testnumber - 1;
			break;
		}
	}
}
