#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "unistd.h"

#define N 10 		//número de usuários
#define CARTAS 20 	//quantidade de cartas na mochila

sem_t sem_pombo;	// Semáforo que garante permissão de acesso ao pombo (funciona como lock).
sem_t sem_cartas;	// Semáforo que garante permissão de escrita para usuário.
sem_t sem_mochila;	// Semáforo que garante permissão de viagem para pombo.

void * f_usuario(void *arg);
void * f_pombo(void *arg);

int num_cartas = 0;			// Variável compartilhada que armazena número total de cartas na mochila do pombo (0 <= num_cartas <= CARTAS).
int total_cartas_B = 0;		// Variável compartilhada que armazena número total de cartas deixadas no ponto B.

int main(int argc, char **argv){
    int i;
    pthread_t usuario[N];
    int *id;
	
	sem_init(&sem_pombo, 0, 1);			// Inicializa com uma permissão (funciona como lock de acesso à mochila do pombo).
	sem_init(&sem_cartas, 0, CARTAS);	// Inicializa com permissões igual a capacidade de cartas na mochila.
    sem_init(&sem_mochila, 0, 0);		// Inicializa com zero, visto que pombo fica bloqueado até mochila lotar.
	
    for(i = 0; i < N; i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&(usuario[i]),NULL,f_usuario, (void *) (id));
    }
    pthread_t pombo;
    id = (int *) malloc(sizeof(int));
    *id = 0;
    pthread_create(&(pombo),NULL,f_pombo, (void*) (id));

    pthread_join(pombo,NULL);   
}

void * f_pombo(void *arg){
  
    while(1){
        //Inicialmente está em A, aguardar/dorme a mochila ficar cheia (20 cartas)
        printf("Pombo: esperando a mochila encher...\n");
		sem_wait(&sem_mochila);		// Pombo espera mochila ficar lotada...
        sem_wait(&sem_pombo);		// Garante permissão de acesso ao pombo (para realizar a viagem).
		//Leva as cartas para B e volta para A
		printf("\nPombo: mochila lotada! Irei viajar para B.\n");
		sleep(5);		// Sleep que simula tempo de viagem de A para B.
		
		printf("Pombo: cheguei em B. Esvaziando mochila...\n");
		sleep(3);						// Sleep que simula tempo que pombo esvazia a mochila.
		total_cartas_B += num_cartas;	// Incrementa número de cartas depositadas no ponto B.
		
		printf("Pombo: total de cartas em B: %d.\n", total_cartas_B);
		printf("Pombo: voltarei para A.\n");
		sleep(5);		// Sleep que simula tempo de viagem de B para A.
		
		while(num_cartas != 0){		// Caso a mochila não esteja vazia...
			num_cartas--;			// Decrementa número de cartas a cada iteração.
            sem_post(&sem_cartas); 	// Dá permissão para que mais uma carta seja escrita (totalizando a capacidade da mochila, no final).
        }
		
        //Acordar os usuários
		printf("Pombo: cheguei em A. Acordem usuários!\n");
		sem_post(&sem_pombo); // Libera permissão de acesso ao pombo.
    }
}

void * f_usuario(void *arg){
    int id = *((int *) arg); // id do usuário
	
    while(1){
		//Escreve uma carta
		printf("Usuario %d: estou escrevendo uma carta.\n", id);
		sleep(rand()%5+3);			// Sleep aleatório que simula tempo de escrita da carta.

		//Caso o pombo não esteja em A ou a mochila estiver cheia, então dorme
		
		sem_wait(&sem_cartas);		// Garante permissão de escrita de carta.
		sem_wait(&sem_pombo);		// Garante permissão de acesso ao pombo (para colocar cartas na mochila).
		
		//Posta sua carta na mochila do pombo
		num_cartas++;		// Incrementa número total de cartas na mochila.
		
		//Caso a mochila fique cheia, acorda pombo
		if(num_cartas == CARTAS){		// Caso mochila tenha ficado lotada...
			printf("\n\tUsuario %d: acorde pombo, coloquei a carta na mochila e ficou lotada.\n\t*- Quantidade total na mochila: %d -*\n\n", id, num_cartas);
			sem_post(&sem_mochila);		// Permissão para pombo realizar viagem, visto que mochila está lotada.
		}else{
			printf("\n\tUsuario %d: coloquei a carta na mochila do pombo.\n\t*- Quantidade total na mochila: %d -*\n\n", id, num_cartas);
		}
		sem_post(&sem_pombo);		// Libera permissão de acesso ao pombo.
    }
}
