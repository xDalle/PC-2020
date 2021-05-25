/*
 * Problema do barbeiro dorminhoco.
 */ 
 
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_CLIENTES 50
#define N_CADEIRAS 5

sem_t sem_cadeiras;           // Definição do semáforo que funcionará como permissões das cadeiras de espera.
sem_t sem_cadeira_barbeiro;   // Definição do semáforo que funcionará como um lock para cliente sentar na cadeira do barbeiro.
sem_t sem_barbeiro;           // Definição do semáforo que funcionará como uma variável condicional (acorda e adormece o barbeiro).
sem_t sem_corte_cabelo;       // Definição do semáforo que funcionará como uma variável condicional (acorda e adormece o cliente).

void * f_barbeiro(void *v) {

  while(1) {
    sem_wait(&sem_barbeiro);	//...Esperar/dormindo algum cliente sentar na cadeira do barbeiro (e acordar o barbeiro)
    
    sleep(1); //Cortar o cabelo do cliente 
    printf("Barbeiro cortou o cabelo de um cliente.\n"); 
    
    sem_post(&sem_corte_cabelo);	//...Liberar/desbloquear o cliente
  }
  pthread_exit(0);
}

void* f_cliente(void* v) {
  int id = *(int*) v;
  sleep(id%3);
  if(sem_trywait(&sem_cadeiras) == 0){ //conseguiu pegar uma cadeira de espera
     printf("Cliente %d entrou na barbearia.\n",id);
     sem_wait(&sem_cadeira_barbeiro);	//... pegar/sentar a cadeira do barbeiro
     sem_post(&sem_cadeiras);	//... liberar a sua cadeira de espera
     sem_post(&sem_barbeiro);	//... acordar o barbeiro para cortar seu cabelo
     sem_wait(&sem_corte_cabelo);	//... aguardar o corte do seu cabelo
     sem_post(&sem_cadeira_barbeiro);	//... liberar a cadeira do barbeiro
     printf("Cliente %d cortou o cabelo e foi embora.\n",id);

  }else{//barbearia cheia
     printf("Barbearia cheia, cliente %d indo embora.\n",id);
  }

  pthread_exit(0);
}

int main() {
  pthread_t thr_clientes[N_CLIENTES], thr_barbeiro;
  int i, id[N_CLIENTES];

  sem_init(&sem_cadeiras, 0, N_CADEIRAS); // Inicia semáforo com permissões igual ao número de cadeiras disponíveis.
  sem_init(&sem_cadeira_barbeiro, 0, 1);  // Inicia semáforo com uma permissão, visto que cliente pega inicialmente (wait) e barbeiro devolve permissão (post).
  sem_init(&sem_barbeiro, 0, 0);          // Inicia semáforo com zero permissões, visto que cliente dá um post quando senta na cadeira do barbeiro, e barbeiro espera isso.
  sem_init(&sem_corte_cabelo, 0, 0);      // Inicia semáforo com zero permissões, uma vez que cliente espera post do barbeiro enquanto o cabelo está sendo cortado.
  
  for (i = 0; i < N_CLIENTES; i++) {
    id[i] = i;
    pthread_create(&thr_clientes[i], NULL, f_cliente, (void*) &id[i]);
  }

  pthread_create(&thr_barbeiro, NULL, f_barbeiro, NULL);
  
  for (i = 0; i < N_CLIENTES; i++) 
    pthread_join(thr_clientes[i], NULL);

  /* Barbeiro assassinado */
  
  return 0;
}

