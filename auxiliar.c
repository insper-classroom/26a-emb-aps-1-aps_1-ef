#include "auxiliar.h"

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"

#include "tft_lcd_ili9341/gfx/gfx_ili9341.h"
#include "tft_lcd_ili9341/ili9341/ili9341.h"

#include "main/audioPerdeu.h"
#include "main/audioGanhou.h"

static volatile int tocando = 0;
static volatile bool pausado = false;
static volatile int wav_position = 0;

static volatile const uint8_t *audio_ptr = audioPerdeu;
static volatile uint32_t audio_len = audioPerdeu_len;

// Botões com IRQ + debounce (mesma lógica do exemplo funcional compartilhado)
static volatile cor_t botao_pendente = COR_NENHUMA;
static volatile bool botao_evento = false;
static volatile uint32_t botao_ultimo_irq_ms = 0;
static const uint32_t BOTAO_DEBOUNCE_MS = 180;

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

static void lcd_escrever_linha(int x, int y, const char *texto, uint16_t cor_texto) {
    gfx_setTextColor(cor_texto);
    gfx_drawText(x, y, (char *)texto);
}

// Mapeia pino GPIO para cor
static cor_t cor_por_gpio(uint gpio) {
    if (gpio == BTN_VERMELHO_PIN) return COR_VERMELHO;
    if (gpio == BTN_VERDE_PIN)    return COR_VERDE;
    if (gpio == BTN_AZUL_PIN)     return COR_AZUL;
    if (gpio == BTN_AMARELO_PIN)  return COR_AMARELO;
    return COR_NENHUMA;
}

// IRQ de botões com debounce
static void btn_callback(uint gpio, uint32_t events) {
    if (!(events & GPIO_IRQ_EDGE_FALL)) {
        return;
    }

    uint32_t agora = to_ms_since_boot(get_absolute_time());
    if ((agora - botao_ultimo_irq_ms) < BOTAO_DEBOUNCE_MS) {
        return;
    }
    botao_ultimo_irq_ms = agora;

    cor_t cor = cor_por_gpio(gpio);
    if (cor != COR_NENHUMA) {
        botao_pendente = cor;
        botao_evento = true;
    }
}

static cor_t mapear_botao_pressionado(void) {
    if (gpio_get(BTN_VERMELHO_PIN) == 0) return COR_VERMELHO;
    if (gpio_get(BTN_VERDE_PIN)    == 0) return COR_VERDE;
    if (gpio_get(BTN_AZUL_PIN)     == 0) return COR_AZUL;
    if (gpio_get(BTN_AMARELO_PIN)  == 0) return COR_AMARELO;
    return COR_NENHUMA;
}

static void pwm_interrupt_handler(void) {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));

    if (!tocando || pausado) {
        pwm_set_gpio_level(AUDIO_PIN, 0);
        return;
    }

    if (wav_position < ((int)audio_len << 3)) {
        int sample = ((int)audio_ptr[wav_position >> 3] - 128) * 2 + 128;

        if (sample < 0) sample = 0;
        if (sample > 255) sample = 255;

        pwm_set_gpio_level(AUDIO_PIN, (uint16_t)sample);
        wav_position++;
    } else {
        tocando = 0;
        wav_position = 0;
        pwm_set_gpio_level(AUDIO_PIN, 0);
    }
}

void audio_init_erro(void) {
    set_sys_clock_khz(125000, true);

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_clear_irq(slice);
    pwm_set_irq_enabled(slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 5.0f);
    pwm_config_set_wrap(&config, 255);

    pwm_init(slice, &config, true);
    pwm_set_gpio_level(AUDIO_PIN, 0);
}

static void tocar_audio(const uint8_t *samples, uint32_t length) {
    audio_ptr = samples;
    audio_len = length;
    wav_position = 0;
    tocando = 1;
    pausado = false;
}

void tocar_audio_erro(void) {
    tocar_audio(audioPerdeu, audioPerdeu_len);
}

void tocar_audio_perdeu(void) {
    tocar_audio(audioPerdeu, audioPerdeu_len);
}

void tocar_audio_ganhou(void) {
    tocar_audio(WAV_DATA, audio_Ganhou_LENGTH);
}

