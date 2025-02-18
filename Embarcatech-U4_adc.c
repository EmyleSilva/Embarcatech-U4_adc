#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
/**
 * Definições dos pinos GPIOs de entradas/saidas 
 */
#define BUTTON_A 5 
#define BUTTON_B 6 //Usado para colocar a placa no modo de bootsel
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_B 22
#define R_LED_PIN 13
#define B_LED_PIN 12
#define G_LED_PIN 11

//Definições para o I2C
#define I2C_PORT i2c1
#define I2C_SDA 14 
#define I2C_SCL 15 
#define address 0x3C

//Valor de WRAP do PWM
#define PWM_WRAP_VALUE 4096
//Tempo de Debounce em ms
#define DEBOUNCE_TIME_MS 200

/**
 * Variáveis globais
 */
uint32_t adc_x_value; //Armazena o valor do eixo x do joystick
uint32_t adc_y_value; //Armazena o valor do eixo y do joystick
uint red_slice_num; //Armazena o numero de slice pwm do pino de led vermelho
uint blue_slice_num; //Armazena o numero de slice pwm do pino de led azul
uint32_t last_time = 0; 
uint8_t display_x; //Armazena a posição x do quadrado no display
uint8_t display_y; //Armazena a posição y do quadrado no display
bool pwm_active = true;
bool g_led_state = false;
ssd1306_t ssd;

/**
 * Função de callback para tratamento de acionamento de botões
 */
void gpio_irq_handler(uint gpio, uint32_t events)
{   
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if ((current_time - last_time) > DEBOUNCE_TIME_MS)
    {
        last_time = current_time;

        switch (gpio)
        {
        case BUTTON_A: //Ativa ou desativa os LEDS PWM
            pwm_active = !pwm_active;
            printf("Novo estado do PWM: %s\n", (pwm_active) ? "Ativado" : "Desativado");
            break;
        case BUTTON_B: //Coloca a placa de desenvolvimento no modo de bootsel
            reset_usb_boot(0, 0);
            break;
        case JOYSTICK_B: //Alterna o estado do LED verde e muda a borda no display 
            g_led_state = !g_led_state;
            gpio_put(G_LED_PIN, g_led_state);
            break;
        }
    }
    
}

/**
 * Inicializa o botão do joystick e o ADC dos eixos X e Y
 */
void init_joystick()
{
    gpio_init(JOYSTICK_B);
    gpio_set_dir(JOYSTICK_B, GPIO_IN);
    gpio_pull_up(JOYSTICK_B);

    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

}

/**
 * Inicializa os botões A e B e leds RGB
 */
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

/**
 * Inicializa os pinos pwm
 */
uint init_pwm_gpio(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, PWM_WRAP_VALUE);

    pwm_set_enabled(slice_num, true);

    return slice_num;
}

/**
 * Atualiza a intensidade do brilho do led de acordo com o valor lido do ADC 
 */
void update_led_brightness(uint gpio, uint32_t adc_value)
{
    uint duty_cycle;

    /**
     * Verifica a posição do joystick e calcula o valor do duty cycle para alterar a 
     * intensidade do brilho do led de acordo com a posição lida 
     **/ 
    if (adc_value > 1800 && adc_value < 2100)
    {
        duty_cycle = 0;
    }else if (adc_value >= 2100) {
        duty_cycle = (adc_value - 2048) * 2;
    }else {
        duty_cycle = (2048 - adc_value) * 2;
    }

    //Garante que o valor calculado esteja dentro do limite
    if (duty_cycle < 0) duty_cycle = 0;
    if (duty_cycle > 4095) duty_cycle = 4095;

    pwm_set_gpio_level(gpio, duty_cycle);
}

/**
 * Desenha uma borda diferente de acordo com o pressionamento do Botão do Joystick
 */
void draw_rectangle()
{
    if (!g_led_state)
    {
        ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);
    }else {
        ssd1306_rect(&ssd, 1, 1, 122, 60, true, false);
        ssd1306_rect(&ssd, 4, 4, 118, 54, true, false);
    }
}

/**
 * Atualiza a posição do quadrado no display de acordo com os valores
 * lidos pelo ADC
 */
void update_position_on_display()
{
    static int last_x = 128 / 2;
    static int last_y = 64 / 2;

    display_x = adc_x_value * (128-8) / 4095; //Calcula o novo valor da posição x
    display_y = adc_y_value * (64-8) / 4095; //Calcula o novo valor da posição y

    ssd1306_fill(&ssd, false); //Limpa a tela anterior
    draw_rectangle(); //Desenha a borda de acordo com o PushBotton do Joystick
    ssd1306_draw_quad(&ssd, display_x, display_y); //Desenha o quadrado na posição atual
    ssd1306_send_data(&ssd); //Envia os dados para o display

    last_x = display_x;
    last_y = display_y;
}

int main()
{
    //Realiza as inicializações
    stdio_init_all();
    init_joystick();
    init_buttons_leds();

    /**
     * Configuração do I2C
     */
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    /**
     * Configuração do display
     */
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, address, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    //Inicializa os pinos dos leds azul e vermelho como PWM
    red_slice_num = init_pwm_gpio(R_LED_PIN);
    blue_slice_num = init_pwm_gpio(B_LED_PIN);

    //Chamam a função de callback para tratamento de interrupções criadas pelos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        //Faz a leitura do eixo X
        adc_select_input(0);
        adc_x_value = adc_read();

        //Faz a leitura do eixo Y
        adc_select_input(1);
        adc_y_value = adc_read();

        if (pwm_active) //Verifica se os LEDs PWM estão ativados
        {
            update_led_brightness(R_LED_PIN, adc_x_value); //Ajusta o brilho do LED vermelho
            update_led_brightness(B_LED_PIN, adc_y_value); //Ajusta o brilho do LED azul
        }

        //Desenha o quadrado no display
        update_position_on_display();

        sleep_ms(10);

    }
}
