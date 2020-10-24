#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG

typedef struct elem{
   int value;
   struct elem *prox;
}Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer; // tamanho máximo
   unsigned int statusBuffer; // tamanho atual
   Elem *head,*last;

   pthread_mutex_t mut; // mutex para a fila

   pthread_cond_t notEmpty; // sinal para avisar quando a fila deixou de ficar vazia
   pthread_cond_t notFull; // sinal para avisar quando a fila deixou de ficar cheia
}BlockingQueue;

Elem* newEl(int val) {
    Elem* e = malloc(sizeof(Elem));

    e->value = val;
    e->prox = NULL;


    return e;
}

BlockingQueue* newBlockingQueue(unsigned int SizeBuffer) {
    BlockingQueue* q = (BlockingQueue*) malloc(sizeof(BlockingQueue));
    Elem* sentinela = newEl(-1);

    q->head = sentinela, q->last = sentinela;
    q->sizeBuffer = SizeBuffer;
    q->statusBuffer = 0;

    pthread_mutex_init(&q->mut, NULL);
    pthread_cond_init(&q->notEmpty, NULL);
    pthread_cond_init(&q->notFull, NULL);

    return q;
}

// <- Saída         <- Entrada
// [NULL|]->[|]->[|]->NULL
// ^ Head/Sent   ^ Tail

//TODO: Implementar o que foi pedido
void putBlockingQueue(BlockingQueue* Q,int newValue) {
    Elem* e = newEl(newValue);

    pthread_mutex_lock(&Q->mut);

    if(Q->statusBuffer == Q->sizeBuffer) {
        puts("A fila está cheia, dormindo...");
        pthread_cond_wait(&Q->notFull, &Q->mut);
    }

    bool was_empty = (Q->head->prox == NULL);

    Q->last->prox = e;
    Q->last = e;

    Q->statusBuffer++;

    if(was_empty) {
#ifdef DEBUG
        puts("Fila estava vazia, mandando sinal");
#endif
        pthread_cond_signal(&Q->notEmpty);
    }
    pthread_mutex_unlock(&Q->mut);
}

//TODO: Implementar o que foi pedido
int takeBlockingQueue(BlockingQueue* Q) {
    pthread_mutex_lock(&Q->mut);
    if(Q->head->prox == NULL) {
        puts("Fila vazia, dormindo...");
        pthread_cond_wait(&Q->notEmpty, &Q->mut);
#ifdef DEBUG
        puts("Fila não está mais vazia!");
#endif
    }

    bool was_full = (Q->sizeBuffer == Q->statusBuffer);

    Elem* n = Q->head->prox->prox;
    int v = Q->head->prox->value;

    free(Q->head->prox);
    Q->head->prox = n;
    Q->statusBuffer--;

    if(was_full) {
#ifdef DEBUG
        puts("A fila estava cheia, mandando sinal");
#endif
        pthread_cond_signal(&Q->notFull);
    }

    pthread_mutex_unlock(&Q->mut);

    return v;
}

void printQ(BlockingQueue* Q) {
    Elem* e = Q->head;
    while(e->prox != NULL) {
        printf("%d ", e->prox->value);
        e = e->prox;
    }
    putchar('\n');
}

void _destroy(Elem* e) {
    if(e == NULL) return;

    _destroy(e->prox);
    free(e);
}

// wrapper around _destroy
void destroyQ(BlockingQueue* Q) {
    _destroy(Q->head);
    pthread_mutex_destroy(&Q->mut);
    pthread_cond_destroy(&Q->notEmpty);
    pthread_cond_destroy(&Q->notFull);
    free(Q);
}

void test_queue() {
    BlockingQueue* q = newBlockingQueue(10);

    putBlockingQueue(q, 10); putBlockingQueue(q, 1); putBlockingQueue(q, 2); putBlockingQueue(q, 3);
    printQ(q);
    takeBlockingQueue(q); takeBlockingQueue(q);
    putBlockingQueue(q, 42); putBlockingQueue(q, 12);
    printQ(q);
    takeBlockingQueue(q);
    putBlockingQueue(q, 100);
    printQ(q);
    destroyQ(q);
}

void* tirar(void* _q) {
    BlockingQueue* Q = _q;
    puts("Tentando tirar da fila");
    int val = takeBlockingQueue(Q);
    printf("%d retirado da fila\n", val);
}

void* colocar(void* _q) {
    BlockingQueue* Q = _q;
    puts("Tentando colocar na fila");
    putBlockingQueue(Q, 42);
    puts("Colocado na fila");
    printQ(Q);
}

void test_notEmpty() {
    puts("[Testando o notEmpty...]");
    pthread_t test;
    BlockingQueue* q = newBlockingQueue(10);
    pthread_create(&test, NULL, tirar, q);

    sleep(2);
    puts("Mandando sinal");

    putBlockingQueue(q, 42);

    pthread_join(test, NULL);

    destroyQ(q);
}

void test_notFull() {

    puts("[Testando o notFull...]");
    pthread_t test;
    BlockingQueue* q = newBlockingQueue(2);

    putBlockingQueue(q, 1);
    putBlockingQueue(q, 2);

    pthread_create(&test, NULL, colocar, q);

    sleep(2);
    puts("Mandando sinal");

    int val = takeBlockingQueue(q);
    printf("tirado da fila %d\n", val);

    pthread_join(test, NULL);

    destroyQ(q);
}

int main() {

#ifdef DEBUG
    test_queue();
    test_notEmpty();
    test_notFull();
#endif

    return 0;
}