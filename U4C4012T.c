#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ws2812.pio.h"

// -------------------------------------------------
// Definições de pinos
// -------------------------------------------------
#define BUTTON_A    5       // Botão "incrementa"
#define BUTTON_B    6       // Botão "decrementa"
#define WS2812_PIN  7       // Matriz 5x5 de LEDs WS2812
#define LED_RED     13      // LED que pisca a 5Hz
#define NUM_LEDS    25      // Tamanho da matriz (5x5)

// -------------------------------------------------
// Variáveis globais
// -------------------------------------------------
volatile int number = 0;  // Número exibido (0–9)
PIO pio = pio0;           // Instância de PIO
int sm  = 0;              // State machine da PIO

// Debouncing
absolute_time_t last_debounce_a;
absolute_time_t last_debounce_b;

// -------------------------------------------------
// Mapeamento físico: (linha, coluna) -> índice do LED
//                   de acordo com a sua descrição
// -------------------------------------------------
static const uint8_t LEDmap[5][5] = {
    {24, 23, 22, 21, 20},  // linha 0
    {15, 16, 17, 18, 19},  // linha 1
    {14, 13, 12, 11, 10},  // linha 2
    { 5,  6,  7,  8,  9},  // linha 3
    { 4,  3,  2,  1,  0}   // linha 4
};

// -------------------------------------------------
// Mapeamento dos números 0–9 (1 = LED aceso, 0 = apagado)
// Cada índice [i] corresponde a (linha = i/5, coluna = i%5),
// da esquerda p/ a direita, de cima p/ baixo.
// -------------------------------------------------
static const uint8_t numbers[10][NUM_LEDS] = {
    { // 0
        1,1,1,1,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,1,1,1,1
    },
    { // 1
        0,0,1,0,0,
        0,1,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,1,1,1,0
    },
    { // 2
        1,1,1,1,1,
        0,0,0,0,1,
        1,1,1,1,1,
        1,0,0,0,0,
        1,1,1,1,1
    },
    { // 3
        1,1,1,1,1,
        0,0,0,0,1,
        0,1,1,1,1,
        0,0,0,0,1,
        1,1,1,1,1
    },
    { // 4
        1,0,0,1,0,
        1,0,0,1,0,
        1,1,1,1,1,
        0,0,0,1,0,
        0,0,0,1,0
    },
    { // 5
        1,1,1,1,1,
        1,0,0,0,0,
        1,1,1,1,1,
        0,0,0,0,1,
        1,1,1,1,1
    },
    { // 6
        1,1,1,1,1,
        1,0,0,0,0,
        1,1,1,1,1,
        1,0,0,0,1,
        1,1,1,1,1
    },
    { // 7
        1,1,1,1,1,
        0,0,0,0,1,
        0,0,0,1,0,
        0,0,1,0,0,
        0,1,0,0,0
    },
    { // 8
        1,1,1,1,1,
        1,0,0,0,1,
        1,1,1,1,1,
        1,0,0,0,1,
        1,1,1,1,1
    },
    { // 9
        1,1,1,1,1,
        1,0,0,0,1,
        1,1,1,1,1,
        0,0,0,0,1,
        1,1,1,1,1
    }
};

// -------------------------------------------------
// Debounce
// -------------------------------------------------
bool debounce(uint gpio, absolute_time_t *last_time) {
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(*last_time, now) >= 200000) {
        *last_time = now;
        return (gpio_get(gpio) == 0); // Pressionado?
    }
    return false;
}

// -------------------------------------------------
// Callback único de interrupção
// -------------------------------------------------
void gpio_callback(uint gpio, uint32_t events) {
    // Botão A -> Incrementa
    if ((gpio == BUTTON_A) && (events & GPIO_IRQ_EDGE_FALL)) {
        if (debounce(BUTTON_A, &last_debounce_a)) {
            number = (number + 1) % 10;
        }
    }
    // Botão B -> Decrementa
    else if ((gpio == BUTTON_B) && (events & GPIO_IRQ_EDGE_FALL)) {
        if (debounce(BUTTON_B, &last_debounce_b)) {
            number = (number - 1 + 10) % 10;
        }
    }
}

// -------------------------------------------------
// Atualiza a matriz WS2812 respeitando o mapeamento
// -------------------------------------------------
void update_matrix() {
    // Se a fita estiver em formato GRB, 0x0000FF acenderá em Vermelho.
    // Você pode ajustar se quiser outra cor.
    uint32_t color_on  = 0x0000FF << 8;
    uint32_t color_off = 0x000000 << 8;

    // 1) Criamos um buffer local de 25 LEDs
    uint32_t ledBuffer[NUM_LEDS];
    for (int i = 0; i < NUM_LEDS; i++) {
        ledBuffer[i] = color_off; // Desligado por padrão
    }

    // 2) Preenche o ledBuffer[] de acordo com o "numbers[number]"
    //    e o mapeamento físico "LEDmap".
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int logicalIndex = row * 5 + col;   // 0..24 logicamente
            if (numbers[number][logicalIndex] == 1) {
                int physicalIndex = LEDmap[row][col];
                ledBuffer[physicalIndex] = color_on;
            }
        }
    }

    // 3) Enviar o buffer para a fita (LED 0 até LED 24)
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, ledBuffer[i]);
    }
}

// -------------------------------------------------
// Piscar LED vermelho a cada 200 ms (5 Hz)
// -------------------------------------------------
bool blink_led_cb(struct repeating_timer *t) {
    static bool state = false;
    gpio_put(LED_RED, state);
    state = !state;
    return true; // Repetir sempre
}

// -------------------------------------------------
// Configuração do sistema
// -------------------------------------------------
void setup() {
    stdio_init_all();

    last_debounce_a = get_absolute_time();
    last_debounce_b = get_absolute_time();

    // LED que pisca
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    // Botão A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Botão B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Interrupções (callback único)
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);

    // PIO para WS2812
    sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
}

// -------------------------------------------------
// Função Principal
// -------------------------------------------------
int main() {
    setup();

    // Timer para piscar o LED no pino 13 a 5Hz
    struct repeating_timer timer;
    add_repeating_timer_ms(200, blink_led_cb, NULL, &timer);

    while (true) {
        update_matrix();
        sleep_ms(200);
    }
    return 0;
}
