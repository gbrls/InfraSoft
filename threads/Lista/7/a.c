#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Struct pra representar cada pixel 
typedef struct{
    int R, G, B;
}pixel;


// faz a leitura do arquivo .ppm e salva na matriz de pixels
void read(pixel ***pixels, int *l1, int *c1, int *max1, char *cod){
    
    FILE *arq = NULL;
    //Abrindo o arquivo para leitura
    arq = fopen("in.ppm", "rt");
    if(arq == NULL){
        exit(0);
    }

    char str[20];
    int l, c, max, i = 0;

    //O while faz a leitura do codigo, das dimensoes da matriz e do valor limite de cada elemento do pixel
    while(fgets(str, 15, arq) != NULL){ // a cada iteracao a linha do arquivo vai estar salva em str
        if(i == 0){ // primeira iteracao recebe o codigo
            strcpy(cod, str);
        }
        else if(i == 1){ // na segunda recebe as dimensoes (considerei <10) e aloca a matriz 
            c = str[0]-'0'; // char -> int (colunas)
            l = str[2]-'0'; // char -> int (linhas)
            *pixels = (pixel **) malloc(l*sizeof(pixel *));
            if(*pixels == NULL){
                exit(-1);
            }
            for(int j=0; j<l; j++){
                (*pixels)[j] = (pixel *) malloc(c*sizeof(pixel));
                if((*pixels)[j] == NULL){
                    exit(-1);
                }
            }
        }
        else if(i == 2){ // faz a leitura do valor limite
            max = atoi(str);
            break;
        }
        i++;
    }
    //Agora lemos os pixels em si e salvamos na matriz
    for(int i=0; i<l; i++){ 
        for(int j=0; j<c; j++){
            int R, G, B;
            fscanf(arq, "%d %d %d", &R, &G, &B);
            (*pixels)[i][j].R = R;
            (*pixels)[i][j].G = G;
            (*pixels)[i][j].B = B;
        }
    }

    *max1 = max; *l1 = l; *c1 = c;

    fclose(arq);
}

//a funcao recebe um ponteiro do tipo pixel que aponta para uma das posicoes de nossa matriz
void *colorToGrey(void *pixel_){
    pixel *p = (pixel *) pixel_;
    int C = (p->R)*0.3+(p->G)*0.59+(p->B)*0.11; //Faz a transposição para tons de cinza
    p->R = C; p->G = C; p->B = C; // atualiza o pixel
    pthread_exit(NULL);
}

// funcao para printar a matriz de pixels
void printPixels(pixel **pixels, int l, int c){
    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            printf("[%d, %d, %d]  ", pixels[i][j].R, pixels[i][j].G, pixels[i][j].B);
        }
        printf("\n");
    }
}

void thread(pixel **pixels, int l, int c){
    // como vamos criar uma thread para processar cada pixel, a quantidade total de thread vai ser o tamanho da matriz
    int num_threads = l*c;

    //Para facilitar o entendimento, em vez de criar um vetor de threads, criamos uma matriz onde cada thread na matriz será responsavel pelo pixel
    //correspondente na matriz de pixels
    pthread_t **threads = NULL;
    threads = (pthread_t **) malloc(l*sizeof(pthread_t *));
    if(threads == NULL){
        exit(-2);
    }
    for(int i=0; i<l; i++){
        threads[i] = (pthread_t *) malloc(c*sizeof(pthread_t));
        if(threads[i] == NULL){
            exit(-2);
        }
    }
    

    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            // criamos a thread e passamos para ela o pixel e a funcao colorToGrey que o processará
            int ret = pthread_create(&threads[i][j], NULL, colorToGrey, (void *) &pixels[i][j]); 
        }
    }

    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            //join nas threads
            pthread_join(threads[i][j], NULL);
        }
    }

    for(int i=0; i<l; i++){
        free(threads[i]);
    }
    free(threads);

}

void write(pixel **pixels, int l, int c, int max, char *cod){

    //abre o arquivo de saida
    FILE *arq = NULL;
    arq = fopen("out.ppm", "wt");
    if(arq == NULL){
        exit(-3);
    }
    fprintf(arq, "%s", cod); //codigo
    fprintf(arq, "%d %d\n", c, l);// colunas e linhas
    fprintf(arq, "%d\n", max);// valor limite
    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            fprintf(arq, "%d %d %d\n", pixels[i][j].R, pixels[i][j].G, pixels[i][j].B); // printa cada pixel RGB
        }
    }
    fclose(arq);
}


int main (){   
  
    pixel **pixels = NULL;
    int l, c, max;
    char cod[5];

    //leitura do arquivo
    read(&pixels, &l, &c, &max, cod);

    printPixels(pixels, l, c);
    printf("\n");

    //Chama as threads e processa o arquivo lido
    thread(pixels, l, c);
    
    printPixels(pixels, l, c);

    //Escreve o resultado obtido no arquivo out.ppm
    write(pixels, l, c, max, cod);
    
    for(int i=0; i<l; i++){
        free(pixels[i]);
    }
    free(pixels);

    // chamando exit(0) ao invés de pthread_exit pois com exit(0) estamos explicitamente terminando o processo.
    exit(0); 

    return 0;
}