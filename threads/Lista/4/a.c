#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// ret é o local para onde a função vai escrever a sua saída
typedef struct {
    void* args;
    int* ret;
} fnArg;

typedef void (*fPtr)(fnArg arg);

typedef struct {
    fnArg arg;
    fPtr fn;
    int id;
} packedFn;


typedef struct elem{
   packedFn* value;
   struct elem *prox;
}Elem;
 
/* O buffer de espera vai ser implementado com a
 * blockingQueue da questão 6
 */
typedef struct blockingQueue{
   unsigned int sizeBuffer; // tamanho máximo
   unsigned int statusBuffer; // tamanho atual
   Elem *head,*last;

   pthread_mutex_t mut; // mutex para a fila

   pthread_cond_t notEmpty; // sinal para avisar quando a fila deixou de ficar vazia
   pthread_cond_t notFull; // sinal para avisar quando a fila deixou de ficar cheia
}BlockingQueue;

Elem* newEl(packedFn* val) {
    Elem* e = malloc(sizeof(Elem));

    e->value = val;
    e->prox = NULL;


    return e;
}

BlockingQueue* newBlockingQueue(unsigned int SizeBuffer) {
    BlockingQueue* q = (BlockingQueue*) malloc(sizeof(BlockingQueue));
    Elem* sentinela = newEl(NULL);

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
void putBlockingQueue(BlockingQueue* Q,packedFn* newValue) {
    Elem* e = newEl(newValue);

    pthread_mutex_lock(&Q->mut);

    if(Q->statusBuffer == Q->sizeBuffer) {
        //puts("A fila está cheia, dormindo...");
        //pthread_cond_wait(&Q->notFull, &Q->mut);
        puts("A fila está cheia, colocando mesmo assim...");
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

packedFn* takeBlockingQueue(BlockingQueue* Q) {
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
    packedFn* v = Q->head->prox->value;

    if(Q->head->prox == Q->last) {
        //printf("Prox é nulo\n");
        Q->last = Q->head;
    }
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

    pthread_mutex_lock(&Q->mut);

    Elem* e = Q->head;
    while(e->prox != NULL) {
        printf("%p ", e->prox->value);
        e = e->prox;
    }
    putchar('\n');

    pthread_mutex_unlock(&Q->mut);
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

// número máximo de threads em execução.
#define N 3

typedef struct {
    int total;
    int atual;
    int arr[N];
    pthread_mutex_t mut[N];
   pthread_cond_t conds[N]; 
} arrMut;

arrMut* arr;
BlockingQueue* queue;
int nucleos[N] = {0};
pthread_mutex_t nucl_mut;
pthread_cond_t nucl_cond;

arrMut* newArrMut() {
    arrMut* a = (arrMut*) malloc(sizeof(arrMut));

    a->atual = 0;
    a->total = 0;

    for(int i=0;i<N;i++) {
        a->arr[i] = 5;
    }

    for(int i=0;i<N;i++) {
        pthread_mutex_init(a->mut + i, NULL);
        pthread_cond_init(a->conds + i, NULL);
    }

    return a;
}

void arrMutDestroy(arrMut* a) {
    for(int i=0;i<N;i++) {
        pthread_mutex_destroy(a->mut + i);
        pthread_cond_destroy(a->conds + i);
    }

    free(a);
}

typedef struct {
    int i; // qual thread a função será executada
    packedFn* p;
} _exe_arg;

void* executarThread(void* arg) {
    _exe_arg* ea = (_exe_arg*)arg;
    int i = ea->i;
    packedFn* p = ea->p;

    int arr_pos = p->id;
    //printf("arr_pos: %d, thread: %d, ret: %p\n",arr_pos, ea->i, ea->p->arg.ret);
    pthread_mutex_lock(&arr->mut[arr_pos]);

    p->fn(p->arg);

    pthread_mutex_unlock(&arr->mut[arr_pos]);

    pthread_mutex_lock(&nucl_mut);
    nucleos[i] = 0;
    //avisando que existem núcleos vazios
    pthread_cond_signal(&nucl_cond);
    pthread_mutex_unlock(&nucl_mut);
}

void* despachanteFn() {
    while(1) {
        int cheio = 1;
        pthread_t t;
        for(int i=0;i<N;i++) {
            pthread_mutex_lock(&nucl_mut);
            int x = nucleos[i];
            pthread_mutex_unlock(&nucl_mut);
            if(x == 0) {
                //chamar

                pthread_mutex_lock(&nucl_mut);
                nucleos[i] = 1;
                pthread_mutex_unlock(&nucl_mut);
                packedFn* p = takeBlockingQueue(queue);
                printf("Taken %d from queue\n",p->id);

                _exe_arg* args = (_exe_arg*) malloc(sizeof(_exe_arg));//{.i=i, .p=p};
                args->i = i; // qual thread está sendo executada
                args->p = p;

                printf("Usando a thread %d\n",i);

                pthread_create(&t, NULL, executarThread, (void*)args);
                //free(p);
                cheio = 0;
            }
        }

        if(cheio) {
            // travou o mutex
            pthread_mutex_lock(&nucl_mut);
            // libera, volta e trava
            pthread_cond_wait(&nucl_cond, &nucl_mut);
            // libera
            pthread_mutex_unlock(&nucl_mut);
        }

    }
}

int agendarExecucao(fPtr funexec, fnArg arg) {
    //sleep(1);
    //funexec(arg);

    int id = arr->total;
    arg.ret = &arr->arr[id%N];

    arr->total++;
    arr->atual = (arr->total)%N;

    packedFn* fn = (packedFn*) malloc(sizeof(packedFn));
    fn->fn = funexec;
    fn->arg = arg;
    fn->id = id;

    printf("agendando %p\n",fn);
    //packedFn fn = {.fn=funexec, .arg=arg};
    putBlockingQueue(queue, fn);
    printQ(queue);

    
    return id;
}

int pegarResultadoExecucao(int id) {
    pthread_mutex_lock(&arr->mut[id%N]);
    int ret = (arr->arr[id%N]);
    pthread_mutex_unlock(&arr->mut[id%N]);

    return ret;
}

void testFun(fnArg arg) {
    printf("Hello there %d!\n", *arg.ret);
    *arg.ret += 1;
    sleep(rand()%10);
    puts("END");
}

void testFun2(fnArg arg) {
    printf("General kenobi\n");
    //*arg.ret = 7;
    sleep(10);
    puts("END");
}

int main() {

    arr = newArrMut();
    queue = newBlockingQueue(N);
    pthread_mutex_init(&nucl_mut, NULL);
    pthread_cond_init(&nucl_cond, NULL);

    pthread_t despachante;
    pthread_create(&despachante, NULL, despachanteFn, NULL);

    fnArg arg = {.args=NULL, .ret=NULL};
    packedFn fn = {.fn=testFun,.arg=arg};
    packedFn fn2 = {.fn=testFun2,.arg=arg};

    agendarExecucao(testFun, arg);
    agendarExecucao(testFun2, arg);
    agendarExecucao(testFun, arg);

    while(1) {
        int n;
        printf("Digite a op: ");
        scanf("%d",&n);

        int id;

        if(n == 0) {
            puts("OP 0");
            id = agendarExecucao(testFun, arg);
        } else if(n == 1) {
            puts("OP 1");
            id = agendarExecucao(testFun2, arg);
        }

        printf("Resultado de %d = %d\n",id,pegarResultadoExecucao(id));

    }

    //pegarResultadoExecucao(0);

    //putBlockingQueue(queue, &fn);
    //putBlockingQueue(queue, &fn);
    //putBlockingQueue(queue, &fn2);

    //while(1) {
    //    sleep(1);
    //    packedFn* p = takeBlockingQueue(queue);
    //    p->fn(p->arg);
    //}

    pthread_join(despachante, NULL);

    arrMutDestroy(arr);
    destroyQ(queue);
    pthread_mutex_destroy(&nucl_mut);
    pthread_cond_destroy(&nucl_cond);

    return 0;
}
