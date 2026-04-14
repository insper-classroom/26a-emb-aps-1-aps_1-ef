#include "pico/stdlib.h"
#include "auxiliar.h"

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
            cor_t botao = ler_botao_colorido();

            lcd_mostrar_cor_tempo(botao, TEMPO_COR_JOGADOR_MS);

            if (botao != sequencia[i]) {
                errou = true;
                break;
            }
        }

        if (errou) {
            tocar_audio_perdeu();
            lcd_tela_erro();
            
            // Espera o áudio terminar antes de reiniciar
            while (audio_tocando()) {
                sleep_ms(10); // Pequena pausa para não sobrecarregar a CPU
            }
            
            iniciar_novo_jogo(sequencia, &rodada_atual);
            continue;
        }

        tocar_audio_ganhou();

        // Espera o áudio terminar antes de continuar
        while (audio_tocando()) {
            sleep_ms(10); // Pequena pausa para não sobrecarregar a CPU
        }

        rodada_atual++;

        if (rodada_atual > TAM_MAX_SEQUENCIA) {
            iniciar_novo_jogo(sequencia, &rodada_atual);
        }

        sleep_ms(PAUSA_NOVA_RODADA_MS);
    }

    return 0;
}
