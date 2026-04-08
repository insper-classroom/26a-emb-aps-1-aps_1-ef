#include "pico/stdlib.h"
#include "auxiliar.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define TEMPO_COR_SEQUENCIA_MS   500
#define INTERVALO_SEQUENCIA_MS   200
#define TEMPO_COR_JOGADOR_MS     220
#define PAUSA_NOVA_RODADA_MS     500
#define PAUSA_REINICIO_MS        1000

static void iniciar_novo_jogo(cor_t sequencia[], int *rodada_atual) {
    gerar_lista_aleatoria_cores(sequencia, TAM_MAX_SEQUENCIA);
    *rodada_atual = 1;
}

int main(void) {
    cor_t sequencia[TAM_MAX_SEQUENCIA];
    int rodada_atual = 1;

    hardware_init_genius();
    lcd_tela_inicio();

    iniciar_novo_jogo(sequencia, &rodada_atual);

    while (true) {
        lcd_tela_rodada(rodada_atual);

        lcd_mostrar_sequencia(
            sequencia,
            rodada_atual,
            TEMPO_COR_SEQUENCIA_MS,
            INTERVALO_SEQUENCIA_MS
        );

        lcd_tela_jogue();

        bool errou = false;

        for (int i = 0; i < rodada_atual; i++) {
            cor_t botao = COR_NENHUMA;

            while (botao == COR_NENHUMA) {
                botao = ler_botao_colorido();
                sleep_ms(10);
            }

            lcd_mostrar_cor_tempo(botao, TEMPO_COR_JOGADOR_MS);

            if (botao != sequencia[i]) {
                errou = true;
                break;
            }
        }

        if (errou) {
            lcd_tela_erro();
            sleep_ms(PAUSA_REINICIO_MS);
            iniciar_novo_jogo(sequencia, &rodada_atual);
            continue;
        }

        rodada_atual++;

        if (rodada_atual > TAM_MAX_SEQUENCIA) {
            iniciar_novo_jogo(sequencia, &rodada_atual);
        }

        sleep_ms(PAUSA_NOVA_RODADA_MS);
    }

    return 0;
}