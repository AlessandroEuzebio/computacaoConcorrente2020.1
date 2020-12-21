#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

int qtd_threads; //quantidade de threads
int N; //quantidade de elementos da serie

void *calculaPI(void* arg){
    long int id = (long int) arg; //id da thread
    double *pi_aux = 0; //bloco da serie
    long int inicio = N/qtd_threads * id;
    long int fim;

    //alocacao de memoria para a variavel do bloco
    pi_aux = (double*)malloc(sizeof(double));
    if(pi_aux == NULL){
        printf("Erro--malloc");
        exit(1);
    }

    if(id == qtd_threads-1)
        fim = N;
    else
        fim = inicio + N/qtd_threads; //se for a ultima thread

    //calculo do bloco
    for(long int i=inicio; i<fim; i++){
        *pi_aux += (double)1/(1 + 2*i) * pow(-1,i);
    }
    pthread_exit((void*)pi_aux);
}

int main(int argc, char *argv[]){
    pthread_t *tid; //ponteiro para as threads
    double pi_seq = 0, pi_conc = 0, tempo_inicio, tempo_fim;
    double *retorno_threads; //valor que sera retornado pelas threads

    //verifica se o usuario digitou todos os argumentos
    if(argc < 3){
        printf("Digite %s <numero de elementos da serie> <quantidade de threads>\n", argv[0]);
        return 1;
    }
    N = atoll(argv[1]);
    qtd_threads = atoi(argv[2]);

    //calculo do pi sequencial
    GET_TIME(tempo_inicio);
    for(long int i=0; i<N; i++){
        pi_seq += (double)1/(1 + 2*i) * pow(-1,i);      
    }
    pi_seq *= 4;
    GET_TIME(tempo_fim);
    printf("Tempo sequencial: %lf\n", tempo_fim - tempo_inicio);
    printf("Valor de pi sequencial: %.15lf\n", pi_seq);

    //calculo do pi concorrente
    GET_TIME(tempo_inicio);

    //alocando memoria para as threads
    tid = (pthread_t*)malloc(sizeof(pthread_t) * qtd_threads);
    if(tid == NULL){
        printf("Erro--malloc\n");
        return 2;
    }
    //criando as threads
    for(long int i=0; i<qtd_threads; i++){
        if(pthread_create(tid+i, NULL, calculaPI, (void*)i)){
            printf("Erro--pthread_create\n");
            return 3;
        }
    }

    for(int i=0; i<qtd_threads; i++){
        if(pthread_join(*(tid+i), (void**) &retorno_threads)){
            printf("Erro--pthread_join\n");
            return 4;
        }
        pi_conc += *retorno_threads;
    }
    pi_conc *= 4;
    GET_TIME(tempo_fim);
    printf("Tempo concorrente: %lf\n", tempo_fim - tempo_inicio);
    printf("Valor de pi concorrente: %.15lf\n", pi_conc);
    printf("Valor da constante PI em math.h: %.15lf\n", M_PI);

    free(tid);

    return 0;
}
