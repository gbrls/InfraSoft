#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct elem{
   int value;
   struct elem *prox;
}Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
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

    return q;
}

// <- Saída         <- Entrada
// [NULL|]->[|]->[|]->NULL
// ^ Head/Sent   ^ Tail


//TODO: Implementar o que foi pedido
void putBlockingQueue(BlockingQueue* Q,int newValue) {
    Elem* e = newEl(newValue);
    Q->last->prox = e;
    Q->last = e;
}

//TODO: Implementar o que foi pedido
int takeBlockingQueue(BlockingQueue* Q) {
    if(Q->head->prox == NULL) {
        fprintf(stderr, "Fila está vazia!\n");
        exit(-1);
    }

    Elem* n = Q->head->prox->prox;
    int v = Q->head->prox->value;

    free(Q->head->prox);
    Q->head->prox = n;

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
    takeBlockingQueue(q); takeBlockingQueue(q); takeBlockingQueue(q); takeBlockingQueue(q); takeBlockingQueue(q); 
    destroyQ(q);
}

int main() {

    test_queue();

    return 0;
}