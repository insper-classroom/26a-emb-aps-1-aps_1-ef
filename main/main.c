/**
 * Genius com suporte visual por LCD
 * Fluxo:
 * - espera botão central
 * - gera sequência nova ao iniciar
 * - mostra sequência no LCD
 * - lê botões coloridos
 * - compara resposta do jogador
 * - se errar, encerra e volta ao início
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "auxiliar.h"

#define TAM_MAX_SEQUENCIA      100
#define TEMPO_COR_SEQUENCIA_MS 500
#define INTERVALO_SEQUENCIA_MS 180
#define TEMPO_COR_JOGADOR_MS   220
#define PAUSA_ENTRE_RODADAS_MS 600

typedef enum {
    ESTADO_ESPERANDO_INICIO = 0,
    ESTADO_MOSTRANDO_SEQUENCIA,
    ESTADO_LENDO_JOGADOR,
    ESTADO_PROXIMA_RODADA,
    ESTADO_FIM_DE_JOGO
} estado_jogo_t;

static void iniciar_novo_jogo(cor_t sequencia[], int tamanho, int *rodada_atual) {
    gerar_lista_aleatoria_cores(sequencia, tamanho);
    *rodada_atual = 1;
}

int main(void) {
    stdio_init_all();

    cor_t sequencia[TAM_MAX_SEQUENCIA];
    int rodada_atual = 1;
    int indice_jogador = 0;
    estado_jogo_t estado = ESTADO_ESPERANDO_INICIO;

    inicializar_botoes();
    inicializar_lcd();

    lcd_preencher_preto();

    while (true) {
        switch (estado) {
            case ESTADO_ESPERANDO_INICIO:
                lcd_preencher_preto();

                while (!botao_inicio_pressionado()) {
                    sleep_ms(10);
                }

                iniciar_novo_jogo(sequencia, TAM_MAX_SEQUENCIA, &rodada_atual);
                estado = ESTADO_MOSTRANDO_SEQUENCIA;
                break;

            case ESTADO_MOSTRANDO_SEQUENCIA:
                sleep_ms(250);
                lcd_mostrar_sequencia(sequencia,
                                      rodada_atual,
                                      TEMPO_COR_SEQUENCIA_MS,
                                      INTERVALO_SEQUENCIA_MS);

                indice_jogador = 0;
                estado = ESTADO_LENDO_JOGADOR;
                break;

            case ESTADO_LENDO_JOGADOR:
                while (indice_jogador < rodada_atual) {
                    cor_t botao = ler_botao_colorido();

                    if (botao == COR_NENHUMA) {
                        sleep_ms(10);
                        continue;
                    }

                    lcd_mostrar_cor_tempo(botao, TEMPO_COR_JOGADOR_MS);

                    if (botao != sequencia[indice_jogador]) {
                        estado = ESTADO_FIM_DE_JOGO;
                        break;
                    }

                    indice_jogador++;
                }

                if (estado == ESTADO_LENDO_JOGADOR) {
                    estado = ESTADO_PROXIMA_RODADA;
                }
                break;

            case ESTADO_PROXIMA_RODADA:
                rodada_atual++;

                if (rodada_atual > TAM_MAX_SEQUENCIA) {
                    lcd_efeito_vitoria();
                    estado = ESTADO_ESPERANDO_INICIO;
                } else {
                    sleep_ms(PAUSA_ENTRE_RODADAS_MS);
                    estado = ESTADO_MOSTRANDO_SEQUENCIA;
                }
                break;

            case ESTADO_FIM_DE_JOGO:
                lcd_efeito_erro();
                sleep_ms(500);
                estado = ESTADO_ESPERANDO_INICIO;
                break;

            default:
                estado = ESTADO_ESPERANDO_INICIO;
                break;
        }
    }

    return 0;
}