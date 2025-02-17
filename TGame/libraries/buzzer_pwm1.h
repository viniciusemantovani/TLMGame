// Extending content from https://github.com/BitDogLab/BitDogLab-C/blob/main/buzzer_pwm1/buzzer_pwm1.c
// to use as a library.

#ifndef BUZZER_PWM1_H
#define BUZZER_PWM1_H

// Configuração do pino do buzzer
#define BUZZER_PIN_A 21
#define BUZZER_PIN_B 10

/**
 * @brief Inicializa o PWM no pino do buzzer
 * @param pin pino do buzzer
*/
void pwm_init_buzzer(uint pin);

/** 
 * @brief Toca uma nota com a frequência e duração especificadas
 * @param pin pino do buzzer
 * @param frequency frequencia da nota
 * @param duration_ms duracao da nota
*/
void play_tone(uint pin, uint frequency, uint duration_ms);

#endif