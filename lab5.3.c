#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>

#define THREADS_LEITORAS 2 // quantidade de threads leitoras
#define THREADS_ESCRITORAS 2 // quantidade de threads escritoras

int vetor[5]; // vetor de 5 elementos para facilitar a vizualização
int leitura = 0, escrita = 0; // variáveis de estado (variam entre zero e um)

// variáveis que vao servir como filas
// as filas ativas referenciam a quantidade de escritores ou leitores que serão executados em certo momento
// as filas de espera referenciam a quantidade de escritores ou leitores esperando para serem executados
int filaAtivaEscrita = 0, filaEsperaEscrita = 0;
int filaAtivaLeitura, filaEsperaLeitura;


// variáveis de sincronização
pthread_mutex_t mutex;
pthread_cond_t cond_leitura, cond_escrita;

// função que imprime o vetor e calcula a media
void leituraVetor(){
    float media = 0;
    printf("Vetor");
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
    // caso exista alguma escrita escrevendo ou uma fila ativa de leitores
    // ou escritores, a thread leitora vai se bloquear e entrar para a fila de espera
    while(escrita > 0 || filaAtivaLeitura > 0 || filaAtivaEscrita > 0){
        printf("L[%d] entrou para a fila de espera\n", id);
        filaEsperaLeitura++;
        pthread_cond_wait(&cond_leitura, &mutex);
        printf("L[%d] desbloqueou\n", id);
    }
    leitura++;
    pthread_mutex_unlock(&mutex);
}

// função que termina a leitura
void saiLeitura(int id){
    pthread_mutex_lock(&mutex);
    leitura--;
    printf("L[%d] terminou de ler\n", id);
    if(filaAtivaLeitura > 1) // verifica se ainda existem leitoras na fila ativa, se tiver decrementa a fila
        filaAtivaLeitura--;
    else if(filaEsperaEscrita > 0){ // se a thread for a última leitora da fila ativa, agora muda a execução para a fila ativa de escritoras
        printf("Iniciando fila de Escritas\n");
        filaAtivaEscrita = filaEsperaEscrita;
        filaEsperaEscrita = 0;
        pthread_cond_signal(&cond_escrita);
    }
    else{ // se não houverem escritores em espera libera escritoras na fila de espera
        pthread_cond_broadcast(&cond_leitura);
        filaEsperaLeitura = 0;
    }
    pthread_mutex_unlock(&mutex);    
}

// thread leitora
void *leitor(void *args){
    int *id = (int*)args;
    while(1){
        entraLeitura(*id);
        leituraVetor();
        saiLeitura(*id);
        sleep(1);
    }

    pthread_exit(NULL);
}

void entraEscrita(int id){
    pthread_mutex_lock(&mutex);
    printf("E[%d] quer escrever\n", id);
    // caso exista alguma leitora ou escritora ativa ou uma fila ativa de escritoras ou leitoras, a thread será bloqueada
    while((leitura > 0) || (escrita > 0) || (filaAtivaEscrita > 0) || (filaAtivaLeitura > 0)){
        printf("E[%d] entrou para a fila de espera\n", id);
        filaEsperaEscrita++;
        pthread_cond_wait(&cond_escrita, &mutex);
        printf("E[%d] desbloqueou\n", id);
    }
    escrita++;
    pthread_mutex_unlock(&mutex);
}

void saiEscrita(int id){
    pthread_mutex_lock(&mutex);
    printf("E[%d] terminou de escrever\n", id);
    escrita--;


    if(filaAtivaEscrita > 1){ // caso ainda existam threads na fila ativa, desbloquear a próxima e liberar um espaço
        filaAtivaEscrita--;
        pthread_cond_signal(&cond_escrita);
    }
    else if(filaEsperaLeitura > 0){ // se essa for a última thread escritora e existir uma fila de espera de leitoras, chavear para leitura
        filaAtivaLeitura = filaEsperaLeitura;
        filaEsperaLeitura = 0;
        pthread_cond_broadcast(&cond_leitura); // como leitoras pode executar ao mesmo tempo, liberar todas as leitoras
    }
    else{ // caso a fila ativa termine e não existam leitoras esperando liberar uma escrita
        pthread_cond_signal(&cond_escrita);
        filaEsperaEscrita--;
    }
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

    // cria as threads leitoras
    for(int i=0; i<THREADS_LEITORAS; i++){
        id[i] = i + 1;
        if(pthread_create(&tid[i], NULL, leitor, (void*)&id[i])){
            printf("Erro--pthread_create");
            return 1;
        }
    }

    // cria as threads escritoras
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
