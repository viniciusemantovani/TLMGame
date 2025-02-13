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
 */
uint8_t geraCorRandom(){
    uint32_t random = get_rand_32();
    int32_t aux = abs(random);
    uint8_t ret = aux%4;
    return ret;
}

/**
 * @brief Determina e desenha o mapa de uma nova fase (aleatoriamente, seguindo parâmetros lógicos)
 */
void determinaMap(){

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
    uint32_t random = get_rand_32();
    uint32_t aux = abs(random);
    uint8_t alt2 = aux%4; // fator aleatório para geração do segundo quadro com base no primeiro. (0 - 90 graus no horario, 1 - no antihorario, 2 - 180, 3 - mudança de cores).
    
    // Determina cores dos quatro leds do primeiro quadro aleatoriamente:

    if(alt2 <= 1){ // Caso seja rotação de 90 graus.
        do{ // Impede que sejam todos iguais no caso de rotacionar, pois nesse caso não é possível concluir nada.
            led_up_left1 = geraCorRandom();
            led_up_right1 = geraCorRandom();
            led_bottom_right1 = geraCorRandom();
            led_bottom_left1 = geraCorRandom();
        } while(led_bottom_left1 == led_up_right1 && led_up_left1 == led_bottom_right1);
    }
    else if(alt2 <= 2){
        do{ // Impede que sejam todos iguais no caso de rotacionar, pois nesse caso não é possível concluir nada.
            led_up_left1 = geraCorRandom();
            led_up_right1 = geraCorRandom();
            led_bottom_right1 = geraCorRandom();
            led_bottom_left1 = geraCorRandom();
        } while(led_up_left1 == led_up_right1 && led_up_left1 == led_bottom_left1 && led_up_left1 == led_bottom_right1);
    } else{
        led_up_left1 = geraCorRandom();
        led_up_right1 = geraCorRandom();
        led_bottom_right1 = geraCorRandom();
        led_bottom_left1 = geraCorRandom();
    }

    // Acende leds do primeiro quadro:
    acendeConjunto(24,23,16,15,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);

    // Terceiro quadro:

    // Verifica se não haverá mudança de cores:
    if(alt2 != 3){
        // Determina cores dos quatro leds do terceiro quadro aleatoriamente:
        led_up_left3 = geraCorRandom();
        led_up_right3 = geraCorRandom();
        led_bottom_right3 = geraCorRandom();
        led_bottom_left3 = geraCorRandom();
    } else{
        uint8_t color = -1; // Armazena uma cor possível para um led do terceiro quadro.
        uint8_t leds_trd[4];
        for(uint8_t i = 0; i < 4; i++){ // para cada led do terceiro quadro:
            do{
                random = get_rand_32();
                aux = abs(random);
                color = aux%4;
                leds_trd[i] = color;
            } while(color != led_up_left1 && color != led_up_right1 && color != led_bottom_right1 && color != led_bottom_left1);
            // ^ os leds do terceiro quadro só podem ter cores que estiverem em algum dos leds do primeiro!
        }

        // Determina cores dos quatro leds do terceiro quadro aleatoriamente:
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

                printf("entrou no 0\n");

                acendeConjunto(20,19,18,21,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);
                criaVerify(led_bottom_left3, led_up_left3, led_up_right3, led_bottom_right3);
                break;

        // Rotação sentido antihorário 90 graus:
        case 1:
                printf("entrou no 1\n");

                acendeConjunto(18,21,20,19,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);
                criaVerify(led_up_right3, led_bottom_right3, led_bottom_left3, led_up_left3);
                break;

        // Rotação 180 graus:
        case 2:
                printf("entrou no 2\n");
                acendeConjunto(19,18,21,20,led_up_left1,led_up_right1,led_bottom_right1,led_bottom_left1);
                criaVerify(led_bottom_right3, led_bottom_left3, led_up_left3, led_up_right3);
                break;
        
        // Troca cores:
        case 3:
                // Determina novas cores:
                printf("entrou no 3\n");
                uint8_t veri_8, veri_9, veri_0, veri_1;

                uint8_t new_col[4]; // Armazena cores corretas para conversão (cor índice -> cor elemento, de 0 a 3)
                new_col[0] = geraCorRandom();
                new_col[1] = geraCorRandom();
                new_col[2] = geraCorRandom();
                new_col[3] = geraCorRandom();
                printf("out funct 0 %d\n", new_col[0]);
                printf("out funct 1 %d\n", new_col[1]);
                printf("out funct 2 %d\n", new_col[2]);
                printf("out funct 3 %d\n", new_col[3]);

                //Determina cada cor correta por posição:
                for(uint8_t i = 0; i < 4; i++){
                    if(led_up_left1 == i){
                        acendeLed(21, new_col[i]);
                        printf("acende 21 = %d\n", new_col[i]);
                    }
                    if(led_up_right1 == i){
                        printf("acende 20 = %d\n", new_col[i]);
                        acendeLed(20, new_col[i]);
                    }
                    if(led_bottom_right1 == i){
                        printf("acende 19 = %d\n", new_col[i]);
                        acendeLed(19, new_col[i]);
                    }                
                    if(led_bottom_left1 == i){
                        printf("acende 18 = %d\n", new_col[i]);
                        acendeLed(18, new_col[i]);
                    }
                    if(led_up_left3 == i){
                        veri_8 = new_col[i];
                        printf("veri 8 = %d\n", new_col[i]);
                    }
                    if(led_up_right3 == i){
                        veri_9 = new_col[i];
                        printf("veri 9 = %d\n", new_col[i]);
                    }
                    if(led_bottom_right3 == i){
                        printf("veri 0 = %d\n", new_col[i]);
                        veri_0 = new_col[i];
                    }
                    if(led_bottom_left3 == i){
                        printf("veri 1 = %d\n", new_col[i]);
                        veri_1 = new_col[i];
                    }
                }

                criaVerify(veri_8, veri_9, veri_0, veri_1);
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

    determinaMap();

    // Reseta parte manipulável do mapa:
    acendeLed(0, 0);
    acendeLed(1, 0);
    acendeLed(8, 0);
    acendeLed(9, 0);
    acendeLed(posicao_atual, cor_atual);

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
    char *welcome[] = {
        " Bem vindo ao   ",
        "                ",
        "    TLogic!     "};

    writeString(welcome, ssd, frame_area); // Escreve a mensagem de welcoming no display

    // Som de welcoming:
    play_tone(BUZZER_PIN_A, 300, 200);
    sleep_ms(100);
    play_tone(BUZZER_PIN_A, 300, 200);
    sleep_ms(1000);

    // Mensagem introdutória:
    char *begin[] = {
        "  Pressione A   ",
        "para prosseguir ",
        "  a cada etapa  ",
        "                "};

    writeString(begin, ssd, frame_area); // Escreve a mensagem introdutória no display

    // Aguarda pressionamento do botão A.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);
    play_tone(BUZZER_PIN_A, 300, 200);


        // Mensagem tutorial 1:
    char *tut1[] = {
        " O jogador deve ",
        "    repetir     ",
        "  a logica dos  ",
        "                "};

    writeString(tut1, ssd, frame_area); // Escreve a mensagem no display

    // Aguarda pressionamento do botão A.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);
    play_tone(BUZZER_PIN_A, 300, 200);

    // Mensagem tutorial 2:
    char *tut2[] = {
        " quadros decima ",
        "  nos quadros   ",
        "    debaixo     ",
        "                "};

    writeString(tut2, ssd, frame_area); // Escreve a mensagem no display

    // Aguarda pressionamento do botão A.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);
    play_tone(BUZZER_PIN_A, 300, 200);

    // Mensagem tutorial 2:
    char *tut3[] = {
        "  A troca cor   ",
        "  B define cor  ",
        "  JS confirma   ",
        "                "};

    writeString(tut3, ssd, frame_area); // Escreve a mensagem introdutória no display

    // Aguarda pressionamento do botão A para começar jogo.
    while(gpio_get(BUTTON_A));
    sleep_ms(500);
}

/**
 * @brief Desenha um x na matriz de led (usado na derrota).
 */
void desenhaX(){

    // Seta leds pertencentes ao X:
    npSetLED(0, 200, 0, 0);
    npSetLED(6, 200, 0, 0);
    npSetLED(8, 200, 0, 0);
    npSetLED(12, 200, 0, 0);
    npSetLED(16, 200, 0, 0);
    npSetLED(24, 200, 0, 0);
    npSetLED(4, 200, 0, 0);
    npSetLED(18, 200, 0, 0);
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
    char *derrota[] = {
        "RESPOSTA ERRADA ",
        "                ",
        "TENTE NOVAMENTE ",
        "                "};

    writeString(derrota, ssd, frame_area);
    
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

}

void apresentaVitoria(uint8_t *ssd, struct render_area frame_area){
    // Escreve mensagem de vitoria no display:
    char *derrota[] = {
        "   MUITO BEM    ",
        "    CORRETO     ",
        " CONTINUE ASSIM ",
        "                "};

    writeString(derrota, ssd, frame_area);

    // Toca som de vitória:
    play_tone(BUZZER_PIN_B, 400, 100);
    play_tone(BUZZER_PIN_B, 420, 100);
    play_tone(BUZZER_PIN_B, 450, 400);

    sleep_ms(500);
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

    mensagensInicio(ssd, frame_area); // Printa no display as mensagens de início.

    //-------------------------------------------------------------------------

        // Configurações de início de jogo:

    // Som de início de Jogo:
    play_tone(BUZZER_PIN_B, 450, 200);
    sleep_ms(100);
    play_tone(BUZZER_PIN_B, 450, 200);
    sleep_ms(1000);

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
                if(fase_atual > 1) apresentaVitoria(ssd, frame_area);
                inicioFase(fase_atual, ssd, frame_area); // Inicia nova fase.
                vitoria = false;
                new_fase = false;
            } else{
                restartFromScratch(&fase_atual, ssd, frame_area);
            }
        }
        
        joystick_read_axis(&vrx_value, &vry_value); // Lê valores do joystick (0-4095)
        posicao_atual = movCursor(posicao_atual, cor_atual, vrx_value, vry_value);
        npWrite(); // Escreve os dados nos LEDs.
        sleep_us(100000);
    }
}

