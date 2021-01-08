#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include"timer.h"

FILE *ptr_txt[17]; //17 arquivos com os capítulos do livro
char palavra[30]; //palavra que será buscada no texto
int qtd_threads; //quantidade de threads

//funcao que verifica quantas vezes uma string esta contida em outra
int contaString(char *str1){
    int ocorrencias = 0; //quantidade de vezes que a palavra aparece na string
    
    //verifica se a palavra aparece na string e retorna quantas vezes
	for(int i=0; i<strlen(str1); i++){
		if(str1[i] == palavra[0] || str1[i] + 32 == palavra[0] || str1[i] - 32 == palavra[0]){
			for(int j=1; j<strlen(palavra); j++){
				if(str1[i + j] != palavra[j] && str1[i + j] + 32 != palavra[j] && str1[i + j] - 32 != palavra[j])
					break;
				if(j == strlen(palavra)-1)
					ocorrencias++;
			}
		}
	}
    return ocorrencias;
}

//funcao executada pelas threads
void *buscaPalavra(void *args){
    long int id = (long int)args; //id ta thread
    int inicio = 17/qtd_threads * id; //onde a thread vai começar a processar o vetor
    int fim; //onde a thread vai terminar o processamento do vetor
    int *qtd_palavras = 0; //quantas vezes a palavra apareceu no capítulo
    char string[1000]; //variavel que vai armazenar a string do arquivo txt
    
    //aloca memoria para o ponteiro qtd_palavras
    if((qtd_palavras = (int*)malloc(sizeof(int))) == NULL){
    	printf("Erro--malloc");
    	exit(1);
    }

    //verifica se é a ultima thread, que vai processar um capítulo a mais
    if(id == qtd_threads - 1)
        fim = 17;
    else
        fim = inicio + 17/qtd_threads;

    //pega linha por linha do arquivo txt e chama a funcao contaString para verificar a quantidade de vezes que a palavra apareceu na string
    for(int j=inicio; j<fim; j++){
    	if((fgets(string, 1000, ptr_txt[j])) == NULL){
    		printf("Erro--fgets\n");
    		exit(1);
    	}
    
        while(fgets(string, 1000, ptr_txt[j])){
            *qtd_palavras += contaString(string);
        }
    }
    //retorna a quantidade achada
    //free(qtd_palavras);
    pthread_exit((void*)qtd_palavras);

}

int main(int argc, char *argv[]){
    char nomeArquivo[10], stringAux[10]; //nomes dos arquivos txt estring auxiliar para 
    int ocorrenciaPalavra = 0; // quantidade de vezes que a palavra apareceu no livro
    int *retornoThreads; // valor retornado pelas threads
    pthread_t *tid; // threads 
    double inicioTempo, fimTempo;

    //verifica se a quantidade de threads foi passada ao executar o programa
    if(argc < 2){
        printf("Digite %s <quantidade de threads>", argv[0]);
        return 1;
    }
    qtd_threads = atoi(argv[1]);

    printf("Qual palavra você deseja buscar no livro? ");
    scanf("%s", palavra);

    // inicializando os ponteiros para os arquivos de texto
    for(int i=0; i<17; i++){
        //forma o nome do arquivo capítulo
        strcpy(nomeArquivo, "cap");
        sprintf(stringAux, "%d", i+1);
        strcat(nomeArquivo, stringAux);
        strcat(nomeArquivo, ".txt");

        //inicializa o vetor de ponteiros para os arquivos com os capítulos do livro
        if((ptr_txt[i] = fopen(nomeArquivo, "r")) == NULL){
            printf("Erro--fopen\n");
            return 1;
        }
    }
	GET_TIME(inicioTempo);
    //alocar memoria para as threads
    if((tid = (pthread_t*)malloc(sizeof(pthread_t) * qtd_threads)) == NULL){
        printf("Erro--malloc\n");
        return 1;
    }

    //criar as threads
    for(long int i=0; i<qtd_threads; i++){
        if(pthread_create(tid+i, NULL, buscaPalavra, (void*)i)){
            printf("Erro--pthread_create");
            return 1;
        }
    }

    //usar o join
    for(int i=0; i<qtd_threads; i++){
        if(pthread_join(*(tid+i), (void**) &retornoThreads)){
            printf("Erro--pthread_join");
            return 1;
        }
        ocorrenciaPalavra += *retornoThreads; //acumula os valores retornados pelas threads
    }
    GET_TIME(fimTempo);

    printf("A palavra %s aparece %d vezes no livro Harry Potter and the Sorcerer's Stone.\n", palavra, ocorrenciaPalavra);
    printf("Tempo decorrido: %lf", fimTempo - inicioTempo);
    
    for(int i=0; i<17; i++){
        free(ptr_txt[i]);
    }
    return 0;
}
