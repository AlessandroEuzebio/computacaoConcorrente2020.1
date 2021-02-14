1-
  /* A)Como em_e e em_l são utilizados para fazer a exclusão mútua, ambos devem ser inicializado com o valor 1. 
      Levando em consideração que os semáforos escr e leit desempenham o papel de semáforo contador, leit deve ser iniciado com o valor de mesma quantidade de 
      leitores e escr com o valor 1, pois só um escritor é permitido por vez.
  */
  
  // B)Sim, como foi visto em aula, a versão abaixo é bem mais simples:
    
    int leitores = 0;     // quantidade de leitores lendo
    sem_t mutex, escrita; //ambos inicializados com 1

    //leitores                                             
    void leitor(){                                         //escritores
        sem_wait(&mutex);                                  void escritor(){
        leitores++;                                            sem_wait(&escrita);
        if (leitores == 1){                                    /* faz a escrita */
            sem_wait(&escrita);                                sem_post(&escrita);
        }                                                  }
        sem_post(&mutex);
        /* faz a leitura */
        sem_wait(&mutex);
        leitores--;
        if (leitores == 0){
            sem_post(&escrita);
        }
        sem_post(&mutex);
    }


  // C)
      #include <stdio.h>
      #include <stdlib.h>
      #include <pthread.h>
      #include <semaphore.h>

      #define LEITORES 2
      #define ESCRITORES 2
      #define NTHREADS 4

      sem_t em_e, em_l, escr, leit;//semaforos
      int e = 0, l = 0;//globais

      void *leitores(void *threadId){
          int *id = (int *) threadId;
          while (1){
              sem_wait(&leit);
              printf("Iniciando leitor %d\n", id)
              sem_wait(&em_l);
              l++;
              if (l == 1)
                  sem_wait(&escr);
              sem_post(&em_l);
              sem_post(&leit)
              printf("Thread %d lendo...\n", id);
              sem_wait(&em_l);
              if (l == 0)
                  sem_post(&escr);
              printf("Finalizando leitura %d\n", id);
              sem_post(&em_l);
          }

          pthread_exit(NULL);
      }

      void *escritores(void *threadId){
          int *id = (int *) threadId;
          while (1){
              sem_wait(&em_e);
              printf("Iniciando escritor %d\n", id);
              e++;
              if (e == 1)
                  sem_wait(&leit);
              sem_post(&em_e);
              sem_wait(&escr);
              printf("Thread %d escrevendo...\n", id);
              sem_post(&escr);
              sem_wait(&em_e);
              e--;
              if (e == 0)
                  sem_post(&leit);
              printf("Finalizando escrita %d\n", id);
              sem_post(&em_e);
          }
      }

      //funcao principal
      int main(int argc, char *argv[]){
          pthread_t tid[NTHREADS]; //vetor de threads
          int *id[3], i;

          for (i = 0; i < NTHREADS; i++){
              if ((id[i] = malloc(sizeof(int))) == NULL){
                  pthread_exit(NULL);
                  return 1;
              }
              *id[i] = i + 1;
          }

          //inicia os semaforos
          sem_init(&em_l, 0, 1);
          sem_init(&em_e, 0, 1);
          sem_init(&escr, 0, 1);
          sem_init(&leit, 0, LEITORES);


          //cria os leitores
          for(i=0; i<LEITORES; i++){
              if(pthread_create(&tid[i], NULL, leitores, (void *)id[i])){
                  printf("--ERRO: pthread_create()\n");
                  exit(-1);
              }
          }

          //cria os escritores
          for(i=ESCRITORES -1; i<LEITORES + ESCRITORES; i++){
              if(pthread_create(&tid[i], NULL, escritores, (void *)id[i])){
                  printf("--ERRO: pthread_create()\n");
                  exit(-1);
              }
          }

          //--espera todas as threads terminarem
          for(i=0; i<NTHREADS; i++){
              if(pthread_join(tid[i], NULL)){
                  printf("--ERRO: pthread_join() \n");
                  exit(-1);
              }
              free(id[i]);
          }
          pthread_exit(NULL);
      }

  //d)
  //e)
