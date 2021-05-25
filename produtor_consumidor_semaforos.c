#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define PR 2 //número de produtores
#define CN 5 // número de consumidores
#define N 5  //tamanho do buffer


void * produtor(void * meuid);
void * consumidor (void * meuid);

sem_t mutex;      // Definição do semáforo que funcionará como um lock mutex.
sem_t empty;      // Definição do semáforo que funcionará como controle de entrada de produtores.
sem_t full;       // Definição do semáforo que funcionará como controle de entrada de consumidores.

int buffer[N];    // Variável compartilhada que representa um buffer de memória de tamanho N.
int inserir = 0;  // Controla posição do ponteiro do buffer a partir de inserção de um elemento (referente à produtores).
int remover = 0;  // Controla posição do ponteiro do buffer a partir de remoção de um elemento (referente à consumidores).
int contador = 0; // Para verificação da quantidade de elementos no buffer (verifica se está lotado ou vazio).

void main(argc, argv)
int argc;
char *argv[];
{

  int erro;
  int i, n, m;
  int *id;
  
  /* 
     Inicialização dos semáforos:

     -> mutex := inicializa com o valor 1, uma vez que possui a função de um lock 
        (é atribuido down no começo da região crítica e up no final).

     -> empty := inicializa com o valor N, uma vez que representa permissões da thread produtor na região crítica, 
        inicialmente para todo o tamanho do buffer (N).

     -> full  := inicializa com o valor 0, uma vez que representa permissões da thread consumidor na região crítica, 
        inicialmente com o buffer vazio.
  */

  sem_init(&mutex, 0, 1);   // Inicialização do semáforo mutex, que executa DOWN antes da região crítica e UP após região crítica, em ambos processos.
  sem_init(&empty, 0, N);   // Inicialização do semáforo empty, que executa DOWN em produtor e UP em consumidor (refere-se a controle de inserção de elementos no buffer).
  sem_init(&full, 0, 0);    // Inicialização do semáforo full, que executa DOWN em consumidor e UP em produtor (refere-se a controle de remoção de elementos no buffer).
  
  pthread_t tid[PR];
  
  srand48(time(NULL));    // Seed a ser utilizada na produção de um item (apenas figurativo).

  for (i = 0; i < PR; i++)
  {
    id = (int *) malloc(sizeof(int));
    *id = i;
    erro = pthread_create(&tid[i], NULL, produtor, (void *) (id));

    if(erro)
    {
      printf("Erro na criacao do thread %d.\n", i);
      exit(1);
    }
  }

  pthread_t tCid[CN];

  for (i = 0; i < CN; i++)
  {
    id = (int *) malloc(sizeof(int));
    *id = i;
    erro = pthread_create(&tCid[i], NULL, consumidor, (void *) (id));

    if(erro)
    {
      printf("Erro na criacao do thread %d.\n", i);
      exit(1);
    }
  }
 
  pthread_join(tid[0],NULL);

}

void * produtor (void* pi)
{
  int item;   // Variável local referente à produção de um item inteiro.
  
  while(1)
  {
	printf("Produtor %d: vou produzir um item!\n", *(int *)(pi));   // Ação de produzir um item (fora da região crítica).
	item = (int) (drand48() * 1000.0);    // Produção de um item de valor aleatório pela thread.
	sleep(1);   // Sleep referente ao tempo de produção do item.
	
	sem_wait(&empty);   // DOWN em empty, caso seja possível (número de elementos no buffer menor do que N).
	sem_wait(&mutex);   // DOWN em mutex para acessar região crítica.

	printf("Produtor %d: vou inserir item %d na posição %d.\n", *(int *)(pi), item, inserir);   // Ação de inserção de item no buffer, para dada posição.
	buffer[inserir] = item;   // Insere item no buffer na posição atual da variável inserir.

	inserir++;          // Incrementa variável inserir, para posterior inserção na posição do buffer.
	if(inserir >= N){   // Caso "inserir" seja maior ou igual ao tamanho do buffer...
		inserir = 0;      // É atribuido o valor zero, uma vez que o buffer é circular (volta a posição inicial).
	}

	contador++;         // Incrementa número de elementos que constam no buffer.
	if(contador == N){  // Caso número de elementos seja igual ao tamanho do buffer...
		printf("\n\tProdutor %d: BUFFER LOTADO! TAMANHO ATUAL: %d/%d.\n\n", *(int *)(pi), contador, N);   // Imprime mensagem de buffer lotado.
	}

	sem_post(&mutex);   // UP em mutex para liberar acesso à região crítica.
	sem_post(&full);    // UP em full para liberar acesso no buffer aos consumidores, uma vez que possui elementos presentes agora.
  }

  pthread_exit(0);
}

void * consumidor (void* pi)
{
 int lido;
 
 while(1)
  {
	sem_wait(&full);    // DOWN em full, caso seja possível (número de elementos no buffer maior do que zero).
	sem_wait(&mutex);   // DOWN em mutex para acessar região crítica.

	lido = buffer[remover];   // Lê item a ser removido do buffer na posição atual da variável remover.
	printf("Consumidor %d: vou remover item %d na posição %d.\n", *(int *)(pi), lido, remover);   // Ação de remoção de item no buffer, para dada posição.

	remover++;          // Incrementa variável remover, para posterior remoção na posição do buffer.
	if(remover >= N){   // Caso "remover" seja maior ou igual ao tamanho do buffer...
		remover = 0;      // É atribuido o valor zero, uma vez que o buffer é circular (volta a posição inicial).
	}

	contador--;         // Decrementa número de elementos que constam no buffer.
	if(contador == 0){  // Caso número de elementos no buffer seja igual a zero...
		printf("\n\tConsumidor %d: BUFFER VAZIO! TAMANHO ATUAL: %d/%d.\n\n", *(int *)(pi), contador, N);  // Imprime mensagem de buffer vazio.
	}

	sem_post(&mutex);   // UP em mutex para liberar acesso à região crítica.
	sem_post(&empty);   // UP em empty para liberar acesso no buffer aos produtores, uma vez que um elemento foi removido.

	printf("Consumidor %d: vou consumir item %d!\n", *(int *)(pi), lido);   // Ação de consumir o item (fora da região crítica).
	sleep(2);   // Sleep referente ao tempo de consumir dado item.
  }

  pthread_exit(0);
}
