#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

float *matrizA, *matrizB, *matrizC;
int qtd_threads;//quantidade de threads

typedef struct{
    int id;//id da thread
    int dimensao;//dimensao das matrizes
}tArgs;

//funcao executada pelas threads
void *multiplicaMat(void *arg){
    tArgs *args = (tArgs *)arg;

    //multiplicacao das matrizes
    for (int i = args->id; i < args->dimensao; i += qtd_threads){
        for (int j = 0; j < args->dimensao; j++){
            for (int k = 0; k < args->dimensao; k++){
                matrizC[i * args->dimensao + j] += matrizA[i * args->dimensao + k] * matrizB[k * args->dimensao + j]; //mesmo que C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    pthread_t *tid; //threads
    tArgs *args; //variavel que vai armazenar o id e a dimensao
    int dim; //dimensao das matrizes
    double inicio, fim, delta; //variaveis de medicao de tempo

    // verifica todos os argumentos foram passados via linha de comando
    if (argc < 3){
        printf("Digite %s <dimensao das matrizes> <quantidade de threads>", argv[0]);
        return 1;
    }
    dim = atoi(argv[1]);
    qtd_threads = atoi(argv[2]);

    //alocacao de memoria para as estruturas
    GET_TIME(inicio);
    matrizA = (float *)malloc(sizeof(float) * (dim * dim));
    if (matrizA == NULL){
        printf("ERRO--malloc");
        return 1;
    }
    matrizB = (float *)malloc(sizeof(float) * (dim * dim));
    if (matrizB == NULL){
        printf("ERRO--malloc");
        return 1;
    }
    matrizC = (float *)malloc(sizeof(float) * (dim * dim));
    if (matrizC == NULL){
        printf("ERRO--malloc");
        return 1;
    }

    //inicializacao das estruturas
    for (int i = 0; i < dim; i++){
        for (int j = 0; j < dim; j++){
            matrizA[i * dim + j] = 1;
            matrizB[i * dim + j] = 1;
            matrizC[i * dim + j] = 0;
        }
    }
    GET_TIME(fim);
    delta = fim - inicio;
    printf("Tempo de alocacao e inicializacao das estruturas: %lf\n", delta);

    GET_TIME(inicio);
    //alocacao de espaco para as threads
    tid = (pthread_t *)malloc(sizeof(pthread_t) * qtd_threads);
    if (tid == NULL){
        printf("ERRO--malloc");
        return 1;
    }

    //alocacao de espaco para as estruturas
    args = (tArgs *)malloc(sizeof(tArgs) * qtd_threads);
    if (args == NULL){
        printf("ERRO--malloc");
        return 1;
    }

    //criando as threads
    for (int i = 0; i < qtd_threads; i++){
        (args + i)->id = i;
        (args + i)->dimensao = dim;
        if (pthread_create(tid + i, NULL, multiplicaMat, (void *)(args + i))){
            printf("Erro--pthread_create");
            return 1;
        }
    }
    
    for (int i = 0; i < qtd_threads; i++){
        pthread_join(*(tid + i), NULL);
    }

    GET_TIME(fim);
    delta = fim - inicio;
    printf("Tempo criacao das threads, execucao da multiplicacao e termino das threads: %lf\n", delta);

    GET_TIME(inicio);
    //liberar a memoria
    free(matrizA);
    free(matrizB);
    free(matrizC);
    GET_TIME(fim);
    delta = fim - inicio;
    printf("Tempo de finalizacao do programa: %lf\n", delta);

    return 0;
}
