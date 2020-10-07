#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Apesar de funcional, este exemplo não é um bom uso de threads, já que 
// apenas iremos incrementar um número, seria melhor fazer-lo sequencialmente.
// A velocidade deste programa é inversamente proporcional ao número de threads, 
// pois essencialmente estamos fazendo uma operação sequencial, mas com um mutex e várias threads.
// Ele acaba sendo mais lento que um programa com uma única thread pois após o contador chegar no máximo
// todas as outras threads precisam destravar o mutex para se terminarem.
// TODO: Tentar corrigir este problema de ter que destravar o mutex para sair da thread.

int contador = 0;
const int MAX = 1000000;
pthread_mutex_t mut;

void* func() {
    bool f = true;
    while(f) {
        pthread_mutex_lock(&mut);

        if(contador == MAX) {
            f = false;
        } else if((++contador) == MAX) {
            printf("Thread %ld fez o contador chegar em %d\n");
            f = false;
        }

        pthread_mutex_unlock(&mut);
    }

    pthread_exit(NULL);
}

int main() {

    int n;
    puts("Digite o Número de threads:");
    scanf("%d", &n);

    pthread_mutex_init(&mut, NULL);
    pthread_t threads[n];
    for(int i=0;i<n;i++) {
        int error = pthread_create(&threads[i], NULL, func, NULL);
        if(error) {
            fprintf(stderr, "Failed to create thread (%d)\n",i);
            exit(-1);
        }
    }

    printf("Main: %x\n", pthread_self());

    // Esperando todas as theads terminarem.
    for(int i=0;i<n;i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mut);

    // Ao invés de chamar pthread_exit da main, fazemos o retorno normal
    // que é equivalente à call exit(0). Diferente de pthread_exit, exit(0)
    // termina o processo, terminando todas as threads dele. Como teremos apenas 
    // uma thread no final do programa, não faz sentido chamar pthread_exit.
    return 0;
}