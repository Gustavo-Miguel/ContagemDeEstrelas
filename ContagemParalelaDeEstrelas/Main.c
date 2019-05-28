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
int conta = 0;

// Caminho das imagens 
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

// Algoritmo de rotulamento

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
                if (b_img[i][j - 1] != 0 || b_img[i - 1][j] != 0 || b_img[i - 1][j - 1] != 0) { // se os vizinhos ja estiverem rotulados                        
                    if (b_img[i][j - 1] != 0 && b_img[i - 1][j] != 0) { // se os dois vizinhos forem rotulados
                        if (b_img[i][j - 1] != b_img[i - 1][j]) { // verifica se tem o mesmo rotulo                     
                            contagem--; // se tiver a contagem de estrelas eh decrementada
                        }
                    }
                    if (b_img[i][j - 1] != 0) { // condicional para nao atribuir zero ao rotulo
                        b_img[i][j] = b_img[i][j - 1];
                    } else if (b_img[i - 1][j] != 0) {
                        b_img[i][j] = b_img[i - 1][j];
                    } else {
                        b_img[i][j] = b_img[i - 1][j - 1];
                    }
                } else {
                    b_img[i][j] = rotulo; // se nao houver vizinhos rotulados é uma nova estrela
                    rotulo++;
                    contagem++;
                }
            }
        }
    }
    conta++;
    printf("Rotulou %d vezes\n\n", conta);
}

void main(int argc, char** argv) {

    int myID, qtdProcessos;
    int contador_TOTAL = 0;


    MPI_Request req = MPI_REQUEST_NULL;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myID);
    MPI_Comm_size(MPI_COMM_WORLD, &qtdProcessos);


    int nSlave = 1;
    int caminho = 0;
    double tempo = MPI_Wtime();

    for (caminho = 0; caminho < 1; caminho++) {

        // Abre imagem
        FILE *img;
        img = fopen(path[caminho], "r");

        // Cabeçalho arquivo pgm
        char p[10];
        char comentario[100];
        int lin = 0, col = 0, tons = 0;
        fscanf(img, "%s", &p);
        fscanf(img, "%s", &comentario);
        fgets(comentario, 300, img);
        fscanf(img, "%d %d", &lin, &col);
        fscanf(img, "%d", &tons);

        int contBloco = 0;

        // Auxiliar para dividir blocos da imagem para tamanho que o compilador suporta                
        int aux_block = 1220;
        
        // Para que a quantidade de blocos possa ser dividida pela quantidade de processos
        while (aux_block%(qtdProcessos -1) != 0){
            aux_block++;
        }
        
        aux_block--;

//        printf("AuxBlock: %d\n", aux_block);
//        getchar();
        
        int k;
        int img_rcv[T_Block][T_Block];

        for (k = 0; k < aux_block + 1; k++) {
            int b_img[T_Block][T_Block];
            if (myID == ROOT) {

                int i = 0, j = 0;

                // Le bloco da imagem
                for (i = 0; i < T_Block; i++) {
                    for (j = 0; j < T_Block; j++) {
                        fscanf(img, "%d ", &b_img[i][j]);
                    }
                }

                // Envia para o slave o bloco lido
                printf("Enviou o bloco: %d para o slave %d\n", b_img[511][511], nSlave);

                MPI_Send(b_img, T_Block * T_Block, MPI_INT, nSlave, 0, MPI_COMM_WORLD);

                // Se chegar no ultimo slave, manda para o primeiro               
                if (nSlave == qtdProcessos - 1) {
                    nSlave = 1;
                } else {
                    nSlave++;
                }

            } else {
                //                MPI_Bcast(img_rcv, T_Block * T_Block, MPI_INT, ROOT, MPI_COMM_WORLD);
                //                rotulamento(img_rcv);
                if (contBloco <= (aux_block / (qtdProcessos - 1))) {
                    MPI_Recv(img_rcv, T_Block * T_Block, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    printf("Recebeu bloco: %d\n", img_rcv[511][511]);
                    rotulamento(img_rcv);
                    contBloco++;
                }
            }
        }
    }

    // Retorna a qtd de estrelas dos slaves para o root
    if (myID != ROOT) {
        MPI_Send(&contagem, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);

    } else {
        // Soma a quantidade de estrelas retornada pelo slaves
        int m;
        for (m = 0; m < qtdProcessos - 1; m++) {
            MPI_Recv(&contador_TOTAL, 1, MPI_INT, nSlave, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            contagem = contagem + contador_TOTAL;
            printf("Chegou para o root %d, soma %d\n", contador_TOTAL, contagem);
            nSlave++;
        }
    }

    //    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

}