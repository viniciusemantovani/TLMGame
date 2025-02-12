#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "libraries/joystick.h"
#include "libraries/neopixel.h"
#include "libraries/buzzer_pwm1.h"
#include "libraries/inc/ssd1306.h"

//----------------------------------------------------------------------------------------------------------------

// Definição dos pinos usados para o display
const uint8_t I2C_SDA = 14;
const uint8_t I2C_SCL = 15;

// Definição dos pinos dos botões
const uint8_t BUTTON_A = 5;
const uint8_t BUTTON_B = 6;

//----------------------------------------------------------------------------------------------------------------

/**
 * @brief Escreve um conjunto de strings no display dada uma mensagem
 * @param text vetor de strings para serem impressos no display
 * @param ssd buffer do display
 * @param frame_area area do quadro
*/ 
void writeString(char **text, uint8_t *ssd, struct render_area frame_area){
    int y = 0;
    for (uint i = 0; i < count_of(*text); i++)
    {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);
}

/**
 * 
 */
void startMap(){

    // Linha vertical
    npSetLED(2, 200, 0, 0);
    npSetLED(7, 200, 0, 0);
    npSetLED(12, 200, 0, 0);
    npSetLED(17, 200, 0, 0);
    npSetLED(22, 200, 0, 0);

    // Linha horizontal
    npSetLED(10, 200, 0, 0);
    npSetLED(11, 200, 0, 0);
    npSetLED(13, 200, 0, 0);
    npSetLED(14, 200, 0, 0);
}

int main(){

        // Inicializacao geral:

    stdio_init_all();
    pwm_init_buzzer(BUZZER_PIN_A); // inicializa o buzzer A.
    pwm_init_buzzer(BUZZER_PIN_B); // inicializa o buzzer B.
    npInit(LED_PIN); // Inicializa matriz de LEDs.
    npClear(); // Limpa matriz de LEDs.

    // Inicializa botão A com pull_up:
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Inicializa botão B com pull_up:
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    setup_joystick(); //Inicaliza o joystick.
    uint16_t vrx_value, vry_value; // Para armazenar valores dos eixos x e y do joystick.

    //-------------------------------------------------------------------------

        // Mapa inicial:

    startMap(); // Seta os Leds componentes do mapa inicial do jogo.
    npWrite(); // Escreve os dados nos LEDs (nave na posicao inicial).

    //-------------------------------------------------------------------------

        // Inicializacao do display:
    
    i2cInitDisplay(I2C_SDA, I2C_SCL); // Inicialização do i2c e do display OLED

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    clearDisplay(ssd, frame_area);

    // Definicao de string para imprimir no display
    char *text[] = {
        "  Bem-vindos!   ",
        "  Embarcatech   "};

    writeString(text, ssd, frame_area); // Escreve a string no display

    //-------------------------------------------------------------------------

        // Loop do programa:

    while(true){
        joystick_read_axis(&vrx_value, &vry_value); // Lê valores do joystick (0-4095)
        npWrite(); // Escreve os dados nos LEDs.
        sleep_us(10000);
    }
}

