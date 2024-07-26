/*
 * Autor: Rubens Laszlo
 * Disciplina: Sistemas Distribuídos
 * Data da última alteração: 01/05/2024
 * TP1: Implementação do algoritmo VCube2 no ambiente de simulação SMPL
 */

#define VERBOSE 0

#include <math.h>
#include "smpl.h"
#include <stdbool.h>
#include "cisj.c"
#include "tempo.h"

// armazena número do teste realizado
int testnumber = 1;		 
// Ao detectar um evento, armazena o identificador do teste 
int eventTestnumber = 0; 

// reinicia/inicia váriavel que armazena informações sobre um evento
void resetEvent(TipoEvento *newEv)
{
	newEv->novoEvento = false;
	newEv->detectado = false;
	newEv->notFirstDetection = false;
	newEv->processoIdentificador = UNKNOWN_STATE;
	newEv->processoIdentificado = UNKNOWN_STATE;
	newEv->TestRound = UNKNOWN_STATE;
	newEv->newState = UNKNOWN_STATE;
	newEv->detected = false;
	newEv->testNumberDetected = 0;
	newEv->initTime = time();
}

// verifica se todos processos detectaram a inicialização sem falhas de todos processos do sistema. Apenas após esta verificação devem ocorrer eventos.
bool checkProcessStartDetection(TipoProcesso *processos, int n)
{
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			if (processos[i].stateVec[j] != 0)
			{
				return false;
			}
		}
	}
	return true;
}

// verifica se todos processos do sistema detectaram um evento
bool checkEventFullDetection(TipoProcesso *processos, int n, int eventProcess, int curStateTested)
{
	int it;
	for (it = 0; it < n; it++)
	{
		if (status(processos[it].id) == 0)
		{ // considera-se apenas processos livre de falha
			if (processos[it].stateVec[eventProcess] != curStateTested)
			{
				return (false);
			}
		}
	}
	return (true);
}

