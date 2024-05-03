//Imprime na tela vetor status
void printStatus(tnodo *nodos, int n, int tokenID){
	int it;
	int stat;
	printf("[");
	for (it = 0; it < n; ++it){
		stat = nodo[tokenID].timestamp[it];
		if(stat == -1){
			printf("\033[1;39;32m%3d\033[0m", stat);
		}else if(stat % 2 == 0)
			printf("\033[1;39;34m%3d\033[0m", stat);
		else
			printf("\033[1;39;31m%3d\033[0m", stat);
	}
	printf("  ]");
}