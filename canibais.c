#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAXCANIBAIS 20

void *canibal(void*meuid);
void *cozinheiro(int m);

pthread_cond_t ca_cond = PTHREAD_COND_INITIALIZER;		// Inicialização da variável condicional dos canibais.
pthread_cond_t co_cond = PTHREAD_COND_INITIALIZER;		// Inicialização da variável condicional dos cozinheiros.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;		// Inicialização do lock mutex para controlar a variável condicional.

int porcoes = 0;	// Variável compartilhada que funciona como contador de porções no caldeirão (0 <= count <= m).

void main(argc, argv)
int argc;
char *argv[];
{
	int erro;
	int i, n, m;
	int *id;

	pthread_t tid[MAXCANIBAIS];

	if(argc != 3){
		printf("erro na chamada do programa: jantar <#canibais> <#comida>\n");
		exit(1);
	}
  
	n = atoi (argv[1]); //número de canibais
	m = atoi (argv[2]); // quantidade de porções que o cozinheiro consegue preparar por vez
	printf("\nNumero de canibais: %d -- Quantidade de comida: %d\n", n, m);

	if(n > MAXCANIBAIS){
		printf("O numero de canibais e' maior que o maximo permitido: %d\n", MAXCANIBAIS);
		exit(1);
	}
  
	for (i = 0; i < n; i++)  {
		id = (int *) malloc(sizeof(int));
		*id = i;
		erro = pthread_create(&tid[i], NULL, canibal, (void *) (id));

		if(erro){
			printf("Erro na criacao do thread %d\n", i);
			exit(1);
		}
	}

	cozinheiro(m);
}

void * canibal (void* pi){
  
	while(1) {
		sleep(rand() % 10 + 5);	// Sleep aleatório para simular canibais acordados, mas não querem comer.
		//pegar uma porção de comida e acordar o cozinheiro se as porções acabaram
		pthread_mutex_lock(&mutex);	// Canibal pega lock mutex para garantir exclusão mútua.
			while(porcoes == 0){	// Enquanto não houver porções no caldeirão...
				pthread_cond_wait(&ca_cond, &mutex);	// Canibais dormem (esperam cozinheiro produzir mais porções).
			}
			
			if(porcoes == 1){	// Caso seja a última porção do caldeirão...
				printf("Canibal %d: Acorde, cozinheiro! Estou pegando a ultima porcao.\n", *(int *)(pi));
				pthread_cond_signal(&co_cond);	// Solicita que a thread cozinheiro continue (acorde).
			}
			porcoes--;	// Decrementa número de porções do caldeirão (caso tenha passado pelo if, ou seja, era a última porção, agora tem zero).
			
		pthread_mutex_unlock(&mutex);	// Libera lock mutex ao finalizar iteração.

		printf("Canibal %d: Vou comer a porcao que peguei.\n", *(int *)(pi));	// Ação de comer (fora da área crítica).
		sleep(5);	// Sleep que simula tempo em que canibal come.
	}
}


void *cozinheiro (int m){
 
	while(1){
		pthread_mutex_lock(&mutex);	// Cozinheiro pega lock mutex para garantir exclusão mútua.
			while(porcoes != 0){	// Enquanto número de porções não for zero, ou seja, houver porções no caldeirão...
				printf("Cozinheiro: Agora vou descansar.\n\n");
				pthread_cond_wait(&co_cond, &mutex);	// Cozinheiro dorme (espera canibais esgotarem o caldeirão).
			}
			
			printf("\nCozinheiro: Vou cozinhar...\n");
			sleep(5);	// Sleep que simula tempo em que cozinheiro produz as porções.
			porcoes = m;	// Cozinheiro enche o caldeirão, isto é, porções = capacidade total do caldeirão.
			printf("Cozinheiro: Preparei %d porcoes. Venham comer!\n", m);
			
			//acordar os canibais
			pthread_cond_broadcast(&ca_cond);	// Acorda todas as threads dos canibais (por isso utiliza-se broadcast, e não signal).
		pthread_mutex_unlock(&mutex);	// Libera lock mutex ao finalizar iteração.
	}
}

