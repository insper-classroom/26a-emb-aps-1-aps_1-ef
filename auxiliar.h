#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    COR_VERMELHO = 0,
    COR_VERDE,
    COR_AZUL,
    COR_AMARELO,
    COR_NENHUMA
} cor_t;

#define BTN_VERMELHO_PIN   10
#define BTN_VERDE_PIN      5
#define BTN_AZUL_PIN       9
#define BTN_AMARELO_PIN    7

#define LED_VERMELHO_PIN   2
#define LED_VERDE_PIN      4
#define LED_AZUL_PIN       8
#define LED_AMARELO_PIN    6

#define AUDIO_PIN          26

#define LITE               15
#define SCREEN_ROTATION    1

#define LCD_LARGURA        320
#define LCD_ALTURA         240

#define LCD_COR_VERMELHO   0xF800
#define LCD_COR_VERDE      0x07E0
#define LCD_COR_AZUL       0x001F
#define LCD_COR_AMARELO    0xFFE0
#define LCD_COR_PRETO      0x0000
#define LCD_COR_BRANCO     0xFFFF

#define TAM_MAX_SEQUENCIA  100

void hardware_init_genius(void);

void gerar_lista_aleatoria_cores(cor_t lista[], int tamanho);

cor_t ler_botao_colorido(void);

void leds_apagar_todos(void);
void led_ligar(cor_t cor);
void led_desligar(cor_t cor);

void lcd_apagar(void);
void lcd_mostrar_cor(cor_t cor);
void lcd_mostrar_cor_tempo(cor_t cor, uint32_t tempo_ms);
void lcd_mostrar_sequencia(const cor_t lista[], int tamanho, uint32_t tempo_cor_ms, uint32_t intervalo_ms);

void lcd_tela_inicio(void);
void lcd_tela_rodada(int rodada);
void lcd_tela_jogue(void);
void lcd_tela_erro(void);

void audio_init_erro(void);
void tocar_audio_erro(void);
void tocar_audio_perdeu(void);
void tocar_audio_ganhou(void);
bool audio_tocando(void);

#endif
