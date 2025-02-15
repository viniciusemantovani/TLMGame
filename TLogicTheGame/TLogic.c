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

// Variável para escolha entre jogos:
bool game; // true - lógica pura, false - lógica matemática;

// Variáveis para controle dos jogos:
bool new_fase = true; // Determina se uma nova fase deve ser iniciada ou não.
bool vitoria = true; // Determina se uma fase foi vencida ou perdida.

bool fst_logic; // Indica se é o jogo de logica está sendo iniciado pela primeira vez ou após o outro jogo.
bool fst_math; // Indica se é o jogo de matemática está sendo iniciado pela primeira vez ou após o outro jogo.
//---------------------------------------------------------------------------------------------------------------

    // Variáveis globais para jogo de lógica:

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

//---------------------------------------------------------------------------------------------------------------

    // Variáveis globais para jogo de matemática:

int16_t resultado_correto; // Resultado correto do cálculo.

int8_t alg[5]; // algarismos da resposta do jogador.

uint8_t which_digit; // Armazena o número do dígito que está sendo editado pelo jogador no momento.

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
 * @brief Organiza um conjunto de strings e envia para serem escritas no display.
 * @param str1 primeira linha
 * @param str2 segunda linha
 * @param str3 terceira linha
 * @param ssd buffer do display
 * @param frame_area area do quadro
*/ 
void organizeStrings(char *str1, char *str2, char *str3, uint8_t *ssd, struct render_area frame_area){
    char *msg[] = {str1, str2, str3, "                "};
    writeString(msg, ssd, frame_area); // Escreve a mensagem no display
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
        default: return;
    }
}

/**
 * @brief Acende os Leds corretos dado um inteiro (para reproduzí-lo na matriz).
 * @param num número a ser reproduzido na matriz (se for 32760 é multiplicação, 32761 é divisão, 32762 é soma, 32763 é subtração)
 */
