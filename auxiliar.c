#include "auxiliar.h"

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"

#include "tft_lcd_ili9341/gfx/gfx_ili9341.h"
#include "tft_lcd_ili9341/ili9341/ili9341.h"

static uint16_t cor_para_rgb565(cor_t cor) {
    switch (cor) {
        case COR_VERMELHO: return LCD_COR_VERMELHO;
        case COR_VERDE:    return LCD_COR_VERDE;
        case COR_AZUL:     return LCD_COR_AZUL;
        case COR_AMARELO:  return LCD_COR_AMARELO;
        default:           return LCD_COR_PRETO;
    }
}

static uint gpio_led_da_cor(cor_t cor) {
    switch (cor) {
        case COR_VERMELHO: return LED_VERMELHO_PIN;
        case COR_VERDE:    return LED_VERDE_PIN;
        case COR_AZUL:     return LED_AZUL_PIN;
        case COR_AMARELO:  return LED_AMARELO_PIN;
        default:           return LED_VERMELHO_PIN;
    }
}

static bool botao_pressionado_com_debounce(uint gpio) {
    if (gpio_get(gpio) == 0) {
        sleep_ms(25);

        if (gpio_get(gpio) == 0) {
            while (gpio_get(gpio) == 0) {
                sleep_ms(5);
            }
            sleep_ms(25);
            return true;
        }
    }
    return false;
}

static void lcd_escrever_linha(int x, int y, const char *texto, uint16_t cor_texto) {
    gfx_setTextColor(cor_texto);
    gfx_drawText(x, y, (char *)texto);
}

void hardware_init_genius(void) {
    stdio_init_all();

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

    gpio_init(LED_VERMELHO_PIN);
    gpio_set_dir(LED_VERMELHO_PIN, GPIO_OUT);

    gpio_init(LED_VERDE_PIN);
    gpio_set_dir(LED_VERDE_PIN, GPIO_OUT);

    gpio_init(LED_AZUL_PIN);
    gpio_set_dir(LED_AZUL_PIN, GPIO_OUT);

    gpio_init(LED_AMARELO_PIN);
    gpio_set_dir(LED_AMARELO_PIN, GPIO_OUT);

    leds_apagar_todos();

    gpio_init(LITE);
    gpio_set_dir(LITE, GPIO_OUT);
    gpio_put(LITE, 1);

    LCD_initDisplay();
    LCD_setRotation(SCREEN_ROTATION);

    gfx_init();
    gfx_clear();
    gfx_setTextSize(2);

    lcd_apagar();
}

void gerar_lista_aleatoria_cores(cor_t lista[], int tamanho) {
    uint32_t seed = time_us_32() ^ (uint32_t)to_ms_since_boot(get_absolute_time());
    srand(seed);

    for (int i = 0; i < tamanho; i++) {
        lista[i] = (cor_t)(rand() % 4);
    }
}

cor_t ler_botao_colorido(void) {
    if (botao_pressionado_com_debounce(BTN_VERMELHO_PIN)) return COR_VERMELHO;
    if (botao_pressionado_com_debounce(BTN_VERDE_PIN))    return COR_VERDE;
    if (botao_pressionado_com_debounce(BTN_AZUL_PIN))     return COR_AZUL;
    if (botao_pressionado_com_debounce(BTN_AMARELO_PIN))  return COR_AMARELO;

    return COR_NENHUMA;
}

void leds_apagar_todos(void) {
    gpio_put(LED_VERMELHO_PIN, 0);
    gpio_put(LED_VERDE_PIN, 0);
    gpio_put(LED_AZUL_PIN, 0);
    gpio_put(LED_AMARELO_PIN, 0);
}

void led_ligar(cor_t cor) {
    if (cor == COR_NENHUMA) return;
    gpio_put(gpio_led_da_cor(cor), 1);
}

void led_desligar(cor_t cor) {
    if (cor == COR_NENHUMA) return;
    gpio_put(gpio_led_da_cor(cor), 0);
}

void led_piscar(cor_t cor, uint32_t tempo_ms) {
    led_ligar(cor);
    sleep_ms(tempo_ms);
    led_desligar(cor);
}

void lcd_apagar(void) {
    gfx_fillRect(0, 0, LCD_LARGURA, LCD_ALTURA, LCD_COR_PRETO);
}

void lcd_mostrar_cor(cor_t cor) {
    gfx_fillRect(0, 0, LCD_LARGURA, LCD_ALTURA, cor_para_rgb565(cor));
}

void lcd_mostrar_cor_tempo(cor_t cor, uint32_t tempo_ms) {
    lcd_mostrar_cor(cor);
    led_ligar(cor);

    sleep_ms(tempo_ms);

    led_desligar(cor);
    lcd_apagar();
}

void lcd_mostrar_sequencia(const cor_t lista[], int tamanho, uint32_t tempo_cor_ms, uint32_t intervalo_ms) {
    for (int i = 0; i < tamanho; i++) {
        lcd_mostrar_cor(lista[i]);
        led_ligar(lista[i]);

        sleep_ms(tempo_cor_ms);

        led_desligar(lista[i]);
        lcd_apagar();

        sleep_ms(intervalo_ms);
    }
}

void lcd_tela_inicio(void) {
    lcd_apagar();
    gfx_setTextSize(2);

    lcd_escrever_linha(90, 60,  "GENIUS", LCD_COR_BRANCO);
    lcd_escrever_linha(40, 110, "MEMORIZE A", LCD_COR_VERDE);
    lcd_escrever_linha(55, 145, "SEQUENCIA", LCD_COR_VERDE);

    sleep_ms(1200);
    lcd_apagar();
}

void lcd_tela_rodada(int rodada) {
    char buffer[32];

    lcd_apagar();
    gfx_setTextSize(2);

    snprintf(buffer, sizeof(buffer), "RODADA %d", rodada);
    lcd_escrever_linha(80, 100, buffer, LCD_COR_BRANCO);

    sleep_ms(700);
    lcd_apagar();
}

void lcd_tela_jogue(void) {
    lcd_apagar();
    gfx_setTextSize(2);

    lcd_escrever_linha(100, 100, "REPITA", LCD_COR_AMARELO);

    sleep_ms(400);
    lcd_apagar();
}

void lcd_tela_erro(void) {
    for (int i = 0; i < 3; i++) {
        gfx_fillRect(0, 0, LCD_LARGURA, LCD_ALTURA, LCD_COR_BRANCO);
        gpio_put(LED_VERMELHO_PIN, 1);
        gpio_put(LED_VERDE_PIN, 1);
        gpio_put(LED_AZUL_PIN, 1);
        gpio_put(LED_AMARELO_PIN, 1);

        sleep_ms(120);

        gfx_fillRect(0, 0, LCD_LARGURA, LCD_ALTURA, LCD_COR_PRETO);
        leds_apagar_todos();

        sleep_ms(120);
    }

    gfx_setTextSize(2);
    lcd_escrever_linha(110, 105, "ERROU", LCD_COR_VERMELHO);
    sleep_ms(900);
    lcd_apagar();
    leds_apagar_todos();
}