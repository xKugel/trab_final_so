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
    sem_t mutex;
    sem_t mutex_sono;
    int mochila[N];
    int quantidade_mensagens;
    int posicao_mochila;
};

// Funções para um timeout pequeno e aleatório.
void dorme_aleatorio();
void leva_mochila_ate_B_e_volta();

// Só um método que exibe a mensagem que foi gerada e atualiza o número depois.
int escreve(int *quantidade_mensagens);

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

int escreve(int *quantidade_mensagens) {
    printf("Escrita a mensagem %i\n", ++*quantidade_mensagens);
    return *quantidade_mensagens;
};

void ColaPostIt(int (*mochila)[N], int *posicao_mochila, int mensagem){
    *mochila[*posicao_mochila] = mensagem;
    *posicao_mochila = (*posicao_mochila + 1)%N;
}

void *usuario(void *arguments) {
    struct args_struct *args = arguments;
    while(1) {
        dorme_aleatorio();
        int mensagem = escreve(&args->quantidade_mensagens);
        // Travo o mutex para acessar a região critica (bolsa do pombo).
        sem_wait(&args->mutex);
        ColaPostIt(&args->mochila, &args->posicao_mochila, mensagem);
        
        contPostIt++;
        // Destravo o mutex para outros acessarem a região.
		sem_post(&args->mutex);
        if(contPostIt == N) {
            printf("mochila cheia %i\n", contPostIt);
            // Destravo/Acordo o pombo.
			sem_post(&args->mutex_sono);
        }
    }
	pthread_exit(NULL);
}

void *pombo(void *arguments) {
    struct args_struct *args = arguments;
    while(1) {
        // Travo todo tipo de acesso a região critica assim que disponivel.
        sem_wait(&args->mutex_sono);
        sem_wait(&args->mutex); 
        
        leva_mochila_ate_B_e_volta();
        contPostIt = 0;

        // Array auxiliar para me ajudar a percorrer os valores da mochila.
        int (*aux)[20] = &args->mochila;
        for (int i = 0; i<N; i++) {
            printf("Adic. msg %d\n", *(args->mochila)+i);
        }
        sem_post(&args->mutex); 
        printf("Voltou.\n");
    }
	pthread_exit(NULL);
}


int main () {
    // Mutex de Usuário e Pombo para acesso a Região Critica (Mochila).
    sem_t mutex; 
    // Mutex adicional e especifíco de Pombo, para gerenciar sua atividade.
    sem_t mutex_sono;

    sem_init(&mutex, 0 , 1);
    sem_init(&mutex_sono, 0, 0);

    // Crio uma estrutura para informar os Mutexs para as Threads de forma mais simples.
    struct args_struct arguments;
    arguments.mutex = mutex;
    arguments.mutex_sono = mutex_sono;
    arguments.quantidade_mensagens = 0;
    arguments.posicao_mochila = 0;

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

    for(int aux = 0; aux < N_USUARIOS; aux++) {
        pthread_join(thread_usuario[aux], NULL);
    }
    
    pthread_join(threahd_pombo, NULL);
  
    return 0;
};