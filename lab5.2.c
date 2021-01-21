#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>

#define THREADS_LEITORAS 2 // quantidade de threads leitoras
#define THREADS_ESCRITORAS 2 // quantidade de threads escritoras

int vetor[5]; // vetor de 5 elementos para facilitar a vizualização
int leitura = 0, escrita = 0; // variáveis de estado (variam entre zero e um)
int filaEscrita = 0; // conta quantos escritores estão esperando


// variáveis de sincronização
pthread_mutex_t mutex;
pthread_cond_t cond_leitura, cond_escrita;

// função que imprime o vetor e calcula a media
void leituraVetor(int id){
    float media = 0;
    printf("L[%d]Vetor", id);
    for(int i=0; i<5; i++){
        printf("%d ", vetor[i]);
        media += vetor[i];
    }
    printf("\n");
    printf("Media do vetor = %f\n", media);
}

// função que faz a escrita no vetor
void escritaVetor(int id){
    vetor[0] = id;
    vetor[4] = id;
    for(int i=1; i<4; i++){
        vetor[i] = 2 * id;
    }
    printf("E[%d] - vetor modificado", id);
    for(int i=0; i<5; i++) // imprime o vetor após a leitura
        printf("%d ", vetor[i]);
    printf("\n");
}

// função que inicializa o vetor com zeros
void inicializaVetor(){
    for(int i=0; i<5; i++)
        vetor[i] = 0;
}

// função que inicia a leitura
void entraLeitura(int id){
    pthread_mutex_lock(&mutex);
    printf("L[%d] quer ler", id);
    while(filaEscrita > 0 || escrita > 0){ // se existir algum escritor na fila ou escrevedo, a leitura não começa
        printf("L[%d] bloqueou\n", id);
        pthread_cond_wait(&cond_leitura, &mutex);
        printf("L[%d] desbloqueou\n", id);
    }
    leitura++;
    pthread_mutex_unlock(&mutex);
}

// função que termina a leitura
void saiLeitura(int id){
    pthread_mutex_lock(&mutex);
    printf("L[%d] terminou de ler\n", id);
    leitura--;
    if(filaEscrita > 0) // se existir alguma thread de escrita bloqueada ela é liberada
        pthread_cond_signal(&cond_escrita);

    pthread_mutex_unlock(&mutex);    
}

// thread leitora
void *leitor(void *args){
    int *id = (int*)args;
    while(1){
        entraLeitura(*id);
        leituraVetor(*id);
        saiLeitura(*id);
        sleep(1);
    }

    pthread_exit(NULL);
}

void entraEscrita(int id){
    pthread_mutex_lock(&mutex);
    printf("E[%d] quer escrever\n", id);
    while((leitura > 0) || (escrita > 0)){ // caso exista alguma leitura ou escrita ativa a thread é bloqueada
        printf("E[%d] entrou para a fila de escrita\n", id);
        filaEscrita++;
        pthread_cond_wait(&cond_escrita, &mutex);
        filaEscrita--;
        printf("E[%d] desbloqueou\n", id);
    }
    escrita++;
    pthread_mutex_unlock(&mutex);
}

void saiEscrita(int id){
    pthread_mutex_lock(&mutex);
    printf("E[%d] terminou de escrever\n", id);
    escrita--;
    if(filaEscrita > 0) // se existirem escritores bloqueados libera apenas um escritor
        pthread_cond_signal(&cond_escrita); //sinaliza um escritor  
    else // só desbloqueia as leitoras se não existir nenhum escritor bloqueado
        pthread_cond_broadcast(&cond_leitura); // desbloqueia todas as leitoras
    pthread_mutex_unlock(&mutex);
}

// thread escritora
void *escritor(void *args){
    int *id = (int*)args;
    while(1){
        entraEscrita(*id);
        escritaVetor(*id);    
        saiEscrita(*id);
        sleep(1);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]){

    pthread_t tid[THREADS_LEITORAS + THREADS_ESCRITORAS];
    int id[THREADS_LEITORAS + THREADS_ESCRITORAS];

    //inicializa as variaveis de sincronizacao
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_leitura, NULL);
    pthread_cond_init(&cond_escrita, NULL);


    for(int i=0; i<THREADS_LEITORAS; i++){
        id[i] = i + 1;
        if(pthread_create(&tid[i], NULL, leitor, (void*)&id[i])){
            printf("Erro--pthread_create");
            return 1;
        }
    }

    for(int i=0; i<THREADS_ESCRITORAS; i++){
        id[i+THREADS_LEITORAS] = i + 1;
        if(pthread_create(&tid[i+THREADS_LEITORAS], NULL, escritor, (void*)&id[i+THREADS_LEITORAS])){
            printf("Erro--pthread_create");
            return 1;
        }
    }

    pthread_exit(NULL);
    return 0;
}
