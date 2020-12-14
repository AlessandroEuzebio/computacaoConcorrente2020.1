#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

float *matrizA, *matrizB, *matrizC;
int qtd_threads;

typedef struct{
    int id;
    int dimensao;
}tArgs;

void *multiplicaMat(void *arg){
    tArgs *args = (tArgs *)arg;

    //multiplica as matrizes
    for (int i = args->id; i < args->dimensao; i += qtd_threads){
        for (int j = 0; j < args->dimensao; j++){
            for (int k = 0; k < args->dimensao; k++){
                matrizC[i * args->dimensao + j] += matrizA[i * args->dimensao + k] * matrizB[k * args->dimensao + j];
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    pthread_t *tid;
    tArgs *args;
    int dim; //dimensao das matrizes
    double inicio, fim, delta; //variaveis de medicao de tempo

    if (argc < 3){
        printf("Digite %s <quantidade de threads> <dimensao das matrizes>", argv[0]);
        return 1;
    }
    qtd_threads = atoi(argv[1]);
    dim = atoi(argv[2]);

    //alocacao de memoria para as estruturas
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

    //inicializar as estruturas
    for (int i = 0; i < dim; i++){
        for (int j = 0; j < dim; j++){
            matrizA[i * dim + j] = 1;
            matrizB[i * dim + j] = 1;
            matrizC[i * dim + j] = 0;
        }
    }

    GET_TIME(inicio);
    //alocando espaco para as threads
    tid = (pthread_t *)malloc(sizeof(pthread_t) * qtd_threads);
    if (tid == NULL){
        printf("ERRO--malloc");
        return 1;
    }

    //alocando espaco para as estruturas
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
    printf("Tempo: %lf\n", delta);

    //libera a memoria
    free(matrizA);
    free(matrizB);
    free(matrizC);

    return 0;
}
