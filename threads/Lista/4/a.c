#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef void* (*fPtr)(void* arg);

typedef struct {
    void* arg;
    fPtr fn;
    int id;
} packedFn;


typedef struct elem{
   packedFn* value;
   struct elem *prox;
}Elem;

typedef struct {
    int* arr;
    int sz;
    pthread_mutex_t mut;
}Vector;

void newVec(Vector* v) {
    pthread_mutex_init(&v->mut, NULL);
    v->arr = (int*) malloc(sizeof(int));
}

void deleteVec(Vector* v) {
    pthread_mutex_destroy(&v->mut);
    free(v->arr);
}

int* _pos(Vector* v, int p) {
    if(p >= v->sz) {
        v->arr = realloc(v->arr,sizeof(int)*(p+1));

        for(int i=v->sz;i<=p;i++) {
            v->arr[i] = -1;
        }

        v->sz = p+1;
    }

    return &v->arr[p];
}
int VecGet(Vector* v, int p) {
    pthread_mutex_lock(&v->mut);

    int x = *_pos(v, p);

    pthread_mutex_unlock(&v->mut);

    return x;
}

void VecSet(Vector* v, int p, int val) {
    pthread_mutex_lock(&v->mut);
    (*_pos(v, p)) = val;
    pthread_mutex_unlock(&v->mut);
}

 
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
    int id;
    pthread_t thread;
}taggedThread;

int total = 0;
BlockingQueue* queue;
taggedThread nucleos[N] = {0};

pthread_mutex_t nucl_mut;
pthread_cond_t nucl_cond;

pthread_cond_t nova_thread_cond;

Vector vec;

typedef struct {
    int i; // qual thread a função será executada
    packedFn* p;
} _exe_arg;

void* executarThread(void* arg) {
    _exe_arg* ea = (_exe_arg*)arg;
    int nucleo_que_esta_rodando = ea->i;
    packedFn* p = ea->p;

    int id_do_agendamento = p->id;
    //printf("agendamento: %d, thread: %d, arg: %d\n", id_do_agendamento, ea->i, (int)arg);

    void* ret = p->fn(p->arg);

    VecSet(&vec, nucleos[nucleo_que_esta_rodando].id,(int)ret);
    printf("Escreveu em %d %d\n",nucleos[nucleo_que_esta_rodando].id, (int) ret);

    pthread_mutex_lock(&nucl_mut);
    nucleos[nucleo_que_esta_rodando].id = -1;
    //avisando que existem núcleos vazios
    pthread_cond_signal(&nucl_cond);
    pthread_mutex_unlock(&nucl_mut);

    return ret;
}

void* despachanteFn() {
    while(1) {
        int cheio = 1;
        for(int i=0;i<N;i++) {
            pthread_mutex_lock(&nucl_mut);
            taggedThread x = nucleos[i];
            pthread_mutex_unlock(&nucl_mut);
            if(x.id == -1) {
                //chamar

                packedFn* p = takeBlockingQueue(queue);
                pthread_mutex_lock(&nucl_mut);
                nucleos[i].id = p->id;
                pthread_mutex_unlock(&nucl_mut);
                printf("Taken %d from queue\n",p->id);


                _exe_arg* args = (_exe_arg*) malloc(sizeof(_exe_arg));//{.i=i, .p=p};
                args->i = i; // qual thread está sendo executada
                args->p = p;

                printf("Usando a thread %d\n",i);

                pthread_create(&nucleos[i].thread, NULL, executarThread, (void*)args);
                pthread_cond_signal(&nova_thread_cond);

                //free(p);
                cheio = 0;
            }
        }

        if(cheio) {
            // travou o mutex
            puts("Núcleos cheios, dormindo");
            pthread_mutex_lock(&nucl_mut);
            // libera, volta e trava
            pthread_cond_wait(&nucl_cond, &nucl_mut);
            puts("Núcles livres, acordando");
            // libera
            pthread_mutex_unlock(&nucl_mut);
        }

    }
}

int agendarExecucao(fPtr funexec, void* arg) {

    int id = total++;

    packedFn* fn = (packedFn*) malloc(sizeof(packedFn));
    fn->fn = funexec;
    fn->arg = arg;
    fn->id = id;

    printf("agendando %p\n",fn);
    putBlockingQueue(queue, fn);

    //printQ(queue);
    
    return id;
}

int pegarResultadoExecucao(int id) {

    if(VecGet(&vec, id) != -1) {
        printf("thread já terminou! %d\n", id);
        return VecGet(&vec, id);
    }

    for(int i=0;i<N;i++) {
        pthread_mutex_lock(&nucl_mut);
        taggedThread x = nucleos[i];
        pthread_mutex_unlock(&nucl_mut);
        if(x.id == id) {
            void* ret;
            printf("Esperando thread %d retornar\n",x.id);
            pthread_join(x.thread, &ret);
            printf("Retornou %d\n", (int) ret);
            return (int)ret;
        }
    }

    printf("A func %d ainda não foi executada, esperando\n",id);

    pthread_mutex_lock(&nucl_mut);
    pthread_cond_wait(&nova_thread_cond, &nucl_mut);
    pthread_mutex_unlock(&nucl_mut);

    puts("Uma nova função começou wowow!");

    return pegarResultadoExecucao(id);
}

void* testFun(void* arg) {
    printf("Hello there %d!\n", (int)arg);
    //sleep(rand()%10);
    //sleep(2);
    puts("END");

    return (void*) 270;
}

void* testFun2(void* arg) {
    printf("General kenobi\n");
    //*arg.ret = 7;
    sleep(2);
    puts("END");

    return (void*) 1234;
}

int main() {

    for(int i=0;i<N;i++) {
        nucleos[i].id = -1;
    }

    newVec(&vec);
    queue = newBlockingQueue(N);
    pthread_mutex_init(&nucl_mut, NULL);
    pthread_cond_init(&nucl_cond, NULL);
    pthread_cond_init(&nova_thread_cond, NULL);

    pthread_t despachante;
    pthread_create(&despachante, NULL, despachanteFn, NULL);

    void* arg = (void*) 42;
    //packedFn fn = {.fn=testFun,.arg=arg};
    //packedFn fn2 = {.fn=testFun2,.arg=arg};

    //agendarExecucao(testFun, arg);
    //agendarExecucao(testFun2, arg);
    //agendarExecucao(testFun, arg);

    // id -> nucl(id)

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

        //sleep(1);
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

    destroyQ(queue);
    pthread_mutex_destroy(&nucl_mut);
    pthread_cond_destroy(&nucl_cond);
    pthread_cond_destroy(&nova_thread_cond);
    deleteVec(&vec);

    return 0;
}
