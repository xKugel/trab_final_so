/* Atividade 2 - Trabalho Final de SO
 *
 * Equipe - Vagus Dream
 * Gabriel Felipe 234782
 * Victor Calebe 194664
  *
 * Para execução de programa basta rodas os seguintes comando:
 * $ gcc -pthread trab.c -o <nome_do_executavel>
 * $ ./<nome_do_executavel>
 */

// Inclusão das Bibliotecas necessárias.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Define a contantes de mensagens, e um Contador de Post It.
#define N 20
int contPostIt = 0;


// Criação de uma struct para passar multiplos argumentos para as Threads.
struct args_struct {
    sem_t mutex_mochila;
    sem_t mutex_sono;
    sem_t mutex_escrita;    
    int mochila[N];
    int quantidade_mensagens;
    int posicao_mochila;
};

// Funções para um timeout pequeno e aleatório.
void dorme_aleatorio();
void leva_mochila_ate_B_e_volta();

// Só um método que atualiza o número de mensagens e exibe a mensagem que foi gerada depois.
void escreve(int *quantidade_mensagens);

// Recebe uma mensagem e cola ela, cola ela na mochila e atualiza a proxima posição de mensagem.
void ColaPostIt(int (*mochila)[N], int *posicao_mochila, int mensagem);

// Define as ações executadas pelas Threads de usuários.
void *usuario(void *arguments);

// Define as ações executadas pela Thread pombo.
void *pombo(void *arguments);


void leva_mochila_ate_B_e_volta() {
    printf("Levando a Mochila até B.\n");
	sleep((rand() % 3)+1);
}

void dorme_aleatorio() {
    printf("Dormindo.\n");
	sleep((rand() % 3)+1);
    printf("Acordou.\n");
}

void escreve(int *quantidade_mensagens) {
    *quantidade_mensagens += 1;
    printf("Escrita a mensagem %i\n", *quantidade_mensagens);
};

void ColaPostIt(int (*mochila)[N], int *posicao_mochila, int mensagem){
    printf("\nColada mensagem %i\n", mensagem);
    *mochila[*posicao_mochila] = mensagem;
    *posicao_mochila = (*posicao_mochila + 1)%N;
}

void *usuario(void *arguments) {
    struct args_struct *args = arguments;
    while(1) {
        dorme_aleatorio();
        // To usando um mutex aqui pra travar a escrita na quantidade de mensagens,
        // porque tive problemas com sobreescrita antes da hora.
        sem_wait(&args->mutex_escrita);
        escreve(&args->quantidade_mensagens);
        

        // Travo o mutex para acessar a região critica (mochila).
        sem_wait(&args->mutex_mochila);

        ColaPostIt(&args->mochila, &args->posicao_mochila, args->quantidade_mensagens);
        // Libero a reescrita da quantidade de mensagens
        sem_post(&args->mutex_escrita);
        
        contPostIt++;
        // Destravo o mutex para outros acessarem a região.
		sem_post(&args->mutex_mochila);
        if(contPostIt == N) {
            printf("mochila cheia %i\n", contPostIt);
            // Destravo/Acordo o pombo.
			sem_post(&args->mutex_sono);
        }
    }
}

void *pombo(void *arguments) {
    struct args_struct *args = arguments;
    while(1) {
        // Travo os acesso a região critica da mochila.
        sem_wait(&args->mutex_sono);
        sem_wait(&args->mutex_mochila); 
        
        leva_mochila_ate_B_e_volta();
        contPostIt = 0;

        for (int i = 0; i<N; i++) {
            printf("Adic. msg %d\n", *(args->mochila)+i);
        }
        sem_post(&args->mutex_mochila); 
        printf("Voltou.\n");
    }
}


int main () {

    // Crio uma estrutura para informar os Mutexs para as Threads de forma mais simples.
    struct args_struct arguments;
    arguments.quantidade_mensagens = 0;
    arguments.posicao_mochila = 0;

    //Inicío o mutex para acesso a variável da mochila.
    sem_init(&arguments.mutex_mochila, 0 , 1);
    //Inicío o mutex para o sono da pomba.
    sem_init(&arguments.mutex_sono, 0, 0);
    //Inicío o mutex para acesso a variável da mensagem.
    sem_init(&arguments.mutex_escrita, 0, 1);

    // Inicializo o gerador de número aleatório.
    srand(time(NULL));

    // Leio o número de Usuários que o Usuário informar.
    int N_USUARIOS;
    printf("Digite a quantidade de Usuários:\n");
    scanf("%d", &N_USUARIOS);
    
    pthread_t threahd_pombo, thread_usuario[N_USUARIOS];

    // Cria N_USUARIOS threads para os Usuários.
    for(int aux = 0; aux < N_USUARIOS; aux++) {
        pthread_create(&thread_usuario[aux], NULL, usuario, (void *)&arguments);
    }
    // Cria threads para o Pombo.
    pthread_create(&threahd_pombo, NULL, pombo, (void *)&arguments);

    // Dou join nas threads para finaliza-las.
    for(int aux = 0; aux < N_USUARIOS; aux++) {
        pthread_join(thread_usuario[aux], NULL);
    }
    
    pthread_join(threahd_pombo, NULL);
  
    return 0;
};