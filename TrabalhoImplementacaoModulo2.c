#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

//estrutura que vai armazenar os limites do vetor e dos subvetores
//passados para as threads além de um ponteiro para a próxima estrutura na fila
typedef struct elemento{
    int esquerda, direita;
    struct elemento *proximo; //próximo elemento da fila
}Elemento;

int *vetor; //vetor que será ordenado
int tamanhoVetor; //tamanho do vetor
int qtd_threads; //quantidade de threads
int threadsInativas = 0; //quantidade de threads inativas

pthread_mutex_t lock; //variável de exclusão mútua
pthread_cond_t cond; //variável de controle

//variáveis de controle do primeiro e último elemento da fila
Elemento *primeiroElementoDaFila = NULL;
Elemento *ultimoElementoDaFila = NULL;


//-----funções auxiliares-----

// cria um elemento para a fila, posicionando-o no fim
Elemento *criaElementoFila(int esquerda, int direita){
    Elemento *elemento = (Elemento*)malloc(sizeof(Elemento));

    elemento->esquerda = esquerda;
    elemento->direita = direita;
    elemento->proximo = NULL;

    return elemento;
}

// adiciona um elemento na Fila
void adicionaFila(Elemento *elemento){
    if(ultimoElementoDaFila == NULL || primeiroElementoDaFila == NULL){
        primeiroElementoDaFila = elemento;
    }
    else{
        ultimoElementoDaFila->proximo = elemento;
    }
    ultimoElementoDaFila = elemento;
}

// retorna o primeiro elemento da fila e o atualiza para o seguinte
Elemento *removeFila(){
    Elemento *elemento = primeiroElementoDaFila;
    if(elemento == NULL)
        return NULL;
    primeiroElementoDaFila = primeiroElementoDaFila -> proximo;
    return elemento;
}

void swap(int a, int b){
    int aux = vetor[a];
    vetor[a] = vetor[b];
    vetor[b] = aux;
}


//assinatura das funções usadas pelo quicksort
int particiona(int *vetor, int esquerda, int direita);
void *quickSort();// será executada pelas threads
int verificadorDeOrdem(int *vetor);


int main(int argc, char *argv[]){
    double inicio, fim; //variáveis para medição do tempo de execução
    pthread_t tid[qtd_threads];

    //verifica se o usuário digitou corretamente os parâmetros da função main
    if(argc < 3){
        printf("Digite: %s <quantidade de elementos do vetor> <número de threads>", argv[0]);
        return 1;
    }
    tamanhoVetor = atoi(argv[1]);
    qtd_threads = atoi(argv[2]);

    if(qtd_threads <= 0 || tamanhoVetor <= 0){
        printf("Quantidade de threads ou tamanho do vetor inválido. Digite um inteiro maior que zero.");
        return 1;
    }

    //criação do vetor aleatório que será ordenado
    vetor = (int*)malloc(sizeof(int) * tamanhoVetor);
    if(vetor == NULL){
        printf("Erro--malloc");
        return 1;
    }
    //preenche o vetor com números aleatórios
    for(int i=0; i<tamanhoVetor; i++)
        vetor[i] = rand() % 1000;

    //imprime o vetor antes de chamar o quicksort
    printf("Vetor não ordenado:\n");
    for(int i=0; i<tamanhoVetor; i++)
        printf("%d ", vetor[i]);
    printf("\n");

    GET_TIME(inicio); //incício da medição do tempo de execução

    Elemento *e = criaElementoFila(0, tamanhoVetor - 1);//primeiro e útltimo elemento da fila nesse momento(vetor inteiro)
    adicionaFila(e);

    pthread_mutex_init(&lock, NULL); //inicializa a variável de exclusão mútua
    pthread_cond_init(&cond, NULL); //inicializa a variável de bloqueio por condição

    // criação das threads
    for(int i=0; i<qtd_threads; i++){
        if(pthread_create(&tid[i], NULL, quickSort, NULL)){
            printf("Erro--pthread_create.\n");
            return 1;
        }
    }

    for(int i=0; i<qtd_threads; i++){
        if(pthread_join(tid[i], NULL)){
            printf("Erro--pthread_join\n");
            return 1;
        }
    }

    // liberação das variáveis de exclusão mútua e de bloqueio por condição
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    GET_TIME(fim); // fim da medição de tempo

    //vetor após execução do quicksort
    int verifica = verificadorDeOrdem(vetor);
    if(verifica == 0){
        printf("Ordenação realizada corretamente\n");
    }
    printf("Vetor ordenado:\n");
    for(int i=0; i<tamanhoVetor; i++)
        printf("%d ", vetor[i]);
    printf("\n");

    printf("Tempo de decorrido: %lf\n", fim - inicio);

    return 0;
}

