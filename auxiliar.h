#ifndef AUXILIAR_H
#define AUXILIAR_H

#include <stdint.h>

typedef enum {
    COR_VERMELHO = 0,
    COR_VERDE,
    COR_AZUL,
    COR_AMARELO,
    COR_NENHUMA
} cor_t;

/* Ajuste os pinos conforme a sua montagem */
#define BTN_INICIO_PIN     12
#define BTN_VERMELHO_PIN   6
#define BTN_VERDE_PIN      7
#define BTN_AZUL_PIN       8
#define BTN_AMARELO_PIN    9

/* Cores RGB565 */
#define LCD_COR_VERMELHO   0xF800
#define LCD_COR_VERDE      0x07E0
#define LCD_COR_AZUL       0x001F
#define LCD_COR_AMARELO    0xFFE0
#define LCD_COR_PRETO      0x0000
#define LCD_COR_BRANCO     0xFFFF

void inicializar_botoes(void);
void inicializar_lcd(void);

int botao_inicio_pressionado(void);
cor_t ler_botao_colorido(void);

void lcd_preencher_preto(void);
void lcd_mostrar_cor(cor_t cor);
void lcd_mostrar_cor_tempo(cor_t cor, uint32_t tempo_ms);
void lcd_mostrar_sequencia(const cor_t lista[], int tamanho, uint32_t tempo_cor_ms, uint32_t intervalo_ms);
void lcd_efeito_erro(void);
void lcd_efeito_vitoria(void);

void gerar_lista_aleatoria_cores(cor_t lista[], int tamanho);

#endif