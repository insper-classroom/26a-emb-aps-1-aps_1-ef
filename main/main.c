/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

volatile int jogo_iniciado = 0;
#define btn_vermelho
#define btn_verde
#define btn_azul
#define btn_amarelo







int main() {
    int TAM_MAX = 100;

    const char *lista_cores[TAM_MAX];


    gerar_lista_aleatoria_cores(lista_cores, TAM_MAX);


    //for (int i = 0; i < TAM_MAX; i++) {
    //    printf("%s\n", lista[i]);
    //}

    stdio_init_all();




    while (true) {
        if(jogo_iniciado) {
            
        }
    }
}
