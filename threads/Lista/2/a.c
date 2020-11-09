#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t *mut;
char **matriz;
char colors[8][10] = {"\e[41m", "\e[43m", "\e[44m", "\e[45m", "\e[42m", "\e[40m", "\e[46m", "\e[47m"};
char reset[10] = "\e[0m";

typedef struct{
    int n;
    char **s;
} mat;

typedef struct{
    int i;
    int num_linhas;
} aux_;

void printa_matriz(int num_linhas){
    system("clear");
    for(int i=0; i<num_linhas; i++){
        printf("%s" "%s" "%s", colors[i], matriz[i], reset);
    }
}

void *func(void *k){
    aux_ *a = (aux_ *) k; 
    // salvamos o valor de i e de num_linhas em variaveis locais para facilitar o uso
    int t = a->i;
    int num_linhas = a->num_linhas;

    char buffer[7] = {};
    sprintf(buffer, "%d.txt", t+1); // salvamos em buffer o numero que deverá ser lido(1.txt, 2.txt, 3.txt, ..., n.txt)
    //abrindo o arquivo
    FILE *arq = NULL;
    arq = fopen(buffer, "rt");
    if(arq == NULL){
        exit(-1);
    }

    // aqui nos vamos usar um struct do tipo mat, que possui uma matriz de char e um n (num de linhas) 
    // o motivo para isso é que nós não temos nenhuma informação sobre o arquivo, então depois de fzr sua leitura
    // precisamos ter guardado em algum lugar a quantidade de linhas dele para que depois possamos atualizar nossa tabela
    mat aux;
    char str[40]; // str é um buffer onde salvaremos o conteudo do arquivo para depois passar para auxiliar
    int j=0;
    aux.s = NULL; // fazemos o ponteiro de de aux apontar para NULL para que possamos usar a funcao realloc
    while(fgets(str, 40, arq) != NULL){
        aux.s = (char **) realloc(aux.s, (j+1)*sizeof(char *)); // vamos alocando linhas novas na matriz a medida que temos novas linhas no arquivo
        // escolhi um tamanho arbitrário de 40 suficiente para salvar toda informação do arquivo
        aux.s[j] = (char *) malloc(40*sizeof(char));
        strcpy(aux.s[j], str);
        j++; // j sera um contador que ao final da leitura indicara a quantidade de linhas no arquivo
    }
    aux.n = j;

    free(arq);

    int n = aux.n;
    for(int i=0; i<n; i+=2){ // como a cada iteração faremos a leitura da linha e da informaçao daquela linha incrementamos nosso contador por 2 unidades
        int l = atoi(aux.s[i]); // char -> int, achamos a linha que deve ser alterada
        l--;
        pthread_mutex_lock(&mut[l]); // travamos o mutex daquela determinada linha para poder alterar seu conteudo
        //while(pthread_mutex_trylock(&mut[l]) != 0){;}
        strcpy(matriz[l], aux.s[i+1]); // alteramos o conteudo da linha
        pthread_mutex_unlock(&mut[l]); // destravamos o mutex
        //printa_matriz(num_linhas);
    }
    //printa_matriz(num_linhas);
    
    for(int i=0; i<n; i++){
        free(aux.s[i]);
    }
    free(aux.s);

    pthread_exit(NULL);
}

void thread(int num_threads, int num_linhas){

    // criando o vetor de threads com tamanho igual ao numero de arquivos da entrada
    pthread_t *threads = NULL;
    threads = (pthread_t *) malloc(num_threads*sizeof(pthread_t));

    //vetor index tem o proposito de salvar um valor correspondente ao arquivo que deverá ser processado por aquele determinada thread e tambem possibilitar
    //que a thread saiba a quantidade de linhas
    //aux_ é uma struct que contem um valor i e um num_linhas
    //Não podemos usar o próprio valor de i, pq o mesmo pode ter seu valor alterado pela thread main durante a execução das outras threads
    aux_ *index = NULL;
    index = (aux_ *) malloc(num_threads*sizeof(aux_));

    for(int i=0; i<num_threads; i++){
        index[i].i = i;
        index[i].num_linhas = num_linhas;
        int ret = pthread_create(&threads[i], NULL, func, (void *) &index[i]); // criamos a thread passando o index e a funcao func que ira processar um dos arquivos
    }
    
    for(int i=0; i<num_threads; i++){
        //join nas threads
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(index);
    
}

void init(int *l, int *n){

    //Faz o ponteiro da matriz apontar pra NULL
    matriz = NULL;

    //seta a quantidade de linhas, pode ser recebida como entrada do usuario, deixei assim para facilitar os testes
    *l = 7;

    // criamos o vetor de mutexes de acordo com a quantidade de linhas para realizar exclusao mutua 
    mut = (pthread_mutex_t *) malloc((*l)*sizeof(pthread_mutex_t));
    for(int i=0; i<*l; i++){
        pthread_mutex_init(mut+i, NULL);
    }

    printf("Entre com o numero de arquivos: ");
    scanf("%d", n);
    printf("\n");

    //alocamos nossa matriz (considerei aqui que cada linha teria um tamanho 25 baseado no exemplo dado)
    matriz = (char **) malloc((*l+1)*sizeof(char *));
    for(int i=0; i<*l; i++){
        matriz[i] = (char *) malloc(26*sizeof(char));
        for(int j=0; j<24; j++){
            matriz[i][j] = ' ';
        }
        matriz[i][24] = '\n';
        matriz[i][25] = '\0';
    }
}

//funcao para dar free
void free_(int num_linhas){
    for(int i=0; i<=num_linhas; i++){
        free(matriz[i]);
    }
    free(matriz);
}


int main() {

    int num_threads, num_linhas;

    //funcao de inicializacao 
    init(&num_linhas, &num_threads);

    //logica principal do programa, onde criamos e processamos as threads
    thread(num_threads, num_linhas);

    //printa a tabela de trens
    printa_matriz(num_linhas);

    free_(num_linhas);

    exit(0);

    return 0;
}