//-------funções do quicksort-------

//particionamento de Lomuto
int particiona(int *vetor, int esquerda, int direita){
    int pivo = vetor[esquerda];
    int i = esquerda;

    for(int j=esquerda + 1; j<=direita; j++){
        if (vetor[j] <= pivo){
            i++;
            swap(i, j);
        }
    }
    swap(esquerda, i);

    return i; //retorna o índice do pivô
}

int verificadorDeOrdem(int *vetor){
    for(int i = 0; i<tamanhoVetor - 1; i++){
        if(vetor[i] <= vetor[i+1]){
            continue;
        }
        else{
            return 1;
        }
    }
    return 0;
}

void *quickSort(){
    int estado = 0;//1 significa inativa e 0 ativa

    while(1){
        pthread_mutex_lock(&lock); //início da seção crítica (evita que mais de uma thread tente remover o mesmo elemento da fila)
        Elemento *elemento = removeFila(); //remove o primeiro elemento da fila e atribiu a elemento

        //caso não haja nenhum elemento na fila e a thread seja a última
        if((elemento == NULL) && (threadsInativas == qtd_threads - 1)){
            threadsInativas++;
            pthread_cond_broadcast(&cond); //desbloqueia as threads bloqueadas ---- evita o deadlock
            pthread_mutex_unlock(&lock); //sai da seção crítica
            pthread_exit(NULL); //encerra a thread
        }

        while(elemento == NULL){
            if(estado == 0){
                estado = 1; //thread inativa pois não tem o que processar
                threadsInativas++;
            }
            pthread_cond_wait(&cond, &lock); //bloqueia a thread
            if(threadsInativas == qtd_threads){ //se ao voltar do bloqueio não houverem threads ativas encerra a thread
                pthread_mutex_unlock(&lock); //evita deadlock
                pthread_exit(NULL); //encerra a thread
            }
            elemento = removeFila(); //remove novamente o primeiro elemento da fila
        }

        pthread_mutex_unlock(&lock); //sai da seção crítica

        if(elemento != NULL){ //caso em que existe elemento na fila
            if(estado == 1){ //verifica se a thread esta inativa e caso esteja muda seu estado
                pthread_mutex_lock(&lock); //seção crítica para decrementar a quantidade de threads ativas (variável global)
                estado = 0;
                threadsInativas--;
                pthread_mutex_unlock(&lock); //sai da seção crítica
            }

            int esquerda = elemento->esquerda; //esquerda recebe o índice inicial do subvetor
            int direita = elemento->direita; //direita recebe o índice final do subvetor
            free(elemento);

            if(esquerda < direita){
                //particiona() retorna o pivo baseado no vetor delimitado pelos índices esquerda e direita
                //e coloca todos os números menores ou iguais ao pivo à sua esquerda e os maiores à sua direita
                int pivo = particiona(vetor, esquerda, direita);

                //novos elementos que serão adicionados à fila
                //os limites dos novos subvetores são baseados no pivo e nos limites esquerdo e direito do vetor anterior
                Elemento *novoElementoEsq = criaElementoFila(esquerda, pivo - 1);
                if(novoElementoEsq == NULL){printf("Erro ao alocar memória para arg.\n"); exit(1);}

                Elemento *novoElementoDir = criaElementoFila(pivo + 1, direita);
                if(novoElementoDir == NULL){printf("Erro ao alocar memória para arg.\n"); exit(1);}


                pthread_mutex_lock(&lock); //inicia seção crítica

                //inclui os novos elementos na fila contendo os subvetores a serem ordenados
                adicionaFila(novoElementoEsq);
                adicionaFila(novoElementoDir);
                pthread_cond_signal(&cond);

                pthread_mutex_unlock(&lock); //sai da seção crítica
            }
        }
    }
}
