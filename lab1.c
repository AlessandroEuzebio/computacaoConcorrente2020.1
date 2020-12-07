#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 2 // quantidade de threads

//referencia global para o vetor
int *ptr_vetor;

//estrutura que vai ter o identificador, inicio e o fim do vetor
typedef struct{
    int id, inicio, fim;
} t_Limits;


void *tarefa(void *arg){
    t_Limits *args = (t_Limits*)arg;

    int *ptr_aux = ptr_vetor + (args->inicio);
    for (int i = args->inicio; i < args->fim; i++){
        printf("Thread %d alterando o valor do elemento %d\n", args->id+1, i);
        *ptr_aux = *ptr_aux + 1;
        ptr_aux++;
    }
    pthread_exit(NULL);
}

int main(void){

    pthread_t tid_sistema[NTHREADS]; // 2 threads
    t_Limits *limites; // referencia para os argumentos das threads
    int tamanhoVetor; // tamanho do vetor que sera inserido pelo usuario

    printf("Digite o tamanho do vetor(10 < N < 100): ");
    scanf("%d", &tamanhoVetor);

    // alocacao de memoria para o vetor
    if((ptr_vetor = (int*)malloc(tamanhoVetor * sizeof(int))) == NULL){
        printf("Erro ao alocar memória para o vetor");
        return 1;
    }

    // populando o vetor com inteiros aleatorios
    int *ptr_aux = ptr_vetor; // ponteiro auxiliar
    for (int i = 0; i < tamanhoVetor; i++){
        *ptr_aux = rand() % 10;
        ptr_aux++;
    }

    // imprime o vetor antes do processamento
    for (int i = 0; i < tamanhoVetor; i++){
        printf("%d ", (*ptr_vetor) + i * 4);
    }
    printf("\n");


    // criacao das threads
    for (int thread = 0; thread < NTHREADS; thread++){
        // alocando espaco para as estruturas que serão passadas como argumento para as threads
        if((limites = (t_Limits*)malloc(sizeof(t_Limits))) == NULL){
            printf("Erro ao alocar memória para o struct de dados\n");
            return 1;
        }

        //preenchendo os valores dos argumentos da thread
        limites-> id = thread; // identificador da thread
        if((tamanhoVetor % 2) == 0){ // vetor com tamanho par
            limites->inicio = thread * (tamanhoVetor/2); //onde a thread vai comecar a processar o vetor
            limites->fim = thread * (tamanhoVetor/2) + tamanhoVetor/2;//onde a thread vai terminar o processamento
        }
        else if((tamanhoVetor % 2) != 0){ //vetor com tamanho ímpar
            limites->inicio = thread * (tamanhoVetor/2); //onde a thread vai comecar a processar o vetor
            limites->fim = thread * (tamanhoVetor/2) + tamanhoVetor/2 + thread;//onde a thread vai terminar o processamento
        }

        //criando as threads
        if(pthread_create(&tid_sistema[thread], NULL, tarefa, (void*)limites)){
            printf("Erro ao criar a thread\n");
            return 1;
        }
    }

    //espera as threads terminarem
     for(int thread = 0; thread < NTHREADS; thread++){
         if(pthread_join(tid_sistema[thread], NULL)){
             printf("--ERRO: pthread_join() \n");
             return 1;
         }
     }

    // imprime o vetor depois do processamento
    for (int i = 0; i < tamanhoVetor; i++){
        printf("%d ", (*ptr_vetor) + i * 4);
    }
    printf("\n");
   
    return 0;
}
