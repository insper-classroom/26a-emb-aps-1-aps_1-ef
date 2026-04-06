#include "auxiliar.h"

#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/time.h"

/*
    INTEGRAÇÃO COM O LCD REAL

    Substitua o conteúdo destas funções pelas chamadas da sua biblioteca.
    Exemplo:
        void lcd_hw_init(void) {
            ili9341_init();
        }

        void lcd_preencher_tela_hw(uint16_t cor_rgb565) {
            ili9341_fill_screen(cor_rgb565);
        }
*/

__attribute__((weak)) void lcd_hw_init(void) {
    /* Inicialização real do LCD entra aqui */
}

__attribute__((weak)) void lcd_preencher_tela_hw(uint16_t cor_rgb565) {
    (void)cor_rgb565;
    /* Preenchimento real da tela entra aqui */
}

static uint16_t converter_cor_lcd(cor_t cor) {
    switch (cor) {
        case COR_VERMELHO: return LCD_COR_VERMELHO;
        case COR_VERDE:    return LCD_COR_VERDE;
        case COR_AZUL:     return LCD_COR_AZUL;
        case COR_AMARELO:  return LCD_COR_AMARELO;
        default:           return LCD_COR_PRETO;
    }
}

static int botao_foi_pressionado(uint gpio) {
    if (gpio_get(gpio) == 0) {
        sleep_ms(30); /* debounce */
        if (gpio_get(gpio) == 0) {
            while (gpio_get(gpio) == 0) {
                sleep_ms(5);
            }
            sleep_ms(30); /* debounce na soltura */
            return 1;
        }
    }
    return 0;
}

void inicializar_botoes(void) {
    gpio_init(BTN_INICIO_PIN);
    gpio_set_dir(BTN_INICIO_PIN, GPIO_IN);
    gpio_pull_up(BTN_INICIO_PIN);

    gpio_init(BTN_VERMELHO_PIN);
    gpio_set_dir(BTN_VERMELHO_PIN, GPIO_IN);
    gpio_pull_up(BTN_VERMELHO_PIN);

    gpio_init(BTN_VERDE_PIN);
    gpio_set_dir(BTN_VERDE_PIN, GPIO_IN);
    gpio_pull_up(BTN_VERDE_PIN);

    gpio_init(BTN_AZUL_PIN);
    gpio_set_dir(BTN_AZUL_PIN, GPIO_IN);
    gpio_pull_up(BTN_AZUL_PIN);

    gpio_init(BTN_AMARELO_PIN);
    gpio_set_dir(BTN_AMARELO_PIN, GPIO_IN);
    gpio_pull_up(BTN_AMARELO_PIN);
}

void inicializar_lcd(void) {
    lcd_hw_init();
    lcd_preencher_tela_hw(LCD_COR_PRETO);
}

int botao_inicio_pressionado(void) {
    return botao_foi_pressionado(BTN_INICIO_PIN);
}

cor_t ler_botao_colorido(void) {
    if (botao_foi_pressionado(BTN_VERMELHO_PIN)) return COR_VERMELHO;
    if (botao_foi_pressionado(BTN_VERDE_PIN))    return COR_VERDE;
    if (botao_foi_pressionado(BTN_AZUL_PIN))     return COR_AZUL;
    if (botao_foi_pressionado(BTN_AMARELO_PIN))  return COR_AMARELO;

    return COR_NENHUMA;
}

void lcd_preencher_preto(void) {
    lcd_preencher_tela_hw(LCD_COR_PRETO);
}

void lcd_mostrar_cor(cor_t cor) {
    lcd_preencher_tela_hw(converter_cor_lcd(cor));
}

void lcd_mostrar_cor_tempo(cor_t cor, uint32_t tempo_ms) {
    lcd_mostrar_cor(cor);
    sleep_ms(tempo_ms);
    lcd_preencher_preto();
}

void lcd_mostrar_sequencia(const cor_t lista[], int tamanho, uint32_t tempo_cor_ms, uint32_t intervalo_ms) {
    for (int i = 0; i < tamanho; i++) {
        lcd_mostrar_cor(lista[i]);
        sleep_ms(tempo_cor_ms);

        lcd_preencher_preto();
        sleep_ms(intervalo_ms);
    }
}

void lcd_efeito_erro(void) {
    for (int i = 0; i < 3; i++) {
        lcd_preencher_tela_hw(LCD_COR_BRANCO);
        sleep_ms(120);
        lcd_preencher_tela_hw(LCD_COR_PRETO);
        sleep_ms(120);
    }
}

void lcd_efeito_vitoria(void) {
    lcd_preencher_tela_hw(LCD_COR_VERDE);
    sleep_ms(180);
    lcd_preencher_tela_hw(LCD_COR_AZUL);
    sleep_ms(180);
    lcd_preencher_tela_hw(LCD_COR_AMARELO);
    sleep_ms(180);
    lcd_preencher_tela_hw(LCD_COR_VERMELHO);
    sleep_ms(180);
    lcd_preencher_tela_hw(LCD_COR_PRETO);
}

void gerar_lista_aleatoria_cores(cor_t lista[], int tamanho) {
    uint32_t seed = time_us_32();
    seed ^= (to_ms_since_boot(get_absolute_time()) << 16);
    srand(seed);

    for (int i = 0; i < tamanho; i++) {
        lista[i] = (cor_t)(rand() % 4);
    }
}