# TRABALHO PRÁTICO 2: Árvore Geradora Mínima Autonômica com o vCube

A árvore geradora mínima é uma estrutura extremente importante em redes e sistemas distribuídos. Ela é a árvore que conecta todos os nodos da rede/sistema com o menor custo. No TP2 vc vai construir uma árvore geradora mínima sobre o vCube do TP1. O algoritmo para construir a árvore é o seguinte:

Algoritmo Árvore Geradora Mínima Autonômica sobre o vCube
Recebe como entrada: Raiz da árvore, Dimensão do vCube, Lista de processos falhos

Imprima(raiz)

Seja i o primeiro processo correto de C(raiz, s-filho), s-filho = 1, 2, ..., Dimensão do vCube (se houver algum): Imprima(i)

A raiz envia o s-filho correspondente para cada filho i

Se i recebe s-filho = 1
Então esse filho é folha, acabou!
Senão /* s-filho > 1 */
      o filho i envia s para o primeiro processo j correto de C(filho,s), s = 1, 2, ..., s-filho -1 (se houver algum): Imprima(j)

Repita para cada um dos filhos, até todos serem folhas e acabarem a execução do algoritmo. 
Fim Algoritmo.

Os logs devem ter obrigatoriamente as árvores geradoras mímimas para os seguintes casos:

    Raiz: 0; Dimensões do vCube: 2; nenhum processo falho;
    Raiz: 0; Dimensões do vCube: 3; nenhum processo falho;
    Raiz: 0; Dimensões do vCube: 3; Lista de processos falhos: 1, 2, 4, 7;
    Raiz: 7; Dimensões do vCube: 3; Lista de processos falhos: 0;
    Raiz: 0; Dimensões do vCube: 4; nenhum processo falho. 

Atenção: a data para disponibilizar o trabalho é 25 de julho de 2024 (não serão aceitos trabalhos fora do prazo!) 