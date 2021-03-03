#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"

//declaração das variaveis globais

int *buffer, *estruturaDeControle, M, N, count = 0;
long long int *qtd_elementos; //primeiro elemento do arquivo a ser lido
FILE *ptr_file;

//thread buscaMaiorSequencia
int elemento_maiorSequencia = -1;
int qtd_maiorSequencia = 0;
long long int posicao = 0;

//thread contaTriplas
int elemento_buscaTriplas = -1;
int qtd_contaTriplas = 1;
int qtd_triplas = 0;

//thread buscaSequencia
int sequencia = 0;
int qtd_sequencia = 0;

//variaveis de sincronização mútua
pthread_mutex_t mutex;
pthread_cond_t cond_Buffer, cond_buscaMaiorSequencia, cond_buscaTriplas, cond_buscaSequencia;

void imprimeBuffer(){
    for(int i=0; i<M; i++){
        for (int j=0; j<N; j++){
            printf("%d ", *(buffer + i*N + j));
        }
        printf("\n");
    }
}

void imprimeEstruturaDeControle(){
    for(int i=0; i<M; i++){
        for (int j=0; j<4; j++){
            printf("%d ", *(buffer + i*4 + j));
        }
        printf("\n");
    }
}

//thread busca maior sequencia
void *buscaMaiorSequencia(){
    int posicaoBuffer = 0, qtd_elementosNoBloco;
    long long int numerosProcessados = 0;
    while(1){
        while(*(estruturaDeControle + posicaoBuffer*4 + 1) == 0){
            printf("buscaMaiorSequencia BLOQUEADA\n - bloco vazio ou já processado pela thread buscaMaiorSequencia.\n"); 
            pthread_cond_wait(&cond_buscaMaiorSequencia, &mutex);
            printf("buscaMaiorSequencia DESBLOQUEADA\n");
        }
        qtd_elementosNoBloco = *(estruturaDeControle + (posicaoBuffer * 4)); //estruturaDeControle[posicaoBuffer][0]
        for(int j=0; j<qtd_elementosNoBloco; j++){
            if(elemento_maiorSequencia != *(buffer + posicaoBuffer*N + j)){
                elemento_maiorSequencia = *(buffer + posicaoBuffer*N + j);
                posicao = numerosProcessados;
                qtd_maiorSequencia = 1;
            }
            else{
                qtd_maiorSequencia++;
            }
        }

        numerosProcessados += qtd_elementosNoBloco;
        pthread_mutex_lock(&mutex);
        *(buffer + posicaoBuffer*N + 1) = 0; //indica que a buscaMaiorSequencia nesse bloco já concluiu a leitura
        pthread_mutex_unlock(&mutex);

        //se todas as threads já tiverem concluido a leitura o bloco é liberado
        if(*(estruturaDeControle + posicaoBuffer*4 + 1) == 0 && (*(estruturaDeControle + posicaoBuffer*4 + 2)) == 0 && (*(estruturaDeControle + posicaoBuffer*4 + 3)) == 0){
            pthread_cond_signal(&cond_Buffer);
        }
        printf("Thread buscaMaiorSequencia terminou a busca no bloco %d\n", posicaoBuffer + 1);
        if(numerosProcessados == *qtd_elementos){
            printf("buscaMaiorSequencia terminou a busca\n");
            break;
        }
        posicaoBuffer = (posicaoBuffer + 1) % M;
    }
    printf("Maior sequência de valores idênticos: %lld %d %d\n", posicao, qtd_maiorSequencia, elemento_maiorSequencia);
    pthread_exit(NULL);
}

