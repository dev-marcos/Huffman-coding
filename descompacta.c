#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Head{
    uint8_t tamanho;
    bool bit[32];
} Head;


typedef struct No{
    char data;
    struct No *esquerda;
    struct No *direita;
} No;

void freeTree(No *no){

    if (no!=NULL){
        freeTree(no->esquerda);
        freeTree(no->direita);
        free(no);
    }else
        return;
}

No *novoNo(char c, No *esquerda, No *direita){

    No *novo;

    if ( ( novo = malloc(sizeof(*novo)) ) == NULL ) 
        return NULL;

    novo->data = c;
    novo->esquerda = esquerda;
    novo->direita = direita;

    return novo;
}

char codigoToChar(uint8_t codigo){

    if (codigo<=25) return codigo+97; //caractere
    else if (codigo==26) return 32;   //Espaco
    else if (codigo==27) return 10;   //Quebra de Linha
    else if (codigo==28) return EOF;  //EOF
    else return 0;   
}

void intToBin(uint8_t byte, bool *bin){
    for(int i=8; i>0; i--){
            bin[i-1]=byte%2;
            byte=byte/2;
        }
}

void gerarHeader(Head *head, FILE *fEntrada){

    uint8_t byte;

    for (int i=0; i<29; i++){

        //Le os 1 byte
        fread (&head[i].tamanho, sizeof(uint8_t), 1, fEntrada);

        //Le os outros 4 byte
        for (int j=0; j<4; j++){

            fread (&byte, sizeof(uint8_t), 1, fEntrada);
            //transforma o byte lido em binario
            intToBin(byte, &head[i].bit[(j*8)]);

        }
    }
}

void printHead(Head *head){
    for (int i=0; i<29; i++){
        if (i<=26)
            printf("%c: ",codigoToChar(i));
        if (i==27)
            printf("\\n: ");
        if (i==28)
            printf("EOF: ");

        for(int altura=0; altura<head[i].tamanho; altura++)
            printf("%d",head[i].bit[altura]); 

        printf("\n");
    }
}

No * geraArvore(Head *head){

    No *Raiz = novoNo(0, NULL, NULL);
    
    No * noAtualRaiz;

    for (int i=0; i<29; i++){

        noAtualRaiz = Raiz;
        //Insere o caractere na Arvore
        for(int altura=0; altura<head[i].tamanho; altura++){
            
            if (head[i].bit[altura]){//vai para direta
                if (noAtualRaiz->direita != NULL){
                    noAtualRaiz = noAtualRaiz->direita;
                }else{
                    No *aux = novoNo(0, NULL, NULL);
                    noAtualRaiz->direita = aux;
                    noAtualRaiz = noAtualRaiz->direita;
                }
           }else{ //vai para esquerda
                if (noAtualRaiz->esquerda != NULL){
                    noAtualRaiz = noAtualRaiz->esquerda;
                }else{
                    No *aux = novoNo(0, NULL, NULL);
                    noAtualRaiz->esquerda = aux;
                    noAtualRaiz = noAtualRaiz->esquerda;
                }
           }
       }
       noAtualRaiz->data = codigoToChar(i);
    }
    return Raiz;
}

void decodifica(No * Raiz, FILE * fEntrada, FILE * fSaida){
    No *noAtualRaiz = Raiz;

    uint8_t byte;

    bool buffer[8]; //uso para armazenar o binario lido do bloco de 1 byte
 
    //Roda um While no arquivo de entrada até acabar
    while (fread (&byte, sizeof(uint8_t), 1, fEntrada) == 1) {

        intToBin(byte, &buffer[0]); //converte o inteiro para binario
        
        for(int i=0; i<8; i++){ //roda para cada bit do byte
            if(noAtualRaiz->data != 0){ //Caso encontre um nó terminal
                if (noAtualRaiz->data != EOF) //Caso nao seja fim do arquivo
                    fwrite(&noAtualRaiz->data,  sizeof(char), 1, fSaida);//grave no arquivo
                else
                    i=8; //solucao simples para parar o for kkk

                noAtualRaiz = Raiz; //Começa no pai da arvore de novo
            }

            if(buffer[i])//atraves do bit, eu sei para qual lado devo descer
                noAtualRaiz = noAtualRaiz->direita;
            else
                noAtualRaiz = noAtualRaiz->esquerda;
        }
    }
}

void Descomprimir(const char *entrada, const char *saida){

    FILE *fEntrada = fopen(entrada, "rb");
    if(fEntrada == NULL){
        printf("O sistema nao pode encontrar o arquivo especificado.\n\n");
        exit(1);
    }

    FILE *fSaida   = fopen(saida,   "wb");
    if(fSaida == NULL){
        printf("O sistema nao pode armazenar no destino especificado.\n\n");
        exit(1);
    }

    //criei uma estrutura para armazenar o header que vem do arquivo de entrada,
    //achei que assim, seria uma solucao mais simples para resolver o exercicio
    Head head[29];
    
    // Antes de transformar tudo em funcao, o geraHeader, printHeader e geraArvore, rodavam
    // dentro do mesmos for. Entretanto, mudei para ficar mais facil a compreensao

    //Le o arquivo de entrada e armazena os codigos na variavel do head
    gerarHeader(&head[0], fEntrada);

    //imprime o conteudo do head conforme o enunciado que foi passado
    printHead(&head[0]);

    //gera a Arvore de Huffman com os codigos do head
    No *Raiz = geraArvore(&head[0]);

    //Gera o arquivo de saida
    decodifica(Raiz, fEntrada, fSaida);

    //Esvazia a raiz da memoria
    freeTree(Raiz);

    //fecha os arquivos
    fclose(fEntrada);
    fclose(fSaida);
}

int main(int argc, char *argv[]){

    if (argc < 2){
        printf("A sintaxe do comando esta incorreta.\n\n");
        printf("descompacta [origem] [destino].\n\n");
        printf("origem\tEspecifica o arquivo compactado.\n");
        printf("destino\tEspecifica o nome do novo arquivo gerado.\n\n");
        return 0;
    }
    Descomprimir(argv[1], argv[2]);

    return 0;
}
