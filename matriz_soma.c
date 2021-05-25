#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXSIZE 10000  /* maximum matrix size */

pthread_barrier_t barrier;

int size;		// Tamanho da matriz (definida por argv[1]).
int soma_parcial[MAXSIZE] = {};	// Vetor que armazena soma de cada linha da matriz (provavelmente esparso, dependendo da variável size).
int soma_total = 0;				// Variável que armazena a soma total dos elementos da matriz. 

int matrix[MAXSIZE][MAXSIZE];

void *Worker(void *);

int main(int argc, char *argv[]) {
	int i, j;
	
	if(argc != 2){  // Analisa número de argumentos passados para evitar segmentation fault...
		printf("Numero de argumentos invalido! Deve ser igual a 2 (nome do programa e tamanho da matriz).\n");
		return 0;
	}
	
    size = atoi(argv[1]);	// Atribui a size o tamanho da matriz.
  
    if(size > MAXSIZE){		// Caso size seja maior que o tamanho máximo...
	    printf("Tamanho muito grande!\n");
        return 1;	// Finaliza programa.
    }

    pthread_t workerid[size];
  
    pthread_barrier_init(&barrier, NULL, size);

    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            matrix[i][j] = 1;		// Cada elemento da matriz será igual a 1 (soma total deverá ser igual a size*size).

    int * id;

    for (i = 0; i < size; i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&workerid[i], NULL, Worker, (void *) (id));
    }

    for (i = 0; i < size; i++){
        if(pthread_join(workerid[i], NULL)){
            printf("\n ERROR joining thread");
            exit(1);
        }
    }

    printf("Bye!\n");
}


void *Worker(void *arg) {
	int myid = *(int *)(arg);
	int j, k;
	int self = pthread_self();
 
	printf("worker %d (pthread id %d) has started\n", myid, self);

	for(k = 0; k < size; k++){
		soma_parcial[myid] += matrix[myid][k];	// Cada worker soma todos os elementos de uma linha da matriz.
	}

    pthread_barrier_wait(&barrier);		// Barreira utilizada para que espere a finalização das threads na soma de cada linha.
	
    if (myid == 0) {	// Primeira thread imprime resultado...
        printf("\n ");
		printf("Matriz criada:");
		printf("\n ");
        for(j = 0; j < size; j++){
			soma_total += soma_parcial[j];	// Soma parcial de cada linha é atribuída à soma total.
	        for(k = 0; k < size; k++){
                printf("%d ",matrix[j][k]);   // Imprime elementos da matriz.
            }
            printf("\n ");
        }
		printf("\nSoma total = %d.\n", soma_total);	// Imprime soma total.
    }
}