//thread busca triplas
void *buscaTriplas(){
    int posicaoBuffer = 0, qtd_elementosNoBloco;
    long long int numerosProcessados = 0;
    while(1){
        while(*(estruturaDeControle + posicaoBuffer*4 + 2) == 0){
            printf("buscaTriplas BLOQUEADA\n - bloco vazio ou já processador pela thread buscaTriplas.\n");
            pthread_cond_wait(&cond_buscaTriplas, &mutex);
            printf("buscaTriplas DESBLOQUEADA\n");
        }
        //printf("INICIANDO buscaTriplas NO BLOCO %d\n", posicaoBuffer);
        qtd_elementosNoBloco = *(estruturaDeControle + (posicaoBuffer * 4));
        for(int j=0; j<qtd_elementosNoBloco; j++){
            if(elemento_buscaTriplas != *(buffer + posicaoBuffer*N + j)){
                qtd_contaTriplas = 1;
                elemento_buscaTriplas = *(buffer + posicaoBuffer*N + j);
            }
            else if(elemento_buscaTriplas == *(buffer + posicaoBuffer*N + j) && qtd_triplas < 3){
                qtd_contaTriplas++;
            }
            else{
                qtd_triplas++;
                qtd_contaTriplas = 1;
            }
        }        
        numerosProcessados += qtd_elementosNoBloco;

        pthread_mutex_lock(&mutex);
        *(estruturaDeControle + posicaoBuffer*4 + 2) = 0; //flag que indica que o bloco já foi lido
        pthread_mutex_unlock(&mutex);

        
        if(*(estruturaDeControle + posicaoBuffer*4 + 1) == 0 && (*(estruturaDeControle + posicaoBuffer*4 + 2)) == 0 && (*(estruturaDeControle + posicaoBuffer*4 + 3)) == 0){
            pthread_cond_signal(&cond_Buffer);
        }
        printf("Thread buscaTriplas terminou no bloco %d\n", posicaoBuffer + 1);
        if(numerosProcessados == *qtd_elementos){
            printf("buscaTriplas terminou a busca\n");
            break;
        }
        posicaoBuffer = (posicaoBuffer + 1) % M;
    }
    pthread_exit(NULL);
}

//thread busca 012345
void *buscaSequencia(){
    int posicaoBuffer = 0, qtd_elementosNoBloco;
    long long int numerosProcessados = 0;
    while(1){
        while(*(estruturaDeControle + posicaoBuffer*4 + 3) == 0){
            printf("buscaSequencia BLOQUEADA\n - bloco vazio ou já processador pela thread buscaSequencia.\n");
            pthread_cond_wait(&cond_buscaSequencia, &mutex);
        }
        //printf("INICIANDO buscaSquencia NO BLOCO %d\n", posicaoBuffer);
        qtd_elementosNoBloco = *(estruturaDeControle + (posicaoBuffer * 4));
        for(int j=0; j<qtd_elementosNoBloco; j++){
            if(sequencia != *(buffer + posicaoBuffer*N + j)){
                sequencia = 0;
            }
            else if(sequencia == *(buffer + posicaoBuffer*N + j) && sequencia == 5){
                qtd_sequencia++;
            }

        }

        numerosProcessados += qtd_elementosNoBloco;

        pthread_mutex_lock(&mutex);
        *(estruturaDeControle + posicaoBuffer*4 + 3) = 0;        
        pthread_mutex_unlock(&mutex);

     
        if(*(estruturaDeControle + posicaoBuffer*4 + 1) == 0 && (*(estruturaDeControle + posicaoBuffer*4 + 2)) == 0 && (*(estruturaDeControle + posicaoBuffer*4 + 3)) == 0){
            pthread_cond_signal(&cond_Buffer);
        }
        printf("Thread buscaSequencia terminou no bloco %d\n", posicaoBuffer + 1);
        if(numerosProcessados == *qtd_elementos){
            printf("buscaSequencia terminou a busca\n");
            break;
        }
        posicaoBuffer = (posicaoBuffer + 1) % M;
    }
    pthread_exit(NULL);
}

//thread que le o arquivo
void *carregaBuffer(){
    printf("INICIANDO O CARREGAMENTO DO BUFFER\n");
    long long int contadorElementos = 0;
    int elementosLidosNoBloco = 0;
    int in = 0;
    while(1){
        //enquanto ainda houver flag 1 no bloco, a leitura nesse bloco é bloqueada
        // *(estruturaDeControle + 4*in + 1) == -1 || 
        while(*(estruturaDeControle + 4*in + 2) == -1 || *(estruturaDeControle + 4*in + 3) == -1){
            printf("carregabuffer bloqueada - bloco %d\n", in + 1);
            printf("aguardando a conclusão de leitura(s) no bloco.");
            pthread_cond_wait(&cond_Buffer, &mutex);
            printf("carrega buffer DESbloqueada. - bloco %d\n", in);
        }

        //leitura do arquivo binário
        elementosLidosNoBloco = fread(buffer + (N * in), sizeof(int), N, ptr_file);
        contadorElementos += elementosLidosNoBloco;

        pthread_mutex_lock(&mutex);
        *(estruturaDeControle + 4*in) = elementosLidosNoBloco;
        *(estruturaDeControle + 4*in + 1) = -1;
        *(estruturaDeControle + 4*in + 2) = -1;
        *(estruturaDeControle + 4*in + 3) = -1;
        pthread_mutex_unlock(&mutex);

        
        // printf("Estado do buffer:\n");
        // for(int i=0; i<M; i++){
        //     for (int j=0; j<N; j++){
        //         printf("%d ", *(buffer + i*N + j));
        //     }
        //     printf("\n");
        // }
        // printf("\n");

        // printf("Estrutura de Controle:\n");
        // for(int i=0; i<M; i++){
        //     for (int j=0; j<4; j++){
        //         printf("%d ", *(estruturaDeControle + i*4 + j));
        //     }
        //     printf("\n");
        // }
        // printf("\n");
        // printf("\n");
        printf("Escrita no bloco %d concluída.\n", in + 1);


        pthread_cond_signal(&cond_buscaMaiorSequencia);
        pthread_cond_signal(&cond_buscaTriplas);
        pthread_cond_signal(&cond_buscaSequencia);

        if(contadorElementos == *qtd_elementos){ //quanto todo o arquivo for lido, termina de carregar o buffer
            printf("leitura finalizada\n");
            break;
        }
        in = (in + 1) % M;
    }

    pthread_cond_signal(&cond_buscaMaiorSequencia);
    pthread_cond_signal(&cond_buscaTriplas);
    pthread_cond_signal(&cond_buscaSequencia);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    char nomeArquivo[30];
    pthread_t tid[4];

    //inicia as variáveis de sincronização
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_Buffer, NULL);
    pthread_cond_init(&cond_buscaMaiorSequencia, NULL);
    pthread_cond_init(&cond_buscaTriplas, NULL);
    pthread_cond_init(&cond_buscaSequencia, NULL);

    if(argc < 4){
        printf("Quantidade de parâmetros inválida. Digite <nomeArquivoLeitura> <tamanhoBuffer> <tamanhoBlocos>\n");
        return 1;
    }

    strcpy(nomeArquivo, argv[1]);
    M = atoi(argv[2]);
    N = atoi(argv[3]);

    //abrindo o ponteiro para o arquivo
    if((ptr_file = fopen(nomeArquivo, "rb")) == NULL){
        printf("Erro--fopen\n");
        return 1;
    }

    if((qtd_elementos = (long long int*)malloc(sizeof(long long int))) == NULL){
        printf("Erro--malloc\n");
        return 1;
    }

    // //pegando o primeiro inteiro que representa a quantidade de números no arquivo
    if(fread(qtd_elementos, sizeof(long long int), 1, ptr_file) != 1){
        printf("Erro--fread\n");
        return 1;
    }

    printf("quantidade de inteiros no arquivo = %lld\n", *qtd_elementos); // --------ok
    
    //alocando memória para o buffer (matriz MxN)
    if((buffer = (int*)malloc(M * N * sizeof(int*))) == NULL){ 
        printf("Erro--malloc\n");
        return 1;
    }

    //alocando memória para a estrutura de controle
    if((estruturaDeControle = (int*)malloc(M * 4 * sizeof(int*))) == NULL){ 
        printf("Erro--malloc\n");
        return 1;
    }    

    //inicializando o buffer com zeros
    for(int i=0; i<M; i++){
        for (int j=0; j<N; j++){
            *(buffer + i*N + j) = 0;
        }
    }

    //estrutura de controle
    for(int i=0; i<M; i++){
        for (int j=0; j<4; j++){
            *(estruturaDeControle + i*4 + j) = 0;
        }
    }

    //imprime o buffer
    imprimeBuffer();
    printf("fim do buffer\n");
    printf("\n");
    imprimeEstruturaDeControle();
    printf("fim da estrutura de controle.\n");
    printf("\n");

    //inicia as threads
    if(pthread_create(&tid[0], NULL, carregaBuffer, NULL)){printf("Erro--pthread_create\n"); return 1;}
    if(pthread_create(&tid[1], NULL, buscaSequencia, NULL)){printf("Erro--pthread_create\n"); return 1;}
    if(pthread_create(&tid[2], NULL, buscaTriplas, NULL)){printf("Erro--pthread_create\n"); return 1;}
    if(pthread_create(&tid[3], NULL, buscaMaiorSequencia, NULL)){printf("Erro--pthread_create\n"); return 1;} 

    //faz o join
    for(int i=0; i<4; i++){
        if(pthread_join(tid[i], NULL)){
            printf("Erro--pthread_join\n");
            return 1;
        }
    }

    free(ptr_file);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_Buffer);
    pthread_cond_destroy(&cond_buscaMaiorSequencia);
    pthread_cond_destroy(&cond_buscaTriplas);
    pthread_cond_destroy(&cond_buscaSequencia);
    
    printf("Maior sequência de valores idênticos: %lld %d %d\n", posicao, qtd_maiorSequencia, elemento_maiorSequencia);
    printf("Quantidade de triplas: %d\n", qtd_triplas);
    printf("Quantidade de ocorrências da sequencia <012345>: %d\n", qtd_sequencia);

    return 0;
}
