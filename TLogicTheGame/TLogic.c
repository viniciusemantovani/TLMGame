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

// Definição dos pinos usados para o display:
const uint8_t I2C_SDA = 14;
const uint8_t I2C_SCL = 15;

// Definição dos pinos dos botões:
const uint8_t BUTTON_A = 5;
const uint8_t BUTTON_B = 6;

// Variáveis relacionadas ao cursor:
uint8_t posicao_atual; // Posição do cursor do jogador na matriz de LED.
uint8_t cor_atual; // Cor atual do cursor (0 - apagado, 1 - azul, 2 - verde, 3 - vermelho).

// Cores definitivas dos LEDs controlados pelo jogador (0 - apagado, 1 - azul, 2 - verde, 3 - vermelho).
uint8_t cor_led_0 = 0;
uint8_t cor_led_1 = 0;
uint8_t cor_led_8 = 0;
uint8_t cor_led_9 = 0;


// Cores corretas para os leds mutáveis
uint8_t cor_led_0_verify = 0;
uint8_t cor_led_1_verify = 0;
uint8_t cor_led_8_verify = 0;
uint8_t cor_led_9_verify = 0;

// Variáveis para controle do jogo:
bool new_fase = true; // Determina se uma nova fase deve ser iniciada ou não.
bool vitoria = true; // Determina se uma fase foi vencida ou perdida.

//----------------------------------------------------------------------------------------------------------------

/**
 * @brief Escreve um conjunto de strings no display dada uma mensagem.
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
 * @brief Gera o mapa inicial do jogo que permanece até o fim.
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

/** 
 * @brief Acende um led de acordo com valor da cor.
 * @param posicao indica a posição do led a ser acendido
 * @param cor_atual indica a cor do led do cursor
 */
void acendeLed(uint8_t posicao, uint8_t cor){
    switch(cor){
        case 0: npSetLED(posicao, 0, 0, 0);
                return;
        case 1: npSetLED(posicao, 0, 0, 200);
                return; 
        case 2: npSetLED(posicao, 0, 200, 0);
                return; 
        case 3: npSetLED(posicao, 200, 0, 0);
                return; 
    }
}

/**
 * @brief Movimenta o cursor na matriz com base no movimento do joystick.
 * @param posicao_atual indica a posição atual do cursor
 * @param cor_atual indica a cor do led do cursor
 * @param x indica a posição do joystick no eixo x
 * @param y indica a posição do joystick no eixo y
 * @return nova posição atual
 */
uint8_t movCursor(uint8_t posicao_atual, uint8_t cor_atual, uint16_t x, uint16_t y){
    if(posicao_atual == 0){
        if(x < 2047 - 1000){
            acendeLed(posicao_atual, cor_led_0);
            posicao_atual = 1;
            acendeLed(posicao_atual, cor_atual);
        } else if(y > 2047 + 1000){
            acendeLed(posicao_atual, cor_led_0);
            posicao_atual = 9;
            acendeLed(posicao_atual, cor_atual);
        }
    } else if(posicao_atual == 1){
        if(x > 2047 + 1000){
            acendeLed(posicao_atual, cor_led_1);
            posicao_atual = 0;
            acendeLed(posicao_atual, cor_atual);
        } else if(y > 2047 + 1000){
            acendeLed(posicao_atual, cor_led_1);
            posicao_atual = 8;
            acendeLed(posicao_atual, cor_atual);
        }
    }  else if(posicao_atual == 8){
        if(x > 2047 + 1000){
            acendeLed(posicao_atual, cor_led_8);
            posicao_atual = 9;
            acendeLed(posicao_atual, cor_atual);
        } else if(y < 2047 - 1000){
            acendeLed(posicao_atual, cor_led_8);
            posicao_atual = 1;
            acendeLed(posicao_atual, cor_atual);
        }
    }   else if(posicao_atual == 9){
        if(x < 2047 - 1000){
            acendeLed(posicao_atual, cor_led_9);
            posicao_atual = 8;
            acendeLed(posicao_atual, cor_atual);
        } else if(y < 2047 - 1000){
            acendeLed(posicao_atual, cor_led_9);
            posicao_atual = 0;
            acendeLed(posicao_atual, cor_atual);
        }
    }

    return posicao_atual;
    
}