bool audio_tocando(void) {
    return tocando != 0;
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

    // IRQ para todos os botões (queda para GND)
    gpio_set_irq_enabled_with_callback(
        BTN_VERMELHO_PIN,
        GPIO_IRQ_EDGE_FALL,
        true,
        &btn_callback
    );
    gpio_set_irq_enabled(BTN_VERDE_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_AZUL_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_AMARELO_PIN, GPIO_IRQ_EDGE_FALL, true);
    irq_set_enabled(IO_IRQ_BANK0, true);

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

    audio_init_erro();

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

cor_t ler_botao_colorido(uint32_t timeout_ms) {
    // Espera por IRQ; se nao vier, faz polling de fallback
    botao_evento = false;
    botao_pendente = COR_NENHUMA;

    const uint32_t timeout_fallback_ms = 800;
    uint32_t inicio = to_ms_since_boot(get_absolute_time());

    while (true) {
        if (botao_evento) {
            cor_t cor = botao_pendente;
            botao_evento = false;
            botao_pendente = COR_NENHUMA;

            while (mapear_botao_pressionado() != COR_NENHUMA) {
                sleep_ms(5);
            }
            sleep_ms(20);
            return cor;
        }

        uint32_t agora = to_ms_since_boot(get_absolute_time());
        if ((agora - inicio) > timeout_ms) {
            return COR_NENHUMA;
        }

        if ((agora - inicio) > timeout_fallback_ms) {
            cor_t cor = mapear_botao_pressionado();
            if (cor != COR_NENHUMA) {
                sleep_ms(25);
                while (mapear_botao_pressionado() != COR_NENHUMA) {
                    sleep_ms(5);
                }
                sleep_ms(10);
                return cor;
            }
        }
        tight_loop_contents();
    }
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

void lcd_apagar(void) {
    gfx_fillRect(0, 0, LCD_LARGURA, LCD_ALTURA, LCD_COR_PRETO);
}

void lcd_mostrar_cor(cor_t cor) {
    gfx_fillRect(0, 0, LCD_LARGURA, LCD_ALTURA, cor_para_rgb565(cor));
}

void lcd_mostrar_cor_tempo(cor_t cor, uint32_t tempo_ms) {
    leds_apagar_todos();
    lcd_mostrar_cor(cor);
    led_ligar(cor);

    sleep_ms(tempo_ms);

    leds_apagar_todos();
    lcd_apagar();
}

void lcd_mostrar_sequencia(const cor_t lista[], int tamanho, uint32_t tempo_cor_ms, uint32_t intervalo_ms) {
    for (int i = 0; i < tamanho; i++) {
        leds_apagar_todos();
        lcd_apagar();

        lcd_mostrar_cor(lista[i]);
        led_ligar(lista[i]);

        sleep_ms(tempo_cor_ms);

        leds_apagar_todos();
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

modo_t lcd_tela_modo(void) {
    lcd_apagar();
    gfx_setTextSize(2);

    lcd_escrever_linha(40, 40, "SELECIONE O MODO", LCD_COR_BRANCO);
    lcd_escrever_linha(40, 90, "VERDE = FACIL", LCD_COR_VERDE);
    lcd_escrever_linha(40, 130, "VERMELHO = DIFICIL", LCD_COR_VERMELHO);
    lcd_escrever_linha(40, 170, "AMARELO = FACIL", LCD_COR_AMARELO);

    while (true) {
        cor_t botao = ler_botao_colorido(UINT32_MAX);
        if (botao == COR_VERDE || botao == COR_AMARELO) {
            lcd_apagar();
            return MODO_FACIL;
        }
        if (botao == COR_VERMELHO || botao == COR_AZUL) {
            lcd_apagar();
            return MODO_DIFICIL;
        }
    }
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

void lcd_tela_rodada_pontuacao(int rodada, int pontuacao, modo_t modo) {
    char buffer[32];

    lcd_apagar();
    gfx_setTextSize(2);

    snprintf(buffer, sizeof(buffer), "RODADA %d", rodada);
    lcd_escrever_linha(60, 80, buffer, LCD_COR_BRANCO);

    snprintf(buffer, sizeof(buffer), "PONTOS %d", pontuacao);
    lcd_escrever_linha(60, 115, buffer, LCD_COR_AMARELO);

    const char *modo_texto = (modo == MODO_FACIL) ? "FACIL" : "DIFICIL";
    uint16_t modo_cor = (modo == MODO_FACIL) ? LCD_COR_VERDE : LCD_COR_VERMELHO;
    lcd_escrever_linha(60, 150, modo_texto, modo_cor);

    sleep_ms(900);
    lcd_apagar();
}

void lcd_tela_jogue(void) {
    lcd_apagar();
    gfx_setTextSize(2);

    lcd_escrever_linha(100, 100, "REPITA", LCD_COR_AMARELO);

    sleep_ms(400);
    lcd_apagar();
}

void lcd_tela_erro(int pontuacao) {
    tocar_audio_perdeu();

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
    lcd_escrever_linha(110, 85, "ERROU", LCD_COR_VERMELHO);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "PONTOS %d", pontuacao);
    lcd_escrever_linha(80, 120, buffer, LCD_COR_AMARELO);

    sleep_ms(1200);
    lcd_apagar();
    leds_apagar_todos();
}

