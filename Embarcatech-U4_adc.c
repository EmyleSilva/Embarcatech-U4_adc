#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

#define BUTTON_A 5
#define BUTTON_B 6 //Usado para colocar a placa no modo de bootsel
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_B 22
#define R_LED_PIN 13
#define B_LED_PIN 12
#define G_LED_PIN 11
#define PWM_WRAP_VALUE 4096

uint32_t adc_x_value;
uint32_t adc_y_value;
uint red_slice_num;
uint blue_slice_num;


void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

void init_joystick()
{
    gpio_init(JOYSTICK_B);
    gpio_set_dir(JOYSTICK_B, GPIO_IN);
    gpio_pull_up(JOYSTICK_B);

    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

}

void init_buttons_leds()
{
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(R_LED_PIN);
    gpio_set_dir(R_LED_PIN, GPIO_OUT);

    gpio_init(G_LED_PIN);
    gpio_set_dir(G_LED_PIN, GPIO_OUT);

    gpio_init(B_LED_PIN);
    gpio_set_dir(B_LED_PIN, GPIO_OUT);
}

uint init_pwm_gpio(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, PWM_WRAP_VALUE);

    pwm_set_enabled(slice_num, true);

    return slice_num;
}

int main()
{
    stdio_init_all();
    init_joystick();
    init_buttons_leds();

    red_slice_num = init_pwm_gpio(R_LED_PIN);
    blue_slice_num = init_pwm_gpio(B_LED_PIN);
    
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        adc_select_input(0);
        adc_x_value = adc_read();

        adc_select_input(1);
        adc_y_value = adc_read();

        printf("X: %d | Y: %d \n", adc_x_value, adc_y_value);

        sleep_ms(200);
    }
}
