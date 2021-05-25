/*

	Neste programa é possível retirar o comentário da função "sleep()" da ação de cada processo respectivo,
	de modo que induza um enchimento ou esvaziamento do buffer, uma vez que um processo "trabalha" mais
			    rápido que o outro. As funções estão no final do arquivo. 



																*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PR 1 //número de produtores
#define CN 1 // número de consumidores
#define N 5  //tamanho do buffer

void * produtor (void * meuid);
void * consumidor (void * meuid);

pthread_cond_t prod_cond = PTHREAD_COND_INITIALIZER;	// Inicialização da variável condicional dos produtores.
pthread_cond_t cons_cond = PTHREAD_COND_INITIALIZER;	// Inicialização da variável condicional dos consumidores.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// Inicialização do lock mutex para controlar a variável condicional.

int produce_item();		// Função que gera um item para ser inserido no buffer.
void insert_item(int item);	// Função que simula uma inserção de item no buffer.
int remove_item();		// Função que simula uma remoção de item do buffer.
void consume_item(int item);	// Função que imprime o item retirado do buffer.

int count = 0;	// Variável compartilhada que funciona como contador de itens no buffer (0 <= count <= N).

void main(argc, argv)
int argc;
char *argv[];
{

	int erro;
	int i, n, m;
	int *id;
	
	printf("\n- TAMANHO OCUPADO INICIAL DO BUFFER: %d/%d -\n\n", count, N);	// Imprime tamanho inicialmente ocupado do buffer ( 0 ).
	pthread_t tid[PR];
   
	for (i = 0; i < PR; i++){
		id = (int *) malloc(sizeof(int));
		*id = i;
		erro = pthread_create(&tid[i], NULL, produtor, (void *) (id));

		if(erro){
			printf("erro na criacao do thread %d\n", i);
			exit(1);
		}
	}	

	pthread_t tCid[CN];

	for (i = 0; i < CN; i++){
		id = (int *) malloc(sizeof(int));
		*id = i;
		erro = pthread_create(&tCid[i], NULL, consumidor, (void *) (id));

		if(erro){
			printf("erro na criacao do thread %d\n", i);
			exit(1);
		}
	}

  pthread_join(tid[0],NULL);

}

void * produtor (void* pi){
	int item;
	
	while(1){
		item = produce_item();		// Produtor gera item.
		pthread_mutex_lock(&mutex);	// Produtor pega lock mutex para garantir exclusão mútua.
			while(count == N){	// Enquanto o buffer estiver lotado...
				printf("Produtor: buffer está lotado, vou descansar!\n");
				pthread_cond_wait(&prod_cond, &mutex);	// Ocorre bloqueio da thread produtor pela variável condicional (produtor espera).
			}
			insert_item(item);	// Produtor insere item no buffer.
			count++;		// Incrementa contador de itens no buffer.
			
			printf("\n- TAMANHO OCUPADO DO BUFFER: %d/%d -\n\n", count, N);	// Imprime relação itens/buffer.
			
			if(count == 1){	// Caso o buffer deixe de estar vazio...
				pthread_cond_signal(&cons_cond);	// Desbloqueia a thread dos consumidores.
			}
		pthread_mutex_unlock(&mutex);		// Libera lock mutex ao finalizar iteração.
	}
  
  pthread_exit(0);
}

void * consumidor (void* pi){
	int item;
	
	while(1){
		pthread_mutex_lock(&mutex);	// Consumidor pega lock mutex para garantir exclusão mútua.
			while(count == 0){	// Enquanto o buffer estiver vazio...
				printf("Consumidor: buffer está vazio, vou descansar!\n");
				pthread_cond_wait(&cons_cond, &mutex);	// Ocorre bloqueio da thread consumidor pela variável condicional (consumidor espera).
			}
			
			item = remove_item();	// Consumidor remove item do buffer.
			count--;		// Decrementa contador de itens no buffer.
			
			printf("\n- TAMANHO OCUPADO DO BUFFER: %d/%d -\n\n", count, N);	// Imprime relação itens/buffer.
			
			if(count == N-1){	// Caso o buffer deixe de estar lotado...
				pthread_cond_signal(&prod_cond);	// Desbloqueia a thread dos produtores.
			}
					
		pthread_mutex_unlock(&mutex);	// Libera lock mutex ao finalizar iteração.
		consume_item(item);		// Consumidor imprime item removido do buffer.
	}
  
  pthread_exit(0);
}

int produce_item(){
	static int item_counter = 0;	// variável para manter um controle (contador) acerca dos itens gerados.
	printf("Produtor: gerando item...\n");
	sleep(rand() % 5);	// sleep para simular tarefas com tempos aleatórios (capacidade do buffer preenchida aleatoriamente).
	
	/* 		- Sleep que força buffer vazio (produtor mais lento que consumidor) -
					Basta retirar comentário abaixo.				    */
					
							
	//sleep(15);
	
	item_counter++;	// incrementa número total de itens gerados.
	return item_counter;	// retorna o índice do item gerado.
}

void insert_item(int item){
	printf("Produtor: colocando item %d no buffer.\n", item);
	sleep(rand() % 5);	// sleep para simular tarefas com tempos aleatórios (capacidade do buffer preenchida aleatoriamente).
}

int remove_item(){
	static int item_counter = 0;	// variável para manter um controle (contador) acerca dos itens removidos.
	printf("Consumidor: removendo item do buffer...\n");
	sleep(rand() % 5);	// sleep para simular tarefas com tempos aleatórios (capacidade do buffer preenchida aleatoriamente).
	item_counter++;	// incrementa número total de itens removidos.
	return item_counter;	// retorna o índice do item removido.
}

void consume_item(int item){
	printf("Consumidor: imprimindo item retirado: %d\n", item);
	sleep(rand() % 5);	// sleep para simular tarefas com tempos aleatórios (capacidade do buffer preenchida aleatoriamente).
	
	/* 		- Sleep que força buffer lotado (consumidor mais lento que produtor) -
					Basta retirar comentário abaixo.				    */
					
	//sleep(15);
}

