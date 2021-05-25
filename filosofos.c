/*
	Para uma melhor visualização do problema, disponibilizei
	uma imagem que representa os ids dos filósofos e dos seus
	respectivos talheres, para o problema inicial (N = 5).
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define N 5

/*
		Para N = 5, temos que, em uma mesa circular:
		Filósofo de id = 0: ESQ(0) = 4 e DIR(0) = 1
		Filósofo de id = 1: ESQ(1) = 0 e DIR(1) = 2
		Filósofo de id = 2: ESQ(2) = 1 e DIR(2) = 3
		Filósofo de id = 3: ESQ(3) = 2 e DIR(3) = 4
		Filósofo de id = 4: ESQ(4) = 3 e DIR(4) = 0
	
	De modo equivalente pode-se alterar N e as macros ESQ e DIR 
		continuam retornando seus adjacentes equivalentes.
*/

#define ESQ(id) (N-1+id)%N		// Retorna valor do id referente ao filósofo à esquerda do especificado.
#define DIR(id)	(id+1)%N		// Retorna valor do id referente ao filósofo à direita do especificado.

/*
	Flags de estado que serão associadas para cada filósofo.
*/

#define PENSANDO 0
#define COM_FOME 1
#define COMENDO 2

sem_t s[N];
sem_t sem_mesa;		// Semáforo que será utilizado como lock para acesso à mesa (pegar/devolver os talheres).

void *filosofos (void *arg);
void pega_talher (int n);
void devolve_talher (int n);

int estado[N] = {PENSANDO};		// Armazena estado dos filósofos (inicialmente todos pensando, ou seja, todos iguais a zero).
int qtd_total_comendo = 0;		// Apenas para verificar a quantidade de filósofos comendo simultaneamente (para N = 5, deve ser no máximo igual a 2).

int main () {
	int i;
	int *id;
	
	//semaforos
	for (i=0; i<N; i++) {
		sem_init(&s[i], 0, 0);	// Inicializam com zero permissões (são dadas quando garantem a posse dos talheres).
	}
	pthread_t r[N]; 
	
	sem_init(&sem_mesa, 0, 1);	// Inicializa com uma permissão (atuará como um lock).
	
	//criacao das threads de filosofos
       for (i = 0; i < N ; i++) {
	     id = (int *) malloc(sizeof(int));
	     *id = i;
	     pthread_create(&r[i], NULL, filosofos, (void*) (id));
	}

    pthread_join(r[0],NULL);
	return 0;
}


void *filosofos (void *arg) {
	int n = *((int *) arg);
	
	/*
		Para N = 5, temos que, em uma mesa circular:
		Filósofo de id = 0: talher à esquerda = 0 e talher à direita = 1
		Filósofo de id = 1: talher à esquerda = 1 e talher à direita = 2
		Filósofo de id = 2: talher à esquerda = 2 e talher à direita = 3
		Filósofo de id = 3: talher à esquerda = 3 e talher à direita = 4
		Filósofo de id = 4: talher à esquerda = 4 e talher à direita = 0
	
	Assim, é importante notar que o talher à esquerda equivale ao id do filósofo
		e o talher à direita equivale ao id do filósofo à direita.
	*/
	
	int talher_esquerda = n;
	int talher_direita = DIR(n);
	
	while(1) {
		//pensar
		printf("Filosofo %d pensando...\n", n);
		sleep(3);
		
		estado[n] = COM_FOME;	// Após pensar, filósofo passa a ter estado faminto.
		printf("Filosofo %d esta com fome!\n", n);
		pega_talher(n);		// Tenta pegar talher.
		
		sem_wait(&s[n]);	// Caso tenha permissão (esteja comendo), passa adiante.
		printf("\tFilosofo %d comendo, utilizando talheres %d e %d.\n\t*- Quantidade de filosofos comendo: %d -*\n", n, talher_esquerda, talher_direita, qtd_total_comendo);
		sleep(5);
		
		printf("\tFilosofo %d acabou de comer.\n", n);
		devolve_talher(n);	// Filósofo devolve talher e chama adjacentes para tentar pegar talher (evita deadlock em sem_wait).
	} 
}

void pega_talher (int n) {	
	sem_wait(&sem_mesa);	// Pega permissão para acesso à mesa.
		if(estado[n] == COM_FOME && estado[ESQ(n)] != COMENDO && estado[DIR(n)] != COMENDO){
		/* Caso o filósofo esteja faminto e seus adjacentes não estejam comendo no momento... */
			estado[n] = COMENDO;	// Filósofo passa a comer.
			qtd_total_comendo++;	// Incrementa variável de quantidade total de filósofos comendo.
			sem_post(&s[n]);		// Permissão para filósofo prosseguir.
		}
	sem_post(&sem_mesa);	// Libera permissão para acesso à mesa.
}

void devolve_talher (int n) {
	sem_wait(&sem_mesa);	// Pega permissão para acesso à mesa.
		estado[n] = PENSANDO;	// Filósofo volta a pensar.
		qtd_total_comendo--;	// Decrementa variável de quantidade total de filósofos comendo.
	sem_post(&sem_mesa);	// Libera permissão para acesso à mesa.
	
	/* Chama filósofos adjacentes para comer, de modo que não fiquem presos em "sem_wait(&s[n])" */
    pega_talher(ESQ(n));
	pega_talher(DIR(n));
}
