#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "unistd.h"

#define N 10		// número de usuários
#define CARTAS 20 	// quantidade de cartas na mochila

/*
	Macros A e B definem ponto onde o pombo está, de modo que 
	o valor será atribuído à variável boolean_ponto.
*/

pthread_mutex_t mochila = PTHREAD_MUTEX_INITIALIZER;		// Lock de acesso à mochila, compartilhado entre pombo e usuário.
pthread_cond_t cond_pombo = PTHREAD_COND_INITIALIZER;		// Variável condicional que controla fluxo da thread pombo (espera mochila lotar).
pthread_cond_t cond_usuario = PTHREAD_COND_INITIALIZER;		// Variável condicional que controla fluxo das threads usuário (espera pombo ficar disponível).

void * f_usuario(void *arg);
void * f_pombo(void *arg);

int num_cartas = 0;			// Variável compartilhada que armazena número total de cartas na mochila do pombo (0 <= num_cartas <= CARTAS).
int boolean_em_A = 1;		// Variável compartilhada que armazena se pombo encontra-se na posição A ou não (1 ou 0).
int total_cartas_B = 0;		// Variável compartilhada que armazena número total de cartas deixadas no ponto B.

int main(int argc, char **argv){
    int i;
	pthread_t usuario[N];
    int *id;
	
    for(i = 0; i < N; i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&(usuario[i]),NULL,f_usuario,  (void *) (id));
    }
    pthread_t pombo;
    id = (int *) malloc(sizeof(int));
    *id = 0;
    pthread_create(&(pombo),NULL,f_pombo, (void*) (id));
	
	pthread_join(pombo,NULL);
}


void * f_pombo(void *arg){
  
    while(1){
		pthread_mutex_lock(&mochila);		// Pombo pega lock para acessar a mochila.
        //Inicialmente está em A, aguardar/dorme a mochila ficar cheia (20 cartas)			
			while(num_cartas < CARTAS){		// Enquanto número de cartas na mochila for menor que a capacidade total...
				printf("Pombo: esperando a mochila encher...\n");
				pthread_cond_wait(&cond_pombo, &mochila);	// Pombo adormece e passa fluxo para thread usuário.
			}
			//Leva as cartas para B e volta para A
			printf("\nPombo: mochila lotada! Irei viajar para B.\n");   
			sleep(5);			// Sleep que simula tempo de viagem de A para B.
			boolean_em_A = 0;	// Pombo chega em B.
			
			printf("Pombo: cheguei em B. Esvaziando mochila...\n");
			sleep(3);						// Sleep que simula tempo que pombo esvazia a mochila.
			total_cartas_B += num_cartas;	// Incrementa número de cartas depositadas no ponto B.
            num_cartas = 0;					// Esvazia mochila.
			
			printf("Pombo: total de cartas em B: %d.\n", total_cartas_B);
			printf("Pombo: voltarei para A.\n");
			sleep(5);			// Sleep que simula tempo de viagem de B para A.
			boolean_em_A = 1;	// Pombo chega em A.
			
			//Acordar os usuários
			printf("Pombo: cheguei em A. Acordem usuários!\n");
			pthread_cond_broadcast(&cond_usuario);		// Broadcast para acordar todos os usuários, visto que mochila está vazia.
		pthread_mutex_unlock(&mochila);		// Libera lock de acesso à mochila.
    }
}

void * f_usuario(void *arg){
	
	int id = *((int *) arg);    // id do usuário
    while(1){
	//Escreve uma carta
	printf("Usuario %d: estou escrevendo uma carta.\n", id);
	sleep(rand()%5+3);		// Sleep aleatório que simula tempo de escrita da carta.
		
		pthread_mutex_lock(&mochila);	// Usuário pega lock para acessar a mochila.
		
			//Caso o pombo não esteja em A ou a mochila estiver cheia, então dorme
			while(!boolean_em_A || num_cartas == CARTAS){		// Enquanto pombo não estiver em A ou mochila estiver lotada...
				
				if(!boolean_em_A){
					printf("Usuario %d: pombo não se encontra em A, vou esperar.\n", id);
				}
				
				if(num_cartas == CARTAS){
                    printf("Usuario %d: mochila do pombo esta lotada, vou esperar.\n", id);
				}
				
				pthread_cond_wait(&cond_usuario, &mochila);		// Usuário atual adormece.
			}
			
			//Posta sua carta na mochila do pombo
			num_cartas++;	// Incrementa número total de cartas na mochila.
			
			//Caso a mochila fique cheia, acorda o pombo
			if(num_cartas == CARTAS){	// Caso mochila tenha ficado lotada...
				// Printa mensagem de mochila lotada.
				printf("\n\tUsuario %d: acorde pombo, coloquei a carta na mochila e ficou lotada.\n\t*- Quantidade total na mochila: %d -*\n\n", id, num_cartas);
				pthread_cond_signal(&cond_pombo);	// Acorda pombo.
			}else{
				// Printa mensagem padrão.
				printf("\n\tUsuario %d: coloquei a carta na mochila do pombo.\n\t*- Quantidade total na mochila: %d -*\n\n", id, num_cartas);
			}
		pthread_mutex_unlock(&mochila);		// Libera lock de acesso à mochila.
    }
}
