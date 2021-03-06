#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// < Se quiser que os prints de debug que demonstram o funcionamento do programa sejam executados, descomente a próxima linha >
//#define DEBUG 

// typedef pra passarmos o ponteiro de função como argumento em uma função 
typedef void* (*fPtr)(void* arg);

// struct contendo uma referência para uma função, a id do processo e o argumento que a função irá receber
typedef struct {
    int id;
    void* arg;
    fPtr fn;
} packedFn;

// struct de elemento utilizado na implementação da fila bloqueante (blocking queue)
typedef struct elem{
   packedFn* value;
   struct elem *prox;
}Elem;

// struct que contém um array, seu tamanho (sz) e um mutex que controla se ele está sendo acessado por alguma thread ou não.
//  (utilizado para a implementação de um array dinâmico)
typedef struct {
    int* arr;
    int sz;
    pthread_mutex_t mut;
}Vector;

// cria um novo vetor do tipo Vector definido acima
void newVec(Vector* v) {
        // inicializa o mutex que controla se o vetor está ou não em uso  
    pthread_mutex_init(&v->mut, NULL);
        // aloca a memória necessária para o array
    v->arr = (int*) malloc(sizeof(int));
}

// deleta o vetor dinâmico criado 
void deleteVec(Vector* v) {
    pthread_mutex_destroy(&v->mut);
    free(v->arr);
}

// retorna um ponteiro para uma posição do vetor, caso essa posição não exista, o vetor aloca o espaço que falta para ela.
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

// pega o valor de uma posição no vetor, com mutexes para evitar condições de corrida.
int VecGet(Vector* v, int p) {
    pthread_mutex_lock(&v->mut);

    int x = *_pos(v, p);

    pthread_mutex_unlock(&v->mut);

    return x;
}

// seta o valor de uma posição no vetor, com mutexes para evitar condições de corrida. 
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

