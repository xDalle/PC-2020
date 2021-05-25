/*
Aluno: Lucas Dalle Rocha
Matrícula: 17/0016641
*/

#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define TRUE 1

#define NE 3 //numero de escritores
#define NL 10 //numero de leitores

pthread_mutex_t lock_bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_nl = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_st = PTHREAD_MUTEX_INITIALIZER; //lock para garantir acesso ao lock_bd (evita starvation dos escritores).

int num_leitores = 0;

void * reader(void *arg);
void * writer(void *arg);
void read_data_base();
void use_data_read();
void think_up_data();
void write_data_base();

int main() {

	pthread_t r[NL], w[NE];
	int i;
    int *id;
    /* criando leitores */
    for (i = 0; i < NL ; i++) {
		id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&r[i], NULL, reader, (void *) (id));
	}
	/* criando escritores */
	for (i = 0; i< NE; i++) {
		id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&w[i], NULL, writer, (void *) (id));
	}
	pthread_join(r[0],NULL);
	return 0;
}

void * reader(void *arg) {
	int i = *((int *) arg);		/* i recebe id do processo */
	while(TRUE) {		/* repete para sempre */
		pthread_mutex_lock(&lock_st);		/* leitor pega lock_st para garantir acesso ao bd */
			pthread_mutex_lock(&lock_nl);		/* pega lock_nl para manipular variável compartilhada (garante exclusão mútua) */
		num_leitores++;			/* incrementa número total de leitores */
		if(num_leitores == 1){		/* caso seja o primeiro processo a executar a função, */
		    pthread_mutex_lock(&lock_bd);		/* pega lock para acesso ao bd (garante exclusão mútua) */
		}
        pthread_mutex_unlock(&lock_nl);		/* libera lock de leitura, uma vez que já manipulou num_leitores */
		pthread_mutex_unlock(&lock_st);		/* libera lock starvation para possibilitar novos leitores ou escritores */

        read_data_base(i);       /* acesso aos dados */
		 
        pthread_mutex_lock(&lock_nl);		/* pega lock_nl para manipular variável compartilhada (garante exclusão mútua) */
			num_leitores--;			/* ocorre decremento do número total de leitores */
		    if(num_leitores == 0){ 		/* caso seja o último processo a executar a função, */
		        pthread_mutex_unlock(&lock_bd);		/* libera lock de acesso ao bd */
		    }
        pthread_mutex_unlock(&lock_nl);		/* libera lock de leitura, uma vez que já manipulou num_leitores */
        use_data_read(i);        /* região não crítica, então fica exterior aos locks */
	}
    pthread_exit(0);	/* encerra processo */
}

void * writer(void *arg) {
	int i = *((int *) arg);
	while(TRUE) {               /* repete para sempre */
		think_up_data(i);        /* região não crítica, então fica exterior aos locks */
		pthread_mutex_lock(&lock_st);		/* escritor pega lock_st para garantir acesso ao bd (garante que não haverá starvation de escritores) */	
        	pthread_mutex_lock(&lock_bd);		/* pega lock para acesso ao bd (garante exclusão mútua, de modo que não haverá múltiplos escritores ao mesmo tempo) */
		pthread_mutex_unlock(&lock_st);		/* libera lock starvation para possibilitar novos leitores ou escritores */
        write_data_base(i);      /* atualiza os dados */
		pthread_mutex_unlock(&lock_bd);		/* libera lock de acesso ao bd */
        }
    pthread_exit(0);	/* encerra processo */
}

void read_data_base(int i) {
	printf("Leitor %d está lendo os dados! Número de leitores: %d\n", i,num_leitores);
	sleep(rand() % 5);
}

void use_data_read(int i) {
	printf("Leitor %d está usando os dados lidos! Número de leitores: %d\n", i,num_leitores);
	sleep(rand() % 5);
}

void think_up_data(int i) {
	printf("Escritor %d está pensando no que escrever!\n", i);
	sleep(rand() % 5);
}

void write_data_base(int i) {
	printf("Escritor %d está escrevendo os dados! Número de leitores: %d\n", i,num_leitores);
	sleep( rand() % 5 + 15);
}