/**
 * @brief Rotina de tratamento do timer de controle do botão A.
 */
bool btnARepeat(struct repeating_timer *t){
    static absolute_time_t click_time_A = 0;
    
    if(!gpio_get(BUTTON_A) && absolute_time_diff_us(click_time_A, get_absolute_time()) > 200000){
        click_time_A = get_absolute_time();
        cor_atual++;
        if(cor_atual > 3){
            cor_atual = 0;
        }
        acendeLed(posicao_atual, cor_atual);
    }

    return true;
}

/**
 * Verifica se o jogador venceu uma rodada
 */
bool verifyVictory(){
    if(cor_led_8 == cor_led_8_verify
    && cor_led_9 == cor_led_9_verify
    && cor_led_0 == cor_led_0_verify
    && cor_led_1 == cor_led_1_verify) return true;
    else return false;
}

/**
 * @brief Rotina de tratamento do timer de controle do botão B.
 */
bool btnBRepeat(struct repeating_timer *t){
    static absolute_time_t click_time_B = 0;
    static absolute_time_t click_time_JS = 0;
    
    if(!gpio_get(BUTTON_B) && absolute_time_diff_us(click_time_B, get_absolute_time()) > 200000){
        click_time_B = get_absolute_time();
        acendeLed(posicao_atual, cor_atual);
        switch(posicao_atual){
            case 0: cor_led_0 = cor_atual;
                    break;
            case 1: cor_led_1 = cor_atual;
                    break;      
            case 8: cor_led_8 = cor_atual;
                    break;  
            case 9: cor_led_9 = cor_atual;
                    break;      
        }
    }

    if(!gpio_get(22) && absolute_time_diff_us(click_time_JS, get_absolute_time()) > 200000){
        click_time_JS = get_absolute_time();
        new_fase = true;
        vitoria = verifyVictory();
    }

    return true;
}

/**
 * @brief Desenha o mapa de uma nova fase (aleatóriamente, seguindo parâmetros lógicos)
 */
