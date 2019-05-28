

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

// Tamanho do bloco
#define T_Block 512

// Variaiveis para MPI
#define ROOT 0

// Contagem de estrelas e rotulos do algoritmo de rotulamento
int rotulo = 2;
int contagem = 2;

int imagem = 0;
char path[20][100] = {
    "/home/a14013/Área de Trabalho/PGM/1_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/2_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/3_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/4_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/5_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/6_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/7_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/8_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/9_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/10_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/11_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/12_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/13_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/14_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/15_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/16_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/17_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/18_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/19_pgm_ascii.pgm",
    "/home/a14013/Área de Trabalho/PGM/20_pgm_ascii.pgm"

};

// algoritmo de rotulamento

void rotulamento(int b_img[T_Block][T_Block]) {

    int i, j;

    for (i = 1; i < T_Block - 1; i++) {
        for (j = 1; j < T_Block - 1; j++) {

            if (b_img[i][j] > 127) {
                b_img[i][j] = 1;
            } else {
                b_img[i][j] = 0;
            }

            if (b_img[i][j] == 1) { // se achar um elemento nao escuro
                if (b_img[i][j - 1] != 0 || b_img[i - 1][j] != 0) { // se os vizinhos ja estiverem rotulados                        
                    if (b_img[i][j - 1] != 0 && b_img[i - 1][j] != 0) { // se os dois vizinhos forem rotulados
                        if (b_img[i][j - 1] != b_img[i - 1][j]) { // verifica se tem o mesmo rotulo                     
                            contagem--; // se tiver a contagem de estrelas eh decrementada
                        }
                    }
                    if (b_img[i][j - 1] != 0) { // condicional para nao atribuir zero ao rotulo
                        b_img[i][j] = b_img[i][j - 1];
                    } else {
                        b_img[i][j] = b_img[i - 1][j];
                    }
                } else {
                    b_img[i][j] = rotulo; // se nao houver vizinhos rotulados é uma nova estrela
                    rotulo++;
                    contagem++;
                }
            }
        }
    }
    printf("Contagem: %d\n", contagem);
}

void printaMatriz(int b[T_Block][T_Block]) {
    int i, j;
    for (i = 0; i < T_Block; i++) {
        for (j = 0; j < T_Block; j++) {
            if (b[i][ j] > 255 || b[i][j] < 0) {
                printf("%d ", b[i][j]);
            }
        }
        //        printf("\n");
    }
}

static int contBloco = 0;

void main(int argc, char** argv) {

    int myID, qtdProcessos;

    MPI_Request req = MPI_REQUEST_NULL;
    MPI_Status status;    
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myID);
    MPI_Comm_size(MPI_COMM_WORLD, &qtdProcessos);



    FILE *resultado;
    resultado = fopen("Resultados.txt", "w");


    int cam = 0;
    double tempoTotal = 0;
    double tempoImagem = 0;
    int estrelasImagem = 0;
    int gambis = 0;

    for (cam = 0; cam < 1; cam++) {

        tempoImagem = 0;
        FILE *img;
        img = fopen(path[cam], "r");

        if (cam == 19) {
            printf("IMAGEM: %d\n", cam + 1);
            getchar();
        }
        // Cabeçalho arquivo pgm
        char p[10];
        char comentario[100];
        int lin, col, tons;

        fscanf(img, "%s", &p);
        fscanf(img, "%s", &comentario);
        fgets(comentario, 300, img);
        fscanf(img, "%d %d", &lin, &col);
        fscanf(img, "%d", &tons);

        // Auxiliar para dividir blocos da imagem para tamanho que o compilador suporta
        int aux_block = lin / T_Block;

        if (gambis == 0) {
            printf("Quantidade de Blocos: %d\n\n", aux_block);
            getchar();
            gambis++;
        }
        // Percorre os blocos somando a qtd de estrelas e rotulos
        int k;
        int nSlave = 1;
        int img_rcv[T_Block][T_Block];
        

        for (k = 0; k < aux_block; k++) {

            printf("Terminou a iteração do %d\n", myID);
            if (myID == ROOT) {
                // Leitura e binarização da imagem
                int b_img[T_Block][T_Block];
                int i = 0, j = 0;

                for (i = 0; i < T_Block; i++) {
                    for (j = 0; j < T_Block; j++) {
                        fscanf(img, "%d ", &b_img[i][j]);
                    }
                }
                
                printf("O root irá enviar o bloco %d\n\n", k);
                
                MPI_Send(b_img, T_Block * T_Block, MPI_INT, nSlave, 0, MPI_COMM_WORLD);
                
                printf("O root enviou o bloco %d para o slave %d \n\n", k, nSlave);
                //getchar();
                
                if (nSlave == qtdProcessos - 1) {
                    nSlave = 1;
                } else {
                    nSlave++;
                }
                
            } else {
                printf("O processo %d irá rotular o %dº bloco: ", myID, contBloco);                
                MPI_Recv(img_rcv, T_Block * T_Block, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                rotulamento(img_rcv);
                printf("O processo %d rotulou o %dº bloco\n\n", myID, contBloco);
                contBloco++;                
            }
            
            printf("Terminou a iteração do %d\n", myID);
            

        }
        
    }

    //MPI_Barrier(MPI_COMM_WORLD);

    // Retorna a qtd de estrelas e a qtd de rotulos

    //MPI_Finalize();
    printf("Quantidade de Rotulos: %d\n", rotulo); // quantidade de rotulos utilizados
    printf("Quantidade de Estrelas: %d\n", contagem); // quantidade de estrelas


}

