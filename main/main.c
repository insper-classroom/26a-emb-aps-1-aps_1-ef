#include "pico/stdlib.h"
#include "auxiliar.h"

#define TEMPO_COR_SEQUENCIA_MS_FACIL   600
#define INTERVALO_SEQUENCIA_MS_FACIL    250
#define TEMPO_MAX_ESPERA_MS_FACIL      12000

#define TEMPO_COR_SEQUENCIA_MS_DIFICIL  400
#define INTERVALO_SEQUENCIA_MS_DIFICIL  150
#define TEMPO_MAX_ESPERA_MS_DIFICIL     8000

#define TEMPO_COR_JOGADOR_MS            220
#define PAUSA_NOVA_RODADA_MS            500
#define PAUSA_REINICIO_MS              1000

static void iniciar_novo_jogo(cor_t sequencia[], int *rodada_atual) {
    gerar_lista_aleatoria_cores(sequencia, TAM_MAX_SEQUENCIA);
    *rodada_atual = 1;
}

int main(void) {
    cor_t sequencia[TAM_MAX_SEQUENCIA];
    int rodada_atual = 1;
    int pontuacao = 0;

    hardware_init_genius();
    lcd_tela_inicio();
    modo_t modo = lcd_tela_modo();
    iniciar_novo_jogo(sequencia, &rodada_atual);

    uint32_t tempo_cor_sequencia_ms = (modo == MODO_FACIL) ? TEMPO_COR_SEQUENCIA_MS_FACIL : TEMPO_COR_SEQUENCIA_MS_DIFICIL;
    uint32_t intervalo_sequencia_ms = (modo == MODO_FACIL) ? INTERVALO_SEQUENCIA_MS_FACIL : INTERVALO_SEQUENCIA_MS_DIFICIL;
    uint32_t tempo_max_espera_ms = (modo == MODO_FACIL) ? TEMPO_MAX_ESPERA_MS_FACIL : TEMPO_MAX_ESPERA_MS_DIFICIL;

    while (true) {
        lcd_tela_rodada_pontuacao(rodada_atual, pontuacao, modo);

        lcd_mostrar_sequencia(
            sequencia,
            rodada_atual,
            tempo_cor_sequencia_ms,
            intervalo_sequencia_ms
        );

        lcd_tela_jogue();

        bool errou = false;

        for (int i = 0; i < rodada_atual; i++) {
            cor_t botao = ler_botao_colorido(tempo_max_espera_ms);

            if (botao == COR_NENHUMA) {
                errou = true;
                break;
            }

            lcd_mostrar_cor_tempo(botao, TEMPO_COR_JOGADOR_MS);

            if (botao != sequencia[i]) {
                errou = true;
                break;
            }
        }

        if (errou) {
            lcd_tela_erro(pontuacao);
            
            // Espera o áudio terminar antes de reiniciar
            while (audio_tocando()) {
                sleep_ms(10); // Pequena pausa para não sobrecarregar a CPU
            }
            
            pontuacao = 0;
            iniciar_novo_jogo(sequencia, &rodada_atual);
            continue;
        }

        tocar_audio_ganhou();

        // Espera o áudio terminar antes de continuar
        while (audio_tocando()) {
            sleep_ms(10); // Pequena pausa para não sobrecarregar a CPU
        }

        pontuacao++;
        rodada_atual++;

        if (rodada_atual > TAM_MAX_SEQUENCIA) {
            pontuacao = 0;
            iniciar_novo_jogo(sequencia, &rodada_atual);
        }

        sleep_ms(PAUSA_NOVA_RODADA_MS);
    }

    return 0;
}
