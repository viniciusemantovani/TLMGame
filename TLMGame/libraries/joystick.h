// criado para expandir https://github.com/BitDogLab/BitDogLab-C/blob/main/Joystick_led/joystick.c
// e usar como biblioteca.

#ifndef JOYSTICK_H
#define JOYSTICK_H

 /**
 * @brief Função para configurar o joystick (pinos de leitura e ADC) 
 */
void setup_joystick();

/**
 * @brief Função para ler os valores dos eixos do joystick (X e Y)
 */ 
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value);


#endif