void alfabetoNum(int16_t num){
    uint16_t div = 10;

    if(num < 32760){
        while(num/div != 0) div = div*10; // Busca primeiro múltiplo de 10 maior que num.
    }

    int16_t aux = num;
    // Escreve cada algarismo, do maior para o menor, no led.
    while(div != 1){
        if(num < 32760){
            num = aux;
            aux = num%(div/10);
            num = (num%div - num%(div/10))/(div/10);
            div = div/10;
        } else div = 1;

        switch(num){
            case 0: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(6,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(13,3);
                    acendeLed(16,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;

            case 1: acendeLed(2,3);
                    acendeLed(7,3);
                    acendeLed(12,3);
                    acendeLed(16,3);
                    acendeLed(17,3);
                    acendeLed(22,3);
                    break;

            case 2: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(6,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;

            case 3: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;

            case 4: acendeLed(1,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(16,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(23,3);    
                    break;

            case 5: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(16,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;

            case 6: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(6,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(16,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;

            case 7: acendeLed(1,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;

            case 8: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(6,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(16,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;  
                    
            case 9: acendeLed(1,3);
                    acendeLed(2,3);
                    acendeLed(3,3);
                    acendeLed(8,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(16,3);
                    acendeLed(18,3);
                    acendeLed(21,3);
                    acendeLed(22,3);
                    acendeLed(23,3);
                    break;  

            case 32760: acendeLed(6,3);
                    acendeLed(8,3);
                    acendeLed(12,3);
                    acendeLed(16,3);
                    acendeLed(18,3);
                    break;

            case 32761: acendeLed(4,3);
                    acendeLed(6,3);
                    acendeLed(12,3);
                    acendeLed(18,3);
                    acendeLed(20,3);
                    break;

            case 32762: acendeLed(7,3);
                    acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    acendeLed(17,3);
                    break;

            case 32763: acendeLed(11,3);
                    acendeLed(12,3);
                    acendeLed(13,3);
                    break;

            default: break;

        }
        
        npWrite();
        npClear();
        sleep_ms(500);
        npWrite();
        sleep_ms(500);

    }
}

/**
 * @brief Gera um número aleatório positivo.
 */
int32_t geraAbs(){
    uint32_t random = get_rand_32();
    int32_t aux = abs(random);
    return aux;
}

/**
 * @brief Gera uma conta aleatória para o jogo de matemática.
 */
int16_t geraConta(uint8_t fase_atual){

    uint16_t op = geraAbs()%4; // Operação

    uint16_t p2; // Operante    
    uint16_t p1; // Operado

    uint16_t max_int;

        // Divisão:
    if(op + 32760 == 32761){

        if(fase_atual <= 180) max_int = fase_atual + 9; 
        else max_int = 180;

        p2 = 1 + geraAbs()%(max_int - 1); // Impede que o divisor seja 0.
        p1 = geraAbs()%(max_int);
        p1 = p1*p2; // Garante divisão inteira.

        // Subtração:
    } else if(op + 32760 == 32763){

        if(fase_atual*2 + 9 < 32760) max_int = fase_atual*2 + 9; 
        else max_int = 32759;

        p2 = geraAbs()%(max_int);
    
        do{ // Impede resultado negativo.
            p1 = geraAbs()%(max_int);
        } while(p1 < p2);

        // Multiplicação:
    } else if(op + 32760 == 32760){
        if(fase_atual + 9 <= 180) max_int = fase_atual + 9;
        else max_int = 180;

        p2 = geraAbs()%(max_int);
        p1 = geraAbs()%(max_int);

        // Soma:
    } else{
        if(fase_atual*4 + 18 < 32760) max_int = fase_atual*2 + 9;
        else max_int = 32759;

        p2 = geraAbs()%(max_int);
        p1 = geraAbs()%(max_int);

    }

    alfabetoNum(p1);
    sleep_ms(1000);
    npClear();

    alfabetoNum(op + 32760);
    sleep_ms(1000);
    npClear();

    alfabetoNum(p2);
    npClear();
    sleep_ms(1000);
    npWrite();

    switch(op + 32760){
        case 32760: return p1*p2;
        case 32761: return p1/p2;
        case 32762: return p1+p2;
        case 32763: return p1-p2;
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
bool btnARepeatLogic(struct repeating_timer *t){
    static absolute_time_t click_time_A = 0;
    static int count = 0;
    printf("entra %d\n", count);
    count++;
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
 * @brief Verifica se o jogador venceu uma rodada de lógica.
 */
bool verifyVictoryLogic(){
    if(cor_led_8 == cor_led_8_verify
    && cor_led_9 == cor_led_9_verify
    && cor_led_0 == cor_led_0_verify
    && cor_led_1 == cor_led_1_verify) return true;
    else return false;
}

/**
 * @brief Verifica se o jogador venceu uma rodada de matemática.
 */
bool verifyVictoryMath(){
    printf("%d\n", 10000*alg[0] + 1000*alg[1] + 100*alg[2] + 10*alg[3] + alg[4]);
    if(10000*alg[0] + 1000*alg[1] + 100*alg[2] + 10*alg[3] + alg[4] == resultado_correto) return true;
    else return false;
}

/**
 * @brief Rotina de tratamento do timer de controle do botão A.
 */
bool btnARepeatMath(struct repeating_timer *t){
    static absolute_time_t click_time_A = 0;
    if(!gpio_get(BUTTON_A) && absolute_time_diff_us(click_time_A, get_absolute_time()) > 200000){
        click_time_A = get_absolute_time();
        which_digit++;
        if(which_digit > 4){
            which_digit = 0;
            vitoria = verifyVictoryMath();
            new_fase = true;
        }
    }

    return true;
}

/**
 * @brief Rotina de tratamento do timer de controle do botão B.
 */
bool btnBRepeatMath(struct repeating_timer *t){
    static absolute_time_t click_time_B = 0;
    
    if(!gpio_get(BUTTON_B) && absolute_time_diff_us(click_time_B, get_absolute_time()) > 200000){
        click_time_B = get_absolute_time();
        if(which_digit > 0) which_digit--;
    }

    return true;
}

/**
 * @brief Rotina de tratamento do timer de controle do botão B.
 */
bool btnBRepeatLogic(struct repeating_timer *t){
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
            default: break; 
        }
    }

    if(!gpio_get(22) && absolute_time_diff_us(click_time_JS, get_absolute_time()) > 200000){
        click_time_JS = get_absolute_time();
        new_fase = true;
        vitoria = verifyVictoryLogic();
    }

    return true;
}

/**
 * @brief Função auxiliar da função drawMap, acende um conjunto de leds com cores determinadas.
 * @param fst primeiro led
 * @param scd segundo led
 * @param trd terceiro led
 * @param fth quarto led
 * @param led_up_left cor do led de cima à esquerda no primeiro quadro
 * @param led_up_right cor do led de cima à direita no primeiro quadro
 * @param led_bottom_right cor do led de baixo à direita no primeiro quadro
 * @param led_bottom_left cor do led de baixo à esquerda no primeiro quadro
 */
void acendeConjunto(uint8_t fst, uint8_t scd, uint8_t trd, uint8_t fth, uint8_t led_up_left, uint8_t led_up_right, uint8_t led_bottom_right, uint8_t led_bottom_left){
    acendeLed(fst, led_up_left);
    acendeLed(scd, led_up_right);
    acendeLed(trd, led_bottom_right);
    acendeLed(fth, led_bottom_left);
}

/**
 * @brief Gera os valores das cores corretas para vitória do usuário.
 * @param color8 cor correta para led 8
 * @param color9 cor correta para led 9
 * @param color0 cor correta para led 0
 * @param color1 cor correta para led 1
 */
void criaVerify(uint8_t color8, uint8_t color9, uint8_t color0, uint8_t color1){
    cor_led_8_verify = color8;
    cor_led_9_verify = color9;
    cor_led_0_verify = color0;
    cor_led_1_verify = color1;
}

/**
 * @brief Gera uma cor aleatória (0 a 3, 0 - apagado, 1 - azul, 2 - verde, 3 - vermelho).
 * @return inteiro de oito bits representando a cor gerada
 */
uint8_t geraCorRandom(){
    uint8_t ret = geraAbs()%4;
    return ret;
}

/**
 * @brief Gera um conjunto de cores aleatórias (usado na função de determinação de mapa)
 * @param led0 primeiro armazenador de cor aleatória
 * @param led1 segundo armazenador de cor aleatória
 * @param led2 terceiro armazenador de cor aleatória
 * @param led3 quarto armazenador de cor aleatória
 */
void geraConjuntoCor(uint8_t *led0, uint8_t *led1,uint8_t *led2, uint8_t *led3){
    *led0 = geraCorRandom();
    *led1 = geraCorRandom();
    *led2 = geraCorRandom();
    *led3 = geraCorRandom();
}

/**
 * @brief Determina e desenha o mapa de uma nova fase do jogo de lógica pura (aleatoriamente, seguindo parâmetros lógicos)
 */
void determinaMap(){

    // Definição dos armazenadores de cores dos leds do primeiro quadro;
    uint8_t led_up_left1;
    uint8_t led_up_right1;
    uint8_t led_bottom_right1;
    uint8_t led_bottom_left1;
    
    // Definição dos armazenadores de cores dos leds do terceiro quadro;
    uint8_t led_up_left3;
    uint8_t led_up_right3;
    uint8_t led_bottom_right3;
    uint8_t led_bottom_left3;

    // Determina aleatoriamente a lógica baseada no primeiro quadro para criar o segundo:
    uint8_t alt2 = geraAbs()%4; // fator aleatório para geração do segundo quadro com base no primeiro,
    // (0 - 90 graus no horario, 1 - no antihorario, 2 - 180, 3 - mudança de cores).
    
    // Determina cores dos quatro leds do primeiro quadro aleatoriamente:
    if(alt2 <= 1){ // Caso seja rotação de 90 graus.
        do{ // Impede que sejam todos iguais no caso de rotacionar, pois nesse caso não é possível concluir nada.
            geraConjuntoCor(&led_up_left1, &led_up_right1, 
            &led_bottom_right1, &led_bottom_left1); // Gera as cores aleatórias.
        } while(led_bottom_left1 == led_up_right1 && led_up_left1 == led_bottom_right1);
    }
    else if(alt2 <= 2){
        do{ // Impede que sejam todos iguais no caso de rotacionar, pois nesse caso não é possível concluir nada.
            geraConjuntoCor(&led_up_left1, &led_up_right1, 
            &led_bottom_right1, &led_bottom_left1); // Gera as cores aleatórias.
        } while(led_up_left1 == led_up_right1 && led_up_left1 == led_bottom_left1 && led_up_left1 == led_bottom_right1);
    } else{
        geraConjuntoCor(&led_up_left1, &led_up_right1, 
        &led_bottom_right1, &led_bottom_left1); // Gera as cores aleatórias.
    }

    // Acende leds do primeiro quadro:
    acendeConjunto(24,23,16,15,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);

    // Terceiro quadro:

    // Verifica se não haverá mudança de cores:
    if(alt2 != 3){
        // Determina cores dos quatro leds do terceiro quadro aleatoriamente:
        geraConjuntoCor(&led_up_left3, &led_up_right3, 
        &led_bottom_right3, &led_bottom_left3); // Gera as cores aleatórias.

    } else{ // Caso haja mudança de cores:
        uint8_t color = -1; // Armazena uma cor possível para um led do terceiro quadro.
        uint8_t leds_trd[4];
        for(uint8_t i = 0; i < 4; i++){ // para cada led do terceiro quadro:
            do{
                color = geraAbs()%4;
                leds_trd[i] = color;
            } while(color != led_up_left1 && color != led_up_right1 && color != led_bottom_right1 && color != led_bottom_left1);
            // ^ os leds do terceiro quadro só podem ter cores que estiverem em algum dos leds do primeiro!
        }

        // Efetiva cores dos quatro leds do terceiro quadro caso ocorra mudança de cor do primeiro para o terceiro quadro:
        led_up_left3 = leds_trd[0];
        led_up_right3 = leds_trd[1];
        led_bottom_right3 = leds_trd[2];
        led_bottom_left3 = leds_trd[3];
    }

    // Acende leds do terceiro quadro:
    acendeConjunto(5,6,3,4,led_up_left3,led_up_right3,led_bottom_right3,led_bottom_left3);

    switch(alt2){

        // Rotação sentido horário 90 graus:
        case 0:
                acendeConjunto(20,19,18,21,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);
                criaVerify(led_bottom_left3, led_up_left3, led_up_right3, led_bottom_right3);
                break;

        // Rotação sentido antihorário 90 graus:
        case 1:
                acendeConjunto(18,21,20,19,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);
                criaVerify(led_up_right3, led_bottom_right3, led_bottom_left3, led_up_left3);
                break;

        // Rotação 180 graus:
        case 2:
                acendeConjunto(19,18,21,20,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);
                criaVerify(led_bottom_right3, led_bottom_left3, led_up_left3, led_up_right3);
                break;
        
        // Troca cores:
        case 3:
                // Determina novas cores:
                uint8_t veri_8, veri_9, veri_0, veri_1;

                uint8_t new_col[4]; // Armazena cores corretas para conversão (cor índice -> cor elemento, de 0 a 3)
                new_col[0] = geraCorRandom();
                new_col[1] = geraCorRandom();
                new_col[2] = geraCorRandom();
                new_col[3] = geraCorRandom();

                //Determina cada cor correta por posição:
                for(uint8_t i = 0; i < 4; i++){
                    if(led_up_left1 == i){
                        acendeLed(21, new_col[i]);
                    }
                    if(led_up_right1 == i){
                        acendeLed(20, new_col[i]);
                    }
                    if(led_bottom_right1 == i){
                        acendeLed(19, new_col[i]);
                    }                
                    if(led_bottom_left1 == i){
                        acendeLed(18, new_col[i]);
                    }
                    if(led_up_left3 == i){
                        veri_8 = new_col[i];
                    }
                    if(led_up_right3 == i){
                        veri_9 = new_col[i];
                    }
                    if(led_bottom_right3 == i){
                        veri_0 = new_col[i];
                    }
                    if(led_bottom_left3 == i){
                        veri_1 = new_col[i];
                    }
                }

                criaVerify(veri_8, veri_9, veri_0, veri_1);
                break;

        default: break;
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
    organizeStrings("   fase atual   ", "                ", fase_str, ssd, frame_area);

    //-------------------------------------------------------------------------

    determinaMap();

    // Reseta parte manipulável do mapa:
    acendeConjunto(0, 1, 8, 9, 0, 0, 0, 0);

    acendeLed(posicao_atual, cor_atual); // Acende o cursor novamente

    cor_led_0 = 0;
    cor_led_1 = 0;
    cor_led_8 = 0;
    cor_led_9 = 0;
}

/**
 * @brief Printa mensagens do início do jogo.
 * @param ssd dados do display
 * @param frame_area area do display
 */
void mensagensInicio(uint8_t *ssd, struct render_area frame_area){

    // Mensagem de welcoming:
    organizeStrings(" Bem vindo ao   ", "                ", "     TGame      ", ssd, frame_area);

    // Som de welcoming:
    play_tone(BUZZER_PIN_A, 300, 200);
    sleep_ms(100);
    play_tone(BUZZER_PIN_A, 300, 200);
    sleep_ms(1000);

    // Escolha do jogo:
    organizeStrings(" Aperte A para  ", "  TLogic ou B   ", "  para TMath    ", ssd, frame_area);

    while(gpio_get(BUTTON_A) && gpio_get(BUTTON_B));
    if(!gpio_get(BUTTON_A)){
        game = true;
    } else if(!gpio_get(BUTTON_B)){
        game = false;
    }


    sleep_ms(500);
    play_tone(BUZZER_PIN_A, 300, 200);

    // Mensagem introdutória:
    organizeStrings("  Pressione A   ", "para prosseguir ", "  a cada etapa  ", ssd, frame_area);

    // Aguarda pressionamento do botão A.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);
    play_tone(BUZZER_PIN_A, 300, 200);

    // Jogo de lógica pura:
    if(game){ 
        
        // Mensagem tutorial 1:
        organizeStrings(" O jogador deve ", "    repetir     ", "  a logica dos  ", ssd, frame_area);

        // Aguarda pressionamento do botão A.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);
        play_tone(BUZZER_PIN_A, 300, 200);

        // Mensagem tutorial 2:
        organizeStrings("quadros de cima ", "  nos quadros   ", "   de baixo     ", ssd, frame_area);

        // Aguarda pressionamento do botão A.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);
        play_tone(BUZZER_PIN_A, 300, 200);

        // Mensagem tutorial 3:
        organizeStrings("  A troca cor   ", "                ", "  B define cor  ", ssd, frame_area);

        // Aguarda pressionamento do botão A.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);
        play_tone(BUZZER_PIN_A, 300, 200);

        // Mensagem tutorial 4:
        organizeStrings("JS move cursor  ", "e pressionado   ", "confirma cores  ", ssd, frame_area);

        // Aguarda pressionamento do botão A para começar jogo.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);

    }

    // Jogo de lógica matemática:
    else{
        // Mensagem tutorial 1:
        organizeStrings(" O jogador deve ", "  executar os   ", "    calculos    ", ssd, frame_area);

        // Aguarda pressionamento do botão A.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);
        play_tone(BUZZER_PIN_A, 300, 200);

        // Mensagem tutorial 2:
        organizeStrings("  A confirma    ", " um algarismo   ", " B desconfirma  ", ssd, frame_area);

        // Aguarda pressionamento do botão A.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);
        play_tone(BUZZER_PIN_A, 300, 200);

        // Mensagem tutorial 3:
        organizeStrings("  JS aumenta e  ", "   diminui um   ", "   algarismo    ", ssd, frame_area);

        // Aguarda pressionamento do botão A para começar jogo.
        while(gpio_get(BUTTON_A));
        sleep_ms(500);
    }

    // Som de início de Jogo:
    play_tone(BUZZER_PIN_B, 450, 200);
    sleep_ms(100);
    play_tone(BUZZER_PIN_B, 450, 200);
    sleep_ms(1000);
}

/**
 * @brief Desenha um x na matriz de led (usado na derrota).
 */
void desenhaX(){

    // Seta leds pertencentes ao X:
    acendeConjunto(0, 6, 8, 12, 3, 3, 3, 3);
    acendeConjunto(16, 24, 4, 18, 3, 3, 3, 3);
    npSetLED(20, 200, 0, 0);

    // Apaga demais leds:
    for(uint8_t i = 0; i < 25; i++){
        if(i != 0 && i != 6 && i != 8 && i != 12 && i != 16 && i != 24 && i != 4 && i != 18 && i != 20)
        npSetLED(i, 0, 0, 0);
    }
}

/**
 * @brief Reinicia o jogo desde as mensagens iniciais.
 * @param fase_atual fase do jogo. É resetada nesta função
 * @param ssd dados do display
 * @param frame_area area do display
 */
void restartFromScratch(uint8_t *fase_atual, uint8_t *ssd, struct render_area frame_area){
    
    //Desenha X indicando derrota:
    desenhaX();
    npWrite();

    // Escreve mensagem de derrota no display:
    organizeStrings("RESPOSTA ERRADA ", "                ", "TENTE NOVAMENTE ", ssd, frame_area);
    
    // Toca som de derrota:
    play_tone(BUZZER_PIN_B, 420, 100);
    play_tone(BUZZER_PIN_B, 410, 100);
    play_tone(BUZZER_PIN_B, 400, 100);
    play_tone(BUZZER_PIN_B, 390, 400);

    sleep_ms(500);

    // Reseta o mapa:
    for(uint8_t i = 0; i < 25; i++){
        npSetLED(i, 0, 0, 0);
    }
    npWrite();

    mensagensInicio(ssd, frame_area);
    *fase_atual = 0;
    new_fase = true;
    vitoria = true;
    alg[0] = 0;
    alg[1] = 0;
    alg[2] = 0;
    alg[3] = 0;
    alg[4] = 0;
}

/**
 * @brief Apresenta ao jogador indicativos de vitória.
 * @param ssd dados do display
 * @param frame_area area do display
 */
void apresentaVitoria(uint8_t *ssd, struct render_area frame_area){
    // Escreve mensagem de vitoria no display:
    organizeStrings("   MUITO BEM    ", "    CORRETO     ", " CONTINUE ASSIM ", ssd, frame_area);

    // Toca som de vitória:
    play_tone(BUZZER_PIN_B, 400, 100);
    play_tone(BUZZER_PIN_B, 420, 100);
    play_tone(BUZZER_PIN_B, 450, 400);

    sleep_ms(500);
}

/**
 * @brief Inicializa IO, pwm para buzzers, matriz de led, botões e joystick.
 */
void generalInit(){

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

}

/** 
 * @brief Apresenta os valores atuais dos dígitos no display.
 * @param ssd dados do display
 * @param frame_area area do display
 * @param fase_atual fase atual do jogo
*/
void showDigitsOnDisplay(uint8_t *ssd, struct render_area frame_area, uint8_t fase_atual){
    char str[17];
    char str_fase[17];
    sprintf(str_fase, "    Fase %.2d    ", fase_atual);
    sprintf(str, "     %d%d%d%d%d     ", alg[0], alg[1], alg[2], alg[3], alg[4]);

    organizeStrings(str_fase, "                ", str, ssd, frame_area);
}

/**
 * @brief Altera o número apresentado no display baseado no movimento do joystick.
 * @param ssd dados do display
 * @param frame_area area do display
 * @param vry representa o movimento do joystick no eixo y (0-4095)
 * @param fase_atual fase atual do jogo
 */
void alterDisplayByJoystk(uint8_t *ssd, struct render_area frame_area, uint16_t vry, uint8_t fase_atual){
    if(vry > 2047 + 1000 && alg[which_digit] < 9){
        alg[which_digit] += 1;
    }if(vry < 2047 - 1000 && alg[which_digit] > 0){
        alg[which_digit] -= 1;
    }
    showDigitsOnDisplay(ssd, frame_area, fase_atual);
}

int main(){

    uint8_t fase_atual = 0; // Define a fase em que o jogador se encontra (Não tem relação com nível de dificuldade).

        // Inicializacao geral:
    
    generalInit();

    uint16_t vrx_value, vry_value; // Para armazenar valores dos eixos x e y do joystick.
    posicao_atual = 0; // Posição inicial do cursor na matriz de led,
    cor_atual = 1; // Inicia em azul.

    char fase_str[17]; // Armazena a fase atual em string.

    fst_logic = true;
    fst_math = true;

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

    mensagensInicio(ssd, frame_area); // Printa no display as mensagens de início.

    //-------------------------------------------------------------------------

    which_digit = 0;
    struct repeating_timer timer_A; // Timer para controle do botão A.
    struct repeating_timer timer_B; // Timer para controle do botão B.

    bool restarted_math = false; // Indica se houve derrota no jogo de matemática para previnir escrita indevida no display.
    while(true){

        joystick_read_axis(&vrx_value, &vry_value); // Lê valores do joystick (0-4095)

        // Jogo de lógica pura:
        if(game){
            bool restart = false; // Garante que o cursor não acende no reinício, para não conflitar com possível mudança de jogo.
            if(fst_logic){
                fst_logic = false;
                fst_math = true;

                npClear();

                // Acende cursor em azul:
                npSetLED(posicao_atual, 0, 0, 200);

                // Inicializa timers para verificação de botões:
                add_repeating_timer_ms(100, btnARepeatLogic, NULL, &timer_A); // Inicializa temporizador para controle do botão A.
                add_repeating_timer_ms(100, btnBRepeatLogic, NULL, &timer_B); // Inicializa temporizador para controle do botão B.
                sleep_ms(100);

            }
            if(new_fase){ // Verifica se está-se iniciando uma nova fase.
                sleep_ms(100);
                if(vitoria){ // Verifica se o jogador passou de fase.
                    fase_atual++;
                    if(fase_atual > 1) apresentaVitoria(ssd, frame_area);
                    inicioFase(fase_atual, ssd, frame_area); // Inicia nova fase.
                    vitoria = false;
                    new_fase = false;
                } else{
                    restartFromScratch(&fase_atual, ssd, frame_area);
                    npClear();
                }
            }
            posicao_atual = movCursor(posicao_atual, cor_atual, vrx_value, vry_value); // Determina nova posição do cursor.
        } 
        
        // Jogo de matemática:
        else{
            if(fst_math){
                fst_logic = true;
                fst_math = false;

                // Inicializa timers para verificação de botões:
                add_repeating_timer_ms(100, btnARepeatMath, NULL, &timer_A); // Inicializa temporizador para controle do botão A.
                add_repeating_timer_ms(100, btnBRepeatMath, NULL, &timer_B); // Inicializa temporizador para controle do botão B.

                //Limpa display:
                organizeStrings("                ", "                ", "                ", ssd, frame_area);
                sleep_ms(100);
            }
            if(new_fase){
                sleep_ms(100);
                if(vitoria){
                    fase_atual++;
                    if(fase_atual > 1) apresentaVitoria(ssd, frame_area);
                    play_tone(BUZZER_PIN_B, 400, 200); // Som de início de fase
                    vitoria = false;
                    new_fase = false;
                    resultado_correto = geraConta(fase_atual);
                    printf("resultado correto: %d", resultado_correto);
                    alg[0] = 0;
                    alg[1] = 0;
                    alg[2] = 0;
                    alg[3] = 0;
                    alg[4] = 0;
                    showDigitsOnDisplay(ssd, frame_area, fase_atual);
                } else{
                    restartFromScratch(&fase_atual, ssd, frame_area);
                    restarted_math = true;
                }
            }
            if(!restarted_math) alterDisplayByJoystk(ssd, frame_area, vry_value, fase_atual);
            else{
                restarted_math = false;
                organizeStrings("                ", "                ", "                ", ssd, frame_area); // Limpa o display.
            }
        }
        
        npWrite(); // Escreve os dados nos LEDs.
        sleep_us(100000);
    }
}