void drawMap(){

    // Determina cores dos quatro leds do primeiro quadro aleatóriamente:
    uint32_t random = get_rand_32();
    int32_t up_left = abs(random);
    uint8_t led_up_left = up_left%4;

    random = get_rand_32();
    int32_t up_right = abs(random);
    uint8_t led_up_right = up_right%4;

    random = get_rand_32();
    int32_t bottom_right = abs(random);
    uint8_t led_bottom_right = bottom_right%4;

    random = get_rand_32();
    int32_t bottom_left = abs(random);
    uint8_t led_bottom_left = bottom_left%4;

    // Primeiro conjunto:
    acendeLed(24, led_up_left);
    acendeLed(23, led_up_right);
    acendeLed(16, led_bottom_right);
    acendeLed(15, led_bottom_left);

    // Determina o segundo quadro aleatoriamente mas com lógica baseada no primeiro quadro:

    random = get_rand_32();
    int32_t alteracao = abs(random);
    uint8_t alt2 = alteracao%3;
    
    switch(alt2){

        // Rotação sentido horário 90 graus:
        case 0:
                acendeLed(20, led_up_left);
                acendeLed(19, led_up_right);
                acendeLed(18, led_bottom_right);
                acendeLed(21, led_bottom_left);
                break;

        // Rotação sentido antihorário 90 graus:
        case 1:
                acendeLed(18, led_up_left);
                acendeLed(21, led_up_right);
                acendeLed(20, led_bottom_right);
                acendeLed(19, led_bottom_left);
                break;

        // Rotação sentido 180 graus:
        case 2:
                acendeLed(19, led_up_left);
                acendeLed(18, led_up_right);
                acendeLed(21, led_bottom_right);
                acendeLed(20, led_bottom_left);
                break;
    }

    // Determina o terceiro quadro aleatoriamente mas com lógica baseada nos primeiros dois quadros:

    random = get_rand_32();
    alteracao = abs(random);
    uint8_t alt3 = alteracao%2;

    // Caso tenha virado 90 no horário o segundo quadro:
    if(alt2 == 0){

        // Vira mais 90 no horário
        if(alt3 == 0){
            acendeLed(3, led_up_left);
            acendeLed(4, led_up_right);
            acendeLed(5, led_bottom_right);
            acendeLed(6, led_bottom_left);

            cor_led_8_verify = led_up_right;
            cor_led_9_verify = led_bottom_right;
            cor_led_0_verify = led_bottom_left;
            cor_led_1_verify = led_up_left;

        // Vira de volta 90 no antihorário
        } else{
            acendeLed(5, led_up_left);
            acendeLed(6, led_up_right);
            acendeLed(3, led_bottom_right);
            acendeLed(4, led_bottom_left);

            cor_led_8_verify = led_bottom_left;
            cor_led_9_verify = led_up_left;
            cor_led_0_verify = led_up_right;
            cor_led_1_verify = led_bottom_right;
        }
        
    }

    // Caso tenha virado 90 no antihorário o segundo quadro:
    else if(alt2 == 1){

        // Vira de volta 90 no horário
        if(alt3 == 0){
            acendeLed(5, led_up_left);
            acendeLed(6, led_up_right);
            acendeLed(3, led_bottom_right);
            acendeLed(4, led_bottom_left);

            cor_led_8_verify = led_up_right;
            cor_led_9_verify = led_bottom_right;
            cor_led_0_verify = led_bottom_left;
            cor_led_1_verify = led_up_left;

        // Vira mais 90 no antihorário
        } else{
            acendeLed(3, led_up_left);
            acendeLed(4, led_up_right);
            acendeLed(5, led_bottom_right);
            acendeLed(6, led_bottom_left);

            cor_led_8_verify = led_bottom_left;
            cor_led_9_verify = led_up_left;
            cor_led_0_verify = led_up_right;
            cor_led_1_verify = led_bottom_right;
        }
        
    }

    // Caso tenha virado 180 o segundo quadro:
    else if(alt2 == 2){

        // Vira mais 90 no horário
        if(alt3 == 0){
            acendeLed(4, led_up_left);
            acendeLed(5, led_up_right);
            acendeLed(6, led_bottom_right);
            acendeLed(3, led_bottom_left);

            cor_led_8_verify = led_up_right;
            cor_led_9_verify = led_bottom_right;
            cor_led_0_verify = led_bottom_left;
            cor_led_1_verify = led_up_left;

        // Vira mais 90 no antihorário
        } else{
            acendeLed(6, led_up_left);
            acendeLed(3, led_up_right);
            acendeLed(4, led_bottom_right);
            acendeLed(5, led_bottom_left);

            cor_led_8_verify = led_bottom_left;
            cor_led_9_verify = led_up_left;
            cor_led_0_verify = led_up_right;
            cor_led_1_verify = led_bottom_right;
        }
        
    }
}

/**
 * @brief Inicia uma nova fase.
 */
void inicioFase(uint8_t fase_atual, uint8_t *ssd, struct render_area frame_area){
    
    play_tone(BUZZER_PIN_B, 400, 200); // Som de início de fase

    //-------------------------------------------------------------------------

        // Escreve fase atual no display:
    char fase_str[17];

    sprintf(fase_str, "       %.2d        ", fase_atual);

    // Mensagem de jogo:
    char *jogando[] = {
        "   fase atual   ",
        "                ",
             fase_str,
        "                "};

    writeString(jogando, ssd, frame_area); // Escreve a mensagem no display

    //-------------------------------------------------------------------------

    drawMap();
}

