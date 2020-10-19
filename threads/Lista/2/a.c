#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



pthread_mutex_t *mut;
char **matriz;
char colors[8][10] = {"\e[41m", "\e[43m", "\e[44m", "\e[45m", "\e[42m", "\e[40m", "\e[46m", "\e[47m"};
char reset[10] = "\e[0m";

typedef struct{
    int n;
    int num_linhas;
    char **s;
} mat;

void printa_matriz(int num_linhas){
    system("clear");
    for(int i=0; i<num_linhas; i++){
        printf("%s" "%s" "%s", colors[i], matriz[i], reset);
    }
}

void *func(void *m){
    mat *aux = (mat *) m;
    int n = aux->n;
    for(int i=0; i<n; i+=2){
        int l = atoi(aux->s[i]);
        l--;
        while(pthread_mutex_trylock(&mut[l]) != 0){;}
        strcpy(matriz[l], aux->s[i+1]);
        pthread_mutex_unlock(&mut[l]);
    }
    //printa_matriz(aux->num_linhas);

    pthread_exit(NULL);
}

void thread(int num_threads, int num_linhas){

    pthread_t *threads = NULL;
    threads = (pthread_t *) malloc(num_threads*sizeof(pthread_t));

    mat *aux = NULL;
    aux = (mat *) malloc(num_threads*sizeof(mat));

    for(int i=0; i<num_threads; i++){

        char buffer[7] = {};
        sprintf(buffer, "%d.txt", i+1);
        FILE *arq = NULL;
        arq = fopen(buffer, "rt");
        if(arq == NULL){
            exit(-1);
        }

        char str[40];
        int j=0;
        aux[i].s = NULL;
        while(fgets(str, 40, arq) != NULL){
            aux[i].s = (char **) realloc(aux[i].s, (j+1)*sizeof(char *));
            aux[i].s[j] = (char *) malloc(40*sizeof(char));
            strcpy(aux[i].s[j], str);
            j++;
        }
        aux[i].n = j;
        aux[i].num_linhas = num_linhas;

        int ret = pthread_create(&threads[i], NULL, func, (void *) &aux[i]);

        free(arq);
    }

    for(int i=0; i<num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    free(threads);

    for(int i=0; i<num_threads; i++){
        for(int j=0; j<aux[i].n; j++){
            free(aux[i].s[j]);
        }
        free(aux[i].s);
    }   

    free(aux);
}

void init(int *l, int *n){
    matriz = NULL;
    *l = 7;
    mut = (pthread_mutex_t *) malloc((*l)*sizeof(pthread_mutex_t));
    for(int i=0; i<*l; i++){
        pthread_mutex_init(mut+i, NULL);
    }

    printf("Entre com o numero de arquivos: ");
    scanf("%d", n);
    printf("\n");

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


int main() {

    int num_threads, num_linhas;

    init(&num_linhas, &num_threads);

    thread(num_threads, num_linhas);

    printa_matriz(num_linhas);

    exit(0);

    return 0;
}