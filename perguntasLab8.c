1-
  //a)Como em_e e em_l são utilizados para fazer a exclusão mútua, ambos devem ser inicializado com o valor 1. 
  
  //b)Sim, como foi visto em aula, a versão abaixo é bem mais simples:
    
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


  //c)
  //d)
  //e)