void putBlockingQueue(BlockingQueue* Q,packedFn* newValue) {
    Elem* e = newEl(newValue);

    pthread_mutex_lock(&Q->mut);

    if(Q->statusBuffer == Q->sizeBuffer) {
        //puts("A fila está cheia, dormindo...");
        //pthread_cond_wait(&Q->notFull, &Q->mut);
#ifdef DEBUG
        puts("A fila está cheia, colocando mesmo assim...");
#endif
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
#ifdef DEBUG
        puts("Fila vazia, dormindo...");
#endif
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

// função de debug para printar os elementos da fila
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

// wrapper de _destroy
void destroyQ(BlockingQueue* Q) {
    _destroy(Q->head);
    pthread_mutex_destroy(&Q->mut);
    pthread_cond_destroy(&Q->notEmpty);
    pthread_cond_destroy(&Q->notFull);
    free(Q);
}

// número máximo de threads em execução.
#define N 3

// struct que guarda uma thread em execução e o id da função que está rodando nela.
typedef struct {
    int id;
    pthread_t thread;
}taggedThread;

int total = 0;
// representa a fila de espera
BlockingQueue* queue;
// representa o estado dos núcleos
taggedThread nucleos[N] = {0};

// muted para o array acima
pthread_mutex_t nucl_mut;
pthread_cond_t nucl_cond;

// variavel condicional para avisar quando uma nova thread foi despachada em algum núcleo.
pthread_cond_t nova_thread_cond;

// buffer que guarda os resultados das funções
Vector vec;

// struct para passar como argumento para uma função
typedef struct {
    int i; // qual thread a função será executada
    packedFn* p;
} _exe_arg;

// função para executar a função passada
void* executarThread(void* arg) {
    _exe_arg* ea = (_exe_arg*)arg;
    int nucleo_que_esta_rodando = ea->i;
    packedFn* p = ea->p;

    int id_do_agendamento = p->id;
    //printf("agendamento: %d, thread: %d, arg: %d\n", id_do_agendamento, ea->i, (int)arg);

    void* ret = p->fn(p->arg);

    VecSet(&vec, nucleos[nucleo_que_esta_rodando].id,(int)ret);
#ifdef DEBUG
    printf("Escreveu em %d %d\n",nucleos[nucleo_que_esta_rodando].id, (int) ret);
#endif

    pthread_mutex_lock(&nucl_mut);
    nucleos[nucleo_que_esta_rodando].id = -1;
    //avisando que existem núcleos vazios, assim o "escalonador" (função dispachante) pode pegar um processo e executa-lo neste processador vazio.
    pthread_cond_signal(&nucl_cond);
    pthread_mutex_unlock(&nucl_mut);

    free(arg);

    return ret;
}

// funciona como o "escalonador" dos processos; para cada núcleo, se este núcleo estiver ocioso, executa o próximo processo da blockingQueue. 
void* despachanteFn() {
    while(1) {
        int cheio = 1;
        // para cada núcleo, se o núcleo estiver livre, executa o próximo processo presente na blockingQueue
        for(int i=0;i<N;i++) {
            pthread_mutex_lock(&nucl_mut);
            taggedThread x = nucleos[i];
            pthread_mutex_unlock(&nucl_mut);
            // se o núcleo está livre,  coloca o próximo processo da blockingQueue em execução
            if(x.id == -1) {

                // pela forma como takeBlockingQueue foi implementada, se a fila estiver vazia, esta thread vai dormir até
                // algum processo ser colocado na fila de espera.
                packedFn* p = takeBlockingQueue(queue);
                pthread_mutex_lock(&nucl_mut);
                nucleos[i].id = p->id;
                pthread_mutex_unlock(&nucl_mut);
#ifdef DEBUG
                printf("Taken %d from queue\n",p->id);
#endif
                _exe_arg* args = (_exe_arg*) malloc(sizeof(_exe_arg));//{.i=i, .p=p};
                args->i = i; // qual thread está sendo executada
                args->p = p; // a função junto com o seu argumento e o seu id

#ifdef DEBUG
                printf("Usando a thread %d\n",i);
#endif
                pthread_create(&nucleos[i].thread, NULL, executarThread, (void*)args);
                pthread_cond_signal(&nova_thread_cond);

                cheio = 0;
            }
        }

        // se tocos os núcleos estiverem ocupados, a thread despachante vai dormir até um deles ser liberado
        if(cheio) {
            // travou o mutex
#ifdef DEBUG
            puts("Núcleos cheios, dormindo");
#endif
            pthread_mutex_lock(&nucl_mut);
            // libera, volta e trava
            pthread_cond_wait(&nucl_cond, &nucl_mut);

#ifdef DEBUG
            puts("Núcles livres, acordando");
#endif
            // libera
            pthread_mutex_unlock(&nucl_mut);
        }

    }
}

// adiciona um processo à bloquingQueue de processos (contendo a função a ser executada e o argumento a ser passado para a função)
int agendarExecucao(fPtr funexec, void* arg) {
    // incrementa o id, de tal forma que cada novo processo possua um id igual ao último processo a ser agendado + 1
    int id = total++;

    // setando os campos da struct packedFn para a execução da função.
    // esta struct tem o propósito de guardar tudo o que é necessário para a execução de uma função.
    packedFn* fn = (packedFn*) malloc(sizeof(packedFn));
    fn->fn = funexec;
    fn->arg = arg;
    fn->id = id;

#ifdef DEBUG
    printf("agendando %p\n",fn);
#endif

    putBlockingQueue(queue, fn);
    
    return id;
}

/*
Quando vamos pegar o resultado, existem três possíveis casos:
O primeiro em que a função já terminou e podemos ler o seu output.
O segundo em que a função ainda está em execução, então vamos esperar o seu output.
O terceiro, em que a função ainda não foi chamada e que devemos esperar o output.
Nota-se que em nenhum desses casos a espera é implementada como espera ocupada.
*/
int pegarResultadoExecucao(int id) {

    // Testando se a função já foi executada e temos o seu output.
    // Nota-se que VecGet é implementado com mutexes internamente, assim evitando uma condição de disputa.
    if(VecGet(&vec, id) != -1) {
#ifdef DEBUG
        printf("thread já terminou! %d\n", id);
#endif
        return VecGet(&vec, id);
    }

    // Procurando se a função está rodando em algum núcleo.
    for(int i=0;i<N;i++) {
        pthread_mutex_lock(&nucl_mut);
        taggedThread x = nucleos[i];
        pthread_mutex_unlock(&nucl_mut);
        if(x.id == id) {
            void* ret;

#ifdef DEBUG
            printf("Esperando thread %d retornar\n",x.id);
#endif
            // se a função estiver rodando, a esperamos com o pthread_join
            pthread_join(x.thread, &ret);

#ifdef DEBUG
            printf("Retornou %d\n", (int) ret);
#endif
            return (int)ret;
        }
    }

#ifdef DEBUG
    printf("A func %d ainda não foi executada, esperando\n",id);
#endif

    // Caso a função ainda não tenha começado a rodar, vamos esperar com o cond
    // a thread será despertada quando uma nova função começar a sua execução.
    pthread_mutex_lock(&nucl_mut);
    pthread_cond_wait(&nova_thread_cond, &nucl_mut);
    pthread_mutex_unlock(&nucl_mut);

#ifdef DEBUG
    puts("Uma nova função começou wowow!");
#endif

    // Assim que uma função nova for executada, vamos testar os três casos de novo.
    return pegarResultadoExecucao(id);
}

// função utilizada como payload para testar o programa
void* testFun(void* arg) {
    printf("Hello there, general %d!\n", (int)arg);
    //sleep(rand()%10);
    //sleep(2);
    puts("fim da testfun");

    return (void*) 270;
}

// função utilizada como payload para testar o programa
void* testFun2(void* arg) {
    puts("Do a barrelroll");
    //*arg.ret = 7;
    sleep(2);
    puts("fim  da testfun2");

    return (void*) 1234;
}

int main() {

    //  inicializa o array de structs do tipo taggedThread (estrutura de dados com o id de uma trhead e uma referência para essa thread)
    for(int i=0;i<N;i++) {
        nucleos[i].id = -1;
    }

    // cria um vetor dinâmico
    newVec(&vec);
    
    // cria a fila bloqueante
    queue = newBlockingQueue(N);

    // inicia mutex e sinais (cond)
    pthread_mutex_init(&nucl_mut, NULL);
    pthread_cond_init(&nucl_cond, NULL);
    pthread_cond_init(&nova_thread_cond, NULL);

    //  cria a thread da função despachante
    pthread_t despachante;
    pthread_create(&despachante, NULL, despachanteFn, NULL);

    void* arg = (void*) 42;

    // program loop
    while(1) {
        int n;
        printf("Digite a op: [0/1], -1 para sair: ");
        scanf("%d",&n);

        int id;

        if(n == 0) {
            id = agendarExecucao(testFun, arg);
        } else if(n == 1) {
            id = agendarExecucao(testFun2, arg);
        } else if(n == -1) {
            exit(0);
        } else {
            continue;
        }

        printf("Resultado de %d = %d\n",id,pegarResultadoExecucao(id));
    }

    // Libera os recursos
    destroyQ(queue);
    pthread_mutex_destroy(&nucl_mut);
    pthread_cond_destroy(&nucl_cond);
    pthread_cond_destroy(&nova_thread_cond);
    deleteVec(&vec);

    return 0;
}

