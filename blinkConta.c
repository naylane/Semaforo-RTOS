#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define LED_RED_PIN 13
#define LED_BLUE_PIN 12
#define LED_GREEN_PIN 11

#define BUTTON_A 5
#define BUTTON_B 6
#define DEBOUNCE_TIME 200000        // Tempo para debounce em ms
static uint32_t last_time_A = 0;    // Tempo da última interrupção do botão A
static uint32_t last_time_B = 0;    // Tempo da última interrupção do botão B

bool night_mode = false;

void vBlinkLed1Task() {
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_GREEN_PIN, true);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_put(LED_GREEN_PIN, false);
        vTaskDelay(pdMS_TO_TICKS(1223));
    }
}

void vBlinkLed2Task() {
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_BLUE_PIN, true);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_BLUE_PIN, false);
        vTaskDelay(pdMS_TO_TICKS(2224));
    }
}

void vDisplay3Task() {
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char str_y[5]; // Buffer para armazenar a string
    int contador = 0;
    bool cor = true;
    while (true) {
        sprintf(str_y, "%d", contador); // Converte em string
        contador++;                     // Incrementa o contador
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "  FreeRTOS", 10, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Contador  LEDs", 10, 41);    // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 40, 52);          // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display
        sleep_ms(735);
    }
}

void buttons_irq(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A) {
        if (current_time - last_time_A > DEBOUNCE_TIME) {
          night_mode = !night_mode;
          return;
        }
    } 
    else if (gpio == BUTTON_B) {
        if (current_time - last_time_B > DEBOUNCE_TIME) {
            reset_usb_boot(0, 0);
            last_time_B = current_time;
            return;
          }
    }
}

int main() {
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &buttons_irq);

    stdio_init_all();

    xTaskCreate(vBlinkLed1Task, "Blink Task Led1", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBlinkLed2Task, "Blink Task Led2", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplay3Task, "Cont Task Disp3", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