int main(){

    uint8_t fase_atual = 0; // Define a fase em que o jogador se encontra (Não tem relação com nível de dificuldade).

        // Inicializacao geral:

    stdio_init_all();
    pwm_init_buzzer(BUZZER_PIN_A); // inicializa o buzzer A.
    pwm_init_buzzer(BUZZER_PIN_B); // inicializa o buzzer B.
    npInit(LED_PIN); // Inicializa matriz de LEDs.
    npClear(); // Limpa matriz de LEDs.
    npWrite(); // Efetiva limpeza.

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
    posicao_atual = 0; // Posição inicial do cursor na matriz de led,
    cor_atual = 1; // Inicia em azul.

    char fase_str[17]; // Armazena a fase atual em string.

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

    // Limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    clearDisplay(ssd, frame_area);

    //-------------------------------------------------------------------------

        // Pré-jogo:

    // Mensagem de welcoming:
    char *welcome[] = {
        " Bem-vindo ao   ",
        "                ",
        "    TLogic!     "};

    writeString(welcome, ssd, frame_area); // Escreve a mensagem de welcoming no display

    // Som de welcoming:
    play_tone(BUZZER_PIN_A, 300, 200);
    sleep_ms(100);
    play_tone(BUZZER_PIN_A, 300, 200);
    sleep_ms(3000);

    // Mensagem introdutória:
    char *begin[] = {
        "  Pressione A   ",
        "para prosseguir ",
        "  a cada etapa  ",
        "                "};

    writeString(begin, ssd, frame_area); // Escreve a mensagem introdutória no display

    // Aguarda pressionamento do botão A para começar tutorial.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);

    // Mensagem tutorial:
    char *tut[] = {
        "  A troca cor   ",
        "  B define cor  ",
        "  JS confirma   ",
        "                "};

    writeString(tut, ssd, frame_area); // Escreve a mensagem introdutória no display

    // Aguarda pressionamento do botão A para começar jogo.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);

    //-------------------------------------------------------------------------

        // Configurações de início de jogo:

    // Som de início de Jogo:
    play_tone(BUZZER_PIN_B, 450, 200);
    sleep_ms(100);
    play_tone(BUZZER_PIN_B, 450, 200);
    sleep_ms(1000);

    // Confiura mapa inicial:
    // startMap(); // Seta os Leds componentes do mapa inicial do jogo.
    // npWrite(); // Escreve os dados nos LEDs (nave na posicao inicial).

    // Acende cursor em azul:
    npSetLED(posicao_atual, 0, 0, 200);

    // Inicializa timers para verificação de botões:
    struct repeating_timer timer_A; // Timer para controle do botão A.
    add_repeating_timer_ms(100, btnARepeat, NULL, &timer_A); // Inicializa temporizador para controle do botão A.

    struct repeating_timer timer_B; // Timer para controle do botão B.
    add_repeating_timer_ms(100, btnBRepeat, NULL, &timer_B); // Inicializa temporizador para controle do botão B.

    //-------------------------------------------------------------------------

        // Loop do programa:

    while(true){

        if(new_fase){ // Verifica se está-se iniciando uma nova fase.
            sleep_ms(100);
            if(vitoria){
                fase_atual++;
                inicioFase(fase_atual, ssd, frame_area); // Inicia nova fase.
                vitoria = false;
                new_fase = false;
                printf("right\n");
            } else{
                fase_atual = 0;
                new_fase = true;
                vitoria = true;
                printf("wrong\n");
                //função de reinício...
            }
        }
        
        joystick_read_axis(&vrx_value, &vry_value); // Lê valores do joystick (0-4095)
        posicao_atual = movCursor(posicao_atual, cor_atual, vrx_value, vry_value);
        npWrite(); // Escreve os dados nos LEDs.
        sleep_us(100000);
    }
}
