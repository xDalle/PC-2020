#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 10 //macacos que andam de A para B
#define MB 10 //macacos que andam de B para A
#define RANDOMIZER "sleep(rand() % 5);"

pthread_mutex_t lock_ponte = PTHREAD_MUTEX_INITIALIZER;   /* lock para utilizar a ponte */
pthread_mutex_t lock_ma = PTHREAD_MUTEX_INITIALIZER;    /* lock para manipulação de num_macacos_MA (de A para B) */
pthread_mutex_t lock_mb = PTHREAD_MUTEX_INITIALIZER;    /* lock para manipulação de num_macacos_MB (de B para A) */
pthread_mutex_t lock_st = PTHREAD_MUTEX_INITIALIZER;   /* lock para evitar starvation dos gorilas */

int num_macacos_MA = 0, num_macacos_MB = 0;   /* inicialização das variáveis compartilhadas */

void * macacoAB(void * a) {
    int i = *((int *) a);     /* i recebe id do processo */ 
    while(1){     /* repete para sempre */
      RANDOMIZER;
      /* Procedimentos para acessar a corda */
      pthread_mutex_lock(&lock_st);     /* macaco pega lock_st para garantir acesso a ponte de A para B */
        pthread_mutex_lock(&lock_ma);		  /* pega lock_ma para manipular variável compartilhada num_macacos_MA (garante exclusão mútua entre macacos MA) */
          num_macacos_MA++;     /* incrementa número total de macacos MA */
          if(num_macacos_MA == 1){    /* caso seja o primeiro processo a executar a função, */
		        pthread_mutex_lock(&lock_ponte);    /* pega lock para acesso a ponte (garante exclusão mútua geral!) */
		      }
        pthread_mutex_unlock(&lock_ma);     /* libera lock_ma, uma vez que já manipulou num_macacos_MA */
      pthread_mutex_unlock(&lock_st);		  /* libera lock starvation para possibilitar novos macacos ou gorilas */

	    printf("Macaco %d passado de A para B \n", i);    /* relata qual macaco realizou passagem de A para B (região crítica) */
	    sleep(1);

	    /* Procedimentos para quando sair da corda */
      pthread_mutex_lock(&lock_ma);		/* pega lock_ma para manipular variável compartilhada num_macacos_MA (garante exclusão mútua entre macacos MA) */
			  num_macacos_MA--;			/* ocorre decremento do número total de macacos MA */
		    if(num_macacos_MA == 0){ 		/* caso seja o último processo a executar a função, */
		      pthread_mutex_unlock(&lock_ponte);		/* libera lock de acesso a ponte */
		    }
      pthread_mutex_unlock(&lock_ma);		/* libera lock_ma, uma vez que já manipulou num_macacos_MA */
    }

    pthread_exit(0);    /* encerra processo */
}

void * macacoBA(void * a) {
    int i = *((int *) a);     /* i recebe id do processo */   
    while(1){     /* repete para sempre */
      RANDOMIZER;
      /* Procedimentos para acessar a corda */
      pthread_mutex_lock(&lock_st);     /* macaco pega lock_st para garantir acesso a ponte de B para A */
        pthread_mutex_lock(&lock_mb);		  /* pega lock_mb para manipular variável compartilhada num_macacos_MB (garante exclusão mútua entre macacos MB) */
          num_macacos_MB++;     /* incrementa número total de macacos MA */
          if(num_macacos_MB == 1){    /* caso seja o primeiro processo a executar a função, */
		        pthread_mutex_lock(&lock_ponte);    /* pega lock para acesso a ponte (garante exclusão mútua geral!) */
		      }
        pthread_mutex_unlock(&lock_mb);     /* libera lock_mb, uma vez que já manipulou num_macacos_MB */
      pthread_mutex_unlock(&lock_st);		  /* libera lock starvation para possibilitar novos macacos ou gorilas */

	    printf("Macaco %d passado de B para A \n", i);    /* relata qual macaco realizou passagem de B para A (região crítica) */
	    sleep(1);

	    /* Procedimentos para quando sair da corda */
      pthread_mutex_lock(&lock_mb);		/* pega lock_ma para manipular variável compartilhada num_macacos_MB (garante exclusão mútua entre macacos MB) */
			  num_macacos_MB--;			/* ocorre decremento do número total de macacos MB */
		    if(num_macacos_MB == 0){ 		/* caso seja o último processo a executar a função, */
		      pthread_mutex_unlock(&lock_ponte);		/* libera lock de acesso a ponte */
		    }
      pthread_mutex_unlock(&lock_mb);		/* libera lock_mb, uma vez que já manipulou num_macacos_MB */

    }

    pthread_exit(0);    /* encerra processo */
}

void * gorilaAB(void * a){
    while(1){     /* repete para sempre */
      RANDOMIZER;
	    /* Procedimentos para acessar a corda */
      printf("Gorila deseja passar de A para B \n");
      pthread_mutex_lock(&lock_st);		/* gorila_ab pega lock_st para garantir acesso a ponte (garante que não haverá starvation de gorilas) */	
        	pthread_mutex_lock(&lock_ponte);		/* pega lock para acesso a ponte (garante exclusão mútua, de modo que não haverá múltiplos gorilas ao mesmo tempo) */
		  pthread_mutex_unlock(&lock_st);		/* libera lock starvation para possibilitar novos macacos ou gorilas */

	    printf("Gorila passado de A para B \n");    /* relata quando gorila realizar passagem de A para B (região crítica) */
    	sleep(5);

      /* Procedimentos para quando sair da corda */
      pthread_mutex_unlock(&lock_ponte);    /* libera lock de acesso a ponte */

    }

    pthread_exit(0);    /* encerra processo */
}

void * gorilaBA(void * a){
    while(1){     /* repete para sempre */
      RANDOMIZER;
	    /* Procedimentos para acessar a corda */
      printf("Gorila deseja passar de B para A \n");
      pthread_mutex_lock(&lock_st);		/* gorila_ba pega lock_st para garantir acesso a ponte (garante que não haverá starvation de gorilas) */	
        	pthread_mutex_lock(&lock_ponte);		/* pega lock para acesso a ponte (garante exclusão mútua, de modo que não haverá múltiplos gorilas ao mesmo tempo) */
		  pthread_mutex_unlock(&lock_st);		/* libera lock starvation para possibilitar novos macacos ou gorilas */

	    printf("Gorila passado de B para A \n");
    	sleep(5);

      /* Procedimentos para quando sair da corda */
      pthread_mutex_unlock(&lock_ponte);    /* libera lock de acesso a ponte */

    }
    pthread_exit(0);    /* encerra processo */
}


int main(int argc, char * argv[])
{
    pthread_t macacos[MA+MB];
    int *id;
    int i = 0;
    for(i = 0; i < MA+MB; i++){
      id = (int *) malloc(sizeof(int));
      *id = i;
      if(i%2 == 0){
        if(pthread_create(&macacos[i], NULL, &macacoAB, (void*)id)){
          printf("Não pode criar a thread %d\n", i);
          return -1;
        }
      }else{
        if(pthread_create(&macacos[i], NULL, &macacoBA, (void*)id)){
          printf("Não pode criar a thread %d\n", i);
          return -1;
        }
      }
    }

    pthread_t gorila_ab;
    pthread_t gorila_ba;

    pthread_create(&gorila_ab, NULL, &gorilaAB, NULL);
    pthread_create(&gorila_ba, NULL, &gorilaBA, NULL);

    pthread_join(macacos[0], NULL);

    return 0;
}
