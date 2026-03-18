#include <stdlib.h>
#include <time.h>
#include "auxiliar.h"

void gerar_lista_aleatoria_cores(const char *lista[], int tamanho) {
    static int iniciou = 0;
    const char *cores[] = {"vermelho", "verde", "azul", "amarelo"};

    if (!iniciou) {
        srand(time(NULL));
        iniciou = 1;
    }

    for (int i = 0; i < tamanho; i++) {
        lista[i] = cores[rand() % 4];
    }
}