// imprime as informações iniciais de log
void printLogHeader(int n, TipoAgendaEvento *eventos, int numEventos)
{
	printf("******************************************************************************************************************************************\n");
	printf("Log Algoritmo VCube versão 2 \n");
	printf("Autor: Rubens Laszlo \n");
	printf("Tempo de simulação: %4.1f segundos\n", simulationTime);
	printf("Número de processos: %d\n", n);
	printf("Intervalo de testes: %4.1f segundos\n", interval);

	printf("\nEstado dos processos: \n");
	printf("Estado Desconhecido: %d \n", UNKNOWN_STATE);
	printf("Estado Correto: %d \n", CORRECT_STATE);
	printf("Estado Falho: %d \n", FAIL_STATE);

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

// Imprime na tela vetor status
void printStatusVec(TipoProcesso *processos, int n, int tokenID)
{
	int it;
	int stat;
	printf("[");
	for (it = 0; it < n; ++it)
	{
		stat = processo[tokenID].stateVec[it];
		if (stat == UNKNOWN_STATE)
		{
			printf("\033[0;33m%3d \033[0m", stat);
		}
		else if (stat % 2 == CORRECT_STATE)
			printf("\033[0;32m%3d \033[0m", stat);
		else
			printf("\033[0;31m%3d \033[0m", stat);
	}
	printf("  ]");
}


// Método retorna quem o processo i deve testar
//  i = processo testador; s = cluster
void getTestingList(TipoProcesso *processos, int n, int i, int s, int *nList)
{
	int alvoTest;
	int k, it, j, iesimoJ;
	for (it = 0; it < n; it++)
	{
		nList[it] = 0; // é necessário zerar vetor;
	}
	
	// printf("\n\n\n PROCESSO: %d", i); 

	node_set *nodesCij; // armazena processos obtidos a partir da função cisj
	node_set *nodesCjs;
	for (s = 1; s <= log2(n); s++)
	{ // clusters

		// Antes de i executar um teste em j pertencente a Cis, ele checa se é o primeiro livre de erro de 	Cjs
		nodesCij = cis(i, s);
		// printf("\n CLUSTER: %d \n", s); 

		for (k = 0; k < POW_2(s - 1); k++)
		{ // k -> iteração dos valores de j
			j = nodesCij->nodes[k];
			nodesCjs = cis(j, s);

			for (it = 0; it < POW_2(s - 1); it++)
			{
				iesimoJ = nodesCjs->nodes[it];
				if (processos[i].stateVec[iesimoJ] % 2 == 0 || processos[i].stateVec[iesimoJ] == UNKNOWN_STATE || iesimoJ == i)
				{ 
					// se livre de erro ou desconhecido
					if (iesimoJ == i)
					{
						// printf("%d ", j);
						nList[j] = 1;

					}
					break;
				}
			}
		}
	}

	// printf("\n\n\n"); 
}


void getTestingListInOrder(TipoProcesso *processos, int n, int i, int s, int *nList)
{
	int alvoTest;
	int k, it, j, iesimoJ;
	for (it = 0; it < n; it++)
	{
		nList[it] = 0; // é necessário zerar vetor;
	}
	
	int childsNodesOrder = 0;
	// printf("\n\n\n PROCESSO: %d", i); 

	node_set *nodesCij; // armazena processos obtidos a partir da função cisj
	node_set *nodesCjs;
	for (s = 1; s <= log2(n); s++)
	{ // clusters

		// Antes de i executar um teste em j pertencente a Cis, ele checa se é o primeiro livre de erro de 	Cjs
		nodesCij = cis(i, s);
		// printf("\n CLUSTER: %d \n", s); 

		for (k = 0; k < POW_2(s - 1); k++)
		{ // k -> iteração dos valores de j
			j = nodesCij->nodes[k];
			nodesCjs = cis(j, s);

			for (it = 0; it < POW_2(s - 1); it++)
			{
				iesimoJ = nodesCjs->nodes[it];
				if (processos[i].stateVec[iesimoJ] % 2 == 0 || processos[i].stateVec[iesimoJ] == UNKNOWN_STATE || iesimoJ == i)
				{ 
					// se livre de erro ou desconhecido
					if (iesimoJ == i)
					{
						// printf("%d ", j);
						nList[childsNodesOrder] = j;
						childsNodesOrder++;
					}
					break;
				}
			}
		}
	}

	// printf("\n\n\n"); 
}


// Realiza testes de acordo com o definido pelo algoritmo VCube
void executeTest(TipoProcesso *processos, int n, int tokenID, bool started, int roundTest, TipoEvento *newEv)
{
	int it;
	int i = tokenID;	// indice do processo tester;
	int procsToTest[n]; // lista de processos que o processo i deve testar
	int j;				// processo a ser testado/
	int jStatus;		// estado de j após test

	int s;											 // define cluster id;
	getTestingList(processos, n, i, s, procsToTest); // esta função armazena o valor 1 nas posições que indicam necessidade de teste no vetor procsToTest
	
	for (j = 0; j < n; j++)
	{
		if (procsToTest[j] == 1)
		{
			jStatus = status(processos[j].id);
			if (jStatus == CORRECT_STATE)
			{ 
				printf("Tempo:%4.1f\tN_Teste:%03d\t\tProcesso %02d testa processo %02d: CORRETO.", time(), testnumber, i, j);

				if (processos[i].stateVec[j] % 2 == FAIL_STATE)
				{ // Se estado anterior correspondia a falha, atualiza para que seja testado conforme novidade
					processos[i].stateVec[j]++;
					if (started && !newEv->novoEvento)
					{
						newEv->novoEvento = true;
						newEv->processoIdentificador = i;
						newEv->processoIdentificado = j;
						newEv->detectado = true;
						newEv->newState = processos[i].stateVec[j];
						newEv->TestRound = roundTest;
						newEv->notFirstDetection = false;
						newEv->detected = false;
					}
				}
				testnumber++;
				for (int it = 0; it < n; it++)
				{	
					if (processos[j].stateVec[it] > processos[i].stateVec[it])
					{ 
					// Se o valor da entrada for maior no vetor STATE do processo testado, então copia a informação
						processos[i].stateVec[it] = processos[j].stateVec[it];
					}
				}

				printf("\t\tProcesso %02d status: ", i);
				printStatusVec(processos, n, tokenID);
				if (started && newEv->novoEvento && !newEv->notFirstDetection)
				{ // se é um novo evento deve imprimir o evento
					printf("\tDetectado evento de recuperação pela primeira vez.");
					newEv->notFirstDetection = true;
				}
			}
			else
			{
				printf("Tempo:%4.1f\tN_Teste:%03d\t\tProcesso %02d testa processo %02d: FALHO.", time(), testnumber, i, j);
				testnumber++;
				printf("\t\tProcesso %02d status: ", i);

				if (processos[i].stateVec[j] % 2 == 0)
				{
					processos[i].stateVec[j]++;
					if (started && !newEv->novoEvento)
					{
						newEv->novoEvento = true;
						newEv->detectado = true;
						newEv->notFirstDetection = false;
						newEv->detected = false;
						newEv->processoIdentificador = i;
						newEv->processoIdentificado = j;
						newEv->newState = processos[i].stateVec[j];
						newEv->TestRound = roundTest;
					}
				}
				printStatusVec(processos, n, tokenID);
				if (started && newEv->novoEvento && !newEv->notFirstDetection)
				{
					printf("\tEvento de falha detectado pela primeira vez.");
					newEv->notFirstDetection = true;
				}
			}

			// testa se evento foi detectado
			if (checkEventFullDetection(processos, n, newEv->processoIdentificado, newEv->newState) && !newEv->detected )
			{
				printf("\t Detecção completa evento no processo %d", newEv->processoIdentificado);
				newEv->detected = true;
				newEv->testNumberDetected = testnumber - 1;
			}

			printf("\n");
		}
	}
}

void printSpanningAutonomicTree(TipoProcesso *processos, int n, int tokenID,  int nChilds, int *printedNodes, int isRootNode)
{
	int it;
	int index = tokenID;	// indice do processo tester;
	int procsToTest[n];     // lista de processos que o processo i deve testar

	int jStatus;		    // estado de j após test
	
	int testedChilds = 0; 
	int printedChilds[n]; 
	for (int i=0;i< n;++i)
		printedChilds[i] = -1; 

	int s;											 // define cluster id;

	if (VERBOSE == 1){
		if (!isRootNode)
			printf("\n (PROCESSO %d , S=%d): Nodos->\t ", index, nChilds+1);
		else 
			printf("\n (PROCESSO %d , S=%d): Nodos->\t ", index, nChilds);

	}
				
	if (nChilds == 1){
		printf("\n");
	}
	if (nChilds > 0 ){
		for (int curS = 1; curS <= nChilds ; curS++)
		{
			node_set *childList = cis(tokenID, curS);
			for (int i = 0; i < childList->size; i++) {
				int child = childList->nodes[i];
				
				jStatus = status(processos[child].id);

				if (jStatus == CORRECT_STATE)
				{ 
					if ( printedNodes[child] == 0 ){
						printedChilds[curS] = child;
						printf("%d ", child ); 
						printedNodes[child]  = 1; 
						
						testedChilds++; 
					}
					break; 
				} else if (i+1 == childList->size ) {
					if (VERBOSE)
						("_ ");
				}
			}
		}
		for (int sChild=1; sChild <=  nChilds; sChild++){
			if (printedChilds[sChild] != -1 ){
				// printf("{%d}",printedChilds[sChild]); 
				printSpanningAutonomicTree(processos, n, printedChilds[sChild], sChild - 1 , printedNodes, 0); 
			}else {
				// printf("[%d]",printedChilds[sChild]); 

			}
		}
	}
}



int main(int argc, char *argv[])
{
	static int N, // Número de processos
		token, event, r, it, j, tr, t;
	// indica a rodada de testes;
	tr = 1;											   
	
	// utilizada para verificar se houve mudança no tempo de execução e imprimir mensagem de nova rodada de testes
	t = UNKNOWN_STATE;								   
	// Os testes são executados a cada 10 segundos, portanto é possível obter a quantidade máxima de rodadas de testes em função do tempo de simulação
	int maxTestround = (int)simulationTime / interval;
	// armazena informaçoes sobre um novo evento detectado
	TipoEvento newEvento;							   
	// inicializa estrutura de armazenamento de eventos.
	resetEvent(&newEvento);						   
	bool procStarted = false;
	// armazena qtd de testes por rodada.
	int qtdTest[maxTestround];
	int iTest = UNKNOWN_STATE;
	static char fa_name[5];

	if (argc != 2)
	{
		puts("Wrong input, use: ./tempo <number_of_nodes>");
		exit(1);
	}

	N = atoi(argv[1]);
	smpl(0, "VCube V2");
	reset();
	stream(1);

	// Inicializacao
	processo = (TipoProcesso *)malloc(sizeof(TipoProcesso) * N);

	for (it = 0; it < N; ++it)
	{
		memset(fa_name, '\0', 5);
		sprintf(fa_name, "%d", it);
		processo[it].id = facility(fa_name, 1);

		// Inicialização de checagem de estados dos processos de cada processo.
		processo[it].stateVec = malloc(N * sizeof(int)); // aloca espaço para vetor que armazena estado dos demais processos;
		for (j = 0; j < N; ++j)
		{
			if (it == j)
			{
				processo[it].stateVec[j] = CORRECT_STATE; // 
			}
			else
			{
				processo[it].stateVec[j] = UNKNOWN_STATE; //
			}
		}
	}
	int printedNodes[N+1];

	for (int i = 0; i < N; i++)
	{
		printedNodes[i] = 0; // é necessário zerar vetor;
	}
	// Schedule de eventos
	for (it = 0; it < N; it++)
	{
		schedule(TEST, interval, it);
	}
	int numeroDeEventos = 2;

	/**
	 */  
	TipoAgendaEvento *sEvent;
	sEvent = (TipoAgendaEvento *)malloc(sizeof(TipoAgendaEvento) * numeroDeEventos);

	// sEvent[1].type = FAULT_TYPE;
	// sEvent[1].time = 10.0;
	// sEvent[1].process = 0;
	
	// sEvent[2].type = FAULT_TYPE;
	// sEvent[2].time = 10.0;
	// sEvent[2].process = 2;

	// sEvent[3].type = FAULT_TYPE;
	// sEvent[3].time = 10.0;
	// sEvent[3].process = 4;

	// sEvent[4].type = FAULT_TYPE;
	// sEvent[4].time = 10.0;
	// sEvent[4].process = 7;

	sEvent[0].process = 0;
	sEvent[0].time = 105.0;
	sEvent[0].type = MIN_SPANNING_TREE_TYPE;

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
		else if (sEvent[it].type == MIN_SPANNING_TREE_TYPE){
			schedule(SPANNING_TREE, sEvent[it].time, sEvent[it].process); 
		}
	}
	// imprime na tela cabeçalho do log
	printLogHeader(N, sEvent, numeroDeEventos); 
	 /**/


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
				printf("\n************************************************ Início rodada: %02d ************************************************\n", tr);
				t = time();
				tr++;
			}
			if (status(processo[token].id) != 0)
			{
				printf("Tempo:%4.1f\t\t\t\tProcesso %02d está falho.\n", time(), token);
			}
			else
			{
				executeTest(processo, N, token, procStarted, tr, &newEvento);
			}

			schedule(TEST, interval, token);

			if (token == N - 1)
			{
				printf("**********************************************Fim da rodada %02d **********************************************\n\n", iTest + 1);
				if (!procStarted )
				{
					procStarted = checkProcessStartDetection(processo, N);
				}
				else
				{
					if (newEvento.novoEvento)
					{ // é necessário verificar se evento já foi detectado
						if (checkEventFullDetection(processo, N, newEvento.processoIdentificado, newEvento.newState))
						{ // se evento foi detectado

							printf("Resumo rodada de teste:\n");
							if (newEvento.newState % 2 == 0)
							{ // evento de recover

								printf("\t\Detecção completa: processo %02d recuperado.\n", newEvento.processoIdentificado);
							}
							else
							{
								printf("\t\Detecção completa: processo %02d falho.\n", newEvento.processoIdentificado);
							}

							printf("\t\Latência: %02d rodada(s).\n", tr + 1 - newEvento.TestRound);
							printf("\t\Tempo de detecção: %4.1f s.\n", time() - newEvento.initTime);

							printf("\t\tTotal de testes: %02d.\n", newEvento.testNumberDetected - eventTestnumber);
							printf("\n");

							// reinicia variavel de armazenamento de evento;
							resetEvent(&newEvento); 
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
			printf(" Tempo:%4.1f\t\t\tProcesso %02d FALHOU.\n", time(), token);
			eventTestnumber = testnumber - 1;
			break;

		case REPAIR:
			release(processo[token].id, token);
			printf("Tempo: %4.1f\t\t\tProcesso %02d recuperado. \t\t\tProcesso %02d status: ", time(), token, token);
			for (it = 0; it < N; it++)
			{
				if (it == token)
				{
					processo[token].stateVec[it] = CORRECT_STATE;
				}
				else
				{
					processo[token].stateVec[it] = UNKNOWN_STATE;
				}
			}
			printStatusVec(processo, N, token);
			eventTestnumber = testnumber - 1;
			break;
		
		case SPANNING_TREE:
			printf("\n\n**********************************************\n");
			printf("Imprimindo Árvore Geradora Autonomica Mínima\n ");
			
			int dimension = (int)log2(N);

			int nChildsToPrint = dimension; 

			printf("Raiz: %d \n Número de Processos: %d \n Dimensão: %d \n",token, N, dimension); 
			printf("Status: "); 
			printStatusVec(processo, N, token);
			printf("\n"); 
			// executeTest(processo, N, token, procStarted, tr, &newEvento);
			// printf("%d \n", token);
			if (VERBOSE)
				printf("\n (PROCESSO RAIZ): \t\t %d", token);
			printf("%d \n",token );
			printedNodes[token] = 1;
			printSpanningAutonomicTree(processo, N, token, nChildsToPrint, printedNodes, 1);
			printf("\n **********************************************\n");
		
		}
	}
}
