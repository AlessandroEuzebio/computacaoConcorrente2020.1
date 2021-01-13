// Módulo 2 - Laboratório 4
#include<stdio.h>
#include<pthread.h>

int x = 0;
pthread_mutex_t x_mutex;
pthread_cond_t x_cond;

void *thread1(void *args){

    printf("tudo bem?\n");
   
    pthread_mutex_lock(&x_mutex);
    x++;
    if(x==2){
        pthread_cond_broadcast(&x_cond);
    }
    pthread_mutex_unlock(&x_mutex);

    pthread_exit(NULL);
}

void *thread2(void *args){

    printf("bom dia!\n");
   
    pthread_mutex_lock(&x_mutex);
    x++;
    if(x==2){
        pthread_cond_broadcast(&x_cond);
    }
    pthread_mutex_unlock(&x_mutex);

    pthread_exit(NULL);
}
void *thread3(void *args){

    pthread_mutex_lock(&x_mutex);
    if(x < 2){
        pthread_cond_wait(&x_cond, &x_mutex);
    }
    printf("até mais!\n");
    pthread_mutex_unlock(&x_mutex);

    pthread_exit(NULL);
}
void *thread4(void *args){

    pthread_mutex_lock(&x_mutex);
    if(x < 2){
        pthread_cond_wait(&x_cond, &x_mutex);
    }
    printf("boa tarde!\n");
    pthread_mutex_unlock(&x_mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]){

    pthread_t tid[4];

    pthread_mutex_init(&x_mutex, NULL);
    pthread_cond_init (&x_cond, NULL);

    pthread_create(&tid[0], NULL, thread1, NULL);
    pthread_create(&tid[1], NULL, thread2, NULL);
    pthread_create(&tid[2], NULL, thread3, NULL);
    pthread_create(&tid[3], NULL, thread4, NULL);


    for(int i=0; i<4; i++){
        if(pthread_join(tid[i], NULL)){
            printf("Erro--pthread_join");
            return 1;
        }
    }

    pthread_mutex_destroy(&x_mutex);
    pthread_cond_destroy(&x_cond);
    return 0;
}
