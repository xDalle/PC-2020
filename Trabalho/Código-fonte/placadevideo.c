/*

Lucas Dalle Rocha
17/0016641

*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

// Macros de constantes
#define NUM_USUARIOS 4							// Quantidade de usuários comuns.
#define NUM_MINERADORES 8						// Quantidade de mineradores.
#define NUM_PLACAS_PRODUZIDAS 10				// Quantidade de placas produzidas por vez pelo produtor.
#define NUM_PLACAS_CONSUMIDAS_MINERADORES 2		// Quantidade de placas consumidas por vez pelos mineradores.
#define NUM_CRIACAO_CENTRO 6					// Quantidade necessária para criação de um centro de mineração, por parte dos mineradores.
#define PRECO_PAGO_USUARIO 100					// Preço pago por placa pelo usuário comum.
#define PRECO_PAGO_MINERADOR 50					// Preço pago por placa pelo minerador.
#define DINHEIRO_GANHO_POR_CENTRO 500			// Dinheiro arrecadado a cada iteração da thread em função de cada centro de mineração possuído pelo minerador.

int dinheiro_produtor = 0;							// Variável compartilhada que armazena dinheiro arrecadado pelo produtor (impressa ao final da venda do NUM_PLACAS_PRODUZIDAS).
int qtd_placas_atual = 0;							// Variável compartilhada que armazena quantidade de placas ainda no estoque (atualiza conforme as vendas).
int dinheiro_minerador[NUM_MINERADORES] = {};		// Armazena dinheiro arrecadado através de centros de mineração de cada minerador.
int qtd_placas_minerador[NUM_MINERADORES] = {};		// Armazena a quantidade de placas possuídas por cada minerador, a fim de alcançar o NUM_CRIACAO_CENTRO.
int qtd_centro_mineracao[NUM_MINERADORES] = {};		// Armazena a quantidade de centros de mineração possuídos por cada minerador.

pthread_t produtor_placa;					// Thread do produtor das placas de vídeo.
pthread_t usuarios[NUM_USUARIOS];			// Threads dos usuários consumidores.
pthread_t mineradores[NUM_MINERADORES];		// Threads dos mineradores consumidores.
pthread_t construtor_centro;				// Thread do construtor dos centros de mineração.

pthread_mutex_t lock_estoque = PTHREAD_MUTEX_INITIALIZER;	// Lock de acesso ao estoque de placas disponíveis, compartilhado entre as threads.

pthread_cond_t produtor_cond = PTHREAD_COND_INITIALIZER;	// Variável condicional que controla fluxo da thread produtor (espera placas esgotarem).
pthread_cond_t usuario_cond = PTHREAD_COND_INITIALIZER;		// Variável condicional que controla fluxo das threads usuários (consome se há placas disponíveis).
pthread_cond_t minerador_cond = PTHREAD_COND_INITIALIZER;	// Variável condicional que controla fluxo das threads mineradores (consome se há placas disponíveis e prioriza usuários).

sem_t usuario_quer;			// Semáforo que incrementa de acordo com a necessidade de usuários por placas de vídeo.
sem_t centro_mineracao;		// Semáforo que incrementa quando mineradores possuem placas de vídeo o suficiente (NUM_CRIACAO_CENTRO) para criação do centro de mineração.

void *produtor(void *arg);		// Função da thread produtor.
void *usuario(void *arg);		// Função das threads usuários.
void *minerador(void *arg);		// Função das threads mineradores.
void *construtor(void *arg);	// Função da thread construtor.

int main(int argc, char *argv[]) {
    int *id, i;

    printf("\n\t* ---   Iniciando programa...   --- *\n\n");	// Apenas indica início do funcionamento do programa.

    sem_init(&usuario_quer, 0, 0);			// Inicializa com zero permissões (nenhum usuário necessita ainda).
    sem_init(&centro_mineracao, 0, 0);		// Inicializa com zero permissões (nenhum minerador possui o suficiente ainda).
	
    if(NUM_PLACAS_CONSUMIDAS_MINERADORES > NUM_PLACAS_PRODUZIDAS){	// Encerra programa caso número de placas consumidas por vez pelos mineradores for maior que o número produzido.
        printf("\tNumero de placas consumidas pelos mineradores maior do que o numero de placas produzidas! O programa sera encerrado.\n");
        return -1;
    }
	
	if(PRECO_PAGO_USUARIO < PRECO_PAGO_MINERADOR){	// Encerra programa caso preço pago pelo usuário por placa for menor do que o preço pago pelo minerador (prioridade).
		printf("\tUsuario deve pagar pela placa de video mais caro do que o minerador, para que tenha prioridade! O programa sera encerrado.\n");
        return -1;
	}

    if(pthread_create(&produtor_placa, NULL, produtor, NULL)){	// Cria thread do produtor e encerra programa em caso de erro.
        printf("\tErro ao criar thread produtor! O programa sera encerrado.\n");
        return -1;
    }

    for (i = 0; i < NUM_USUARIOS; i++) {	// Cria threads dos usuários e encerra programa em caso de erro.
        id = (int *) malloc(sizeof(int));
        *id = i;
        if(pthread_create(&usuarios[i], NULL, usuario, (void *)(id))){
            printf("\tErro ao criar thread usuario %d! O programa sera encerrado.\n", i);
            return -1;
        }
    }

    for (i = 0; i < NUM_MINERADORES; i++) {	// Cria threads dos mineradores e encerra programa em caso de erro.
        id = (int *) malloc(sizeof(int));
        *id = i;
        if(pthread_create(&mineradores[i], NULL, minerador, (void *)(id))){
            printf("\tErro ao criar thread minerador %d! O programa sera encerrado.\n", i);
            return -1;
        }
    }

    if(pthread_create(&construtor_centro, NULL, construtor, NULL)){	// Cria thread do construtor e encerra programa em caso de erro.
        printf("\tErro ao criar thread construtor! O programa sera encerrado.\n");
        return -1;
    }

    pthread_join(produtor_placa, NULL);		// Inicia sincronização...

    return 0;
}

void *produtor(void *arg){
    while(1){
        pthread_mutex_lock(&lock_estoque);		// Pega lock para acessar estoque das placas de vídeo.
            while(qtd_placas_atual != 0) {		// Caso o estoque não esteja vazio...
                pthread_cond_wait(&produtor_cond, &lock_estoque);	// Produtor paraliza o fluxo
            }
			
			printf("\tProdutor: acumulei %d reais no total.\n", dinheiro_produtor);		// Imprime quantidade total adquirida em cada iteração da thread.
            printf("\tProdutor: estou fabricando %d placas de video!\n", NUM_PLACAS_PRODUZIDAS);
			sleep(2);	// Sleep que simula tempo de produção das placas de vídeo.
            qtd_placas_atual = NUM_PLACAS_PRODUZIDAS;	// Quantidade atual recebe a quantidade de placas produzidas.

            printf("\tProdutor: acabei de fabricar as placas de video.\n");

            pthread_cond_broadcast(&usuario_cond);		// Acorda threads usuários, já que possui placas no estoque.
            pthread_cond_broadcast(&minerador_cond);	// Acorda threads mineradores, já que possui placas no estoque.
        pthread_mutex_unlock(&lock_estoque);	// Libera lock do estoque.
    }
}

void *usuario(void *arg){
    int i = *((int *) arg);		// Id do usuário.

    while(1){
        sleep(rand() % 10 + 5);		// Sleep aleatório que simula tempo em que usuários não desejam adquirir placas de vídeo.
		
		printf("\t\tUsuario %d: quero placa de video!\n", i);
		sem_post(&usuario_quer);	// Incrementa no semáforo a quantidade de usuários que deseja realizar compra.
		
        pthread_mutex_lock(&lock_estoque);		// Pega lock para acessar estoque das placas de vídeo.
            while(qtd_placas_atual == 0) {		// Enquanto não houver placas de vídeo no estoque...
                printf("\t\tUsuario %d: nao ha placas de video disponiveis, esperarei pelo produtor...\n", i);
                pthread_cond_signal(&produtor_cond);	// Acorda produtor.
                pthread_cond_wait(&usuario_cond, &lock_estoque);	// Adormece usuário.
            }

            qtd_placas_atual--;		// Decrementa quantidade atual de placas de vídeo no estoque.
			dinheiro_produtor += PRECO_PAGO_USUARIO;	// Incrementa dinheiro do produtor.
            printf("\t\tUsuario %d: consegui uma placa de video.\n", i);
            sem_wait(&usuario_quer);	// Decrementa semáforo a quantidade de usuários que deseja realizar compra.
        pthread_mutex_unlock(&lock_estoque);	// Libera lock do estoque.
    }
}

void *minerador(void *arg){
    int i = *((int *) arg);		// Id do minerador
    int qtd_usuario_quer;		// Variável que receberá valor do semáforo da quantidade de usuários que desejam adquirir placas de vídeo.

    while(1){
        sleep(rand() % 10 + 5);		// Sleep aleatório que simula tempo em que mineradores não desejam adquirir placas de vídeo.
		
		if(qtd_centro_mineracao[i] > 0){	// Caso minerador possua pelo menos um centro de mineração...
			dinheiro_minerador[i] += (qtd_centro_mineracao[i] * DINHEIRO_GANHO_POR_CENTRO);		// Incrementa dinheiro do minerador, baseado na quantidade de centros.
			if(qtd_centro_mineracao[i] == 1){	// Caso minerador possua exatamente um centro...
				printf("\t\tMinerador %d: ja ganhei %d reais com meu unico centro de mineracao.\n", i, dinheiro_minerador[i]);	// Imprime respectiva mensagem.
			}else{	// Caso possua mais de um centro...
				printf("\t\tMinerador %d: ja ganhei %d reais com meus %d centros de mineracao.\n", i, dinheiro_minerador[i], qtd_centro_mineracao[i]); // Imprime respectiva mensagem.
			}
		}
		
        pthread_mutex_lock(&lock_estoque);		// Pega lock para acessar estoque das placas de vídeo.
            sem_getvalue(&usuario_quer, &qtd_usuario_quer);		// Pega valor do semáforo da quantidade de usuários que desejam comprar placa.
            while((qtd_usuario_quer > 0) || (qtd_placas_atual < NUM_PLACAS_CONSUMIDAS_MINERADORES)){	// Enquanto não houver placas suficientes no estoque ou algum usuário quer... 
                if(qtd_usuario_quer > 0){
                    printf("\t\tMinerador %d: algum usuario necessita da placa de video, vou esperar...\n", i);
					pthread_cond_broadcast(&usuario_cond);	// Acorda usuários.
                }else if(qtd_placas_atual < NUM_PLACAS_CONSUMIDAS_MINERADORES){
                    printf("\t\tMinerador %d: nao ha placas de video suficientes no estoque, esperarei pelo produtor...\n", i);
					pthread_cond_signal(&produtor_cond);	// Acorda produtor.
                }
                pthread_cond_wait(&minerador_cond, &lock_estoque);	// Adormece minerador.
				sem_getvalue(&usuario_quer, &qtd_usuario_quer);		// Pega valor do semáforo para próxima iteração do loop while.
            }
			
			// Baseado na quantidade de placas que mineradores consomem e quanto pagam por cada placa de vídeo...
            qtd_placas_atual -= NUM_PLACAS_CONSUMIDAS_MINERADORES;	// Decrementa quantidade atual de placas de vídeo no estoque.
			dinheiro_produtor += (PRECO_PAGO_MINERADOR * NUM_PLACAS_CONSUMIDAS_MINERADORES);	// Incrementa dinheiro do produtor. 
            qtd_placas_minerador[i] += NUM_PLACAS_CONSUMIDAS_MINERADORES;	// Incrementa quantidade de placas de vídeo possuídas pelo minerador.

            if(NUM_PLACAS_CONSUMIDAS_MINERADORES == 1){		// Caso mineradores só consumam uma placa...
                printf("\t\tMinerador %d: consegui %d placa de video.\n", i, NUM_PLACAS_CONSUMIDAS_MINERADORES);	// Imprime respectiva mensagem.
            }else{	// Caso mineradores consumam mais de uma placa...
                printf("\t\tMinerador %d: consegui %d placas de video.\n", i, NUM_PLACAS_CONSUMIDAS_MINERADORES);	// Imprime respectiva mensagem.
            }
        pthread_mutex_unlock(&lock_estoque);	// Libera lock do estoque.

        if(qtd_placas_minerador[i] >= NUM_CRIACAO_CENTRO){		// Caso minerador possua placas suficientes para criação do centro de mineração...
            printf("\t\tMinerador %d: possuo o suficiente para criar um centro de mineracao, chamando construtor...\n", i);
            qtd_placas_minerador[i] -= NUM_CRIACAO_CENTRO;		// Decrementa quantidade de placas de vídeo do minerador.
            sem_post(&centro_mineracao);	// Incrementa semáforo do centro de mineração.
			qtd_centro_mineracao[i]++;		// Incrementa quantidade de centros de mineração do minerador.
        }
    }
}

void *construtor(void *arg){
    int total_centro_mineracao = 0;		// Quantidade total de centros de mineração produzidos pelo construtor.
	printf("\tConstrutor: preciso de %d placas de video para criacao de centro de mineracao.\n", NUM_CRIACAO_CENTRO);
	
    while(1){
        sem_wait(&centro_mineracao);	// Espera permissão para construir centro (qtd_placas_minerador[i] >= NUM_CRIACAO_CENTRO).
		printf("\tConstrutor: vou construir o centro de mineracao!\n");
		sleep(2);	// Sleep que simula construção do centro.
        total_centro_mineracao++;	// Incrementa quantidade total de centros construídos.
        printf("\tConstrutor: acabei de fazer um centro de mineracao. Ja construi %d no total.\n", total_centro_mineracao);
    }
}