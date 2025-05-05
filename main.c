#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/ws2812.h"
#include "ws2812.pio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define LED_RED_PIN 13
#define LED_GREEN_PIN 11

#define WS2812_PIN 7

#define BUZZER_PIN 10

#define BUTTON_A 5
#define BUTTON_B 6
#define DEBOUNCE_TIME 200000        // Tempo para debounce em ms
static uint32_t last_time_A = 0;    // Tempo da última interrupção do botão A
static uint32_t last_time_B = 0;    // Tempo da última interrupção do botão B

#define TIME 10                      // Tempo para cada sinal em segundos

bool night_mode = false;
bool button_a_pressed = false;

void vLedsTask() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

    gpio_init(BUZZER_PIN); 
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    while (true) {
        if (night_mode) {
            // MODO NOTURNO
            gpio_put(LED_GREEN_PIN, 1);
            gpio_put(LED_RED_PIN, 1);
            for (int i = 0; i < TIME-2; i++) {
                if (!night_mode) break;
                //gpio_put(BUZZER_PIN, true);
                vTaskDelay(pdMS_TO_TICKS(1000));
                //gpio_put(BUZZER_PIN, false);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            gpio_put(LED_GREEN_PIN, false);
            gpio_put(LED_RED_PIN, false);
        } else {
            // Sinal verde
            gpio_put(LED_GREEN_PIN, true);
            for (int i = 0; i < TIME; i++) {
                //gpio_put(BUZZER_PIN, true);
                vTaskDelay(pdMS_TO_TICKS(500));
                //gpio_put(BUZZER_PIN, false);
                vTaskDelay(pdMS_TO_TICKS(500));
                if (night_mode) break;
            }
            gpio_put(LED_GREEN_PIN, false);

            if (night_mode) continue;

            // Sinal amarelo
            gpio_put(LED_GREEN_PIN, 1);
            gpio_put(LED_RED_PIN, 1);
            for (int i = 0; i < TIME*2; i++) {
                //gpio_put(BUZZER_PIN, true);
                vTaskDelay(pdMS_TO_TICKS(250));
                //gpio_put(BUZZER_PIN, false);
                vTaskDelay(pdMS_TO_TICKS(250));
                if (night_mode) break;
            }
            gpio_put(LED_GREEN_PIN, false);
            gpio_put(LED_RED_PIN, false);

            if (night_mode) continue;

            // Sinal vermelho
            gpio_put(LED_RED_PIN, true);
            for (int i = 0; i < TIME/2; i++){
                //gpio_put(BUZZER_PIN, true);
                vTaskDelay(pdMS_TO_TICKS(500));
                //gpio_put(BUZZER_PIN, false);
                vTaskDelay(pdMS_TO_TICKS(1500));
                if (night_mode) break;
            }
            gpio_put(LED_RED_PIN, false);
        }   
    }
}

void vMatrixTask() {
    // Inicializa o PIO para controlar a matriz de LEDs (WS2812)
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, WS2812_PIN);
    clear_matrix(pio, sm);

    int count = 0;

    sleep_ms(100);
    while (true) {
        if (night_mode) {
            count = 0;
            set_pattern(pio, sm, 10, "amarelo");
        } else {
            set_pattern(pio, sm, count, "cinza");
            count++;
            if (count >= 10) {
                count = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vDisplayTask() {
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

    bool cor = true;
    while (true) {
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 20, 123, 20, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "   SEMAFORO", 8, 6);
        ssd1306_rect(&ssd, 95, 15, 100, 20, cor, !cor);    // Desenha um retângulo
        ssd1306_send_data(&ssd);                           // Atualiza o display
        sleep_ms(735);
    }
}

void vButtonTask() {
    while (true) {
        if (button_a_pressed) {
            taskENTER_CRITICAL();
            night_mode = !night_mode;
            taskEXIT_CRITICAL();
            button_a_pressed = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // para evitar polling excessivo
    }
}

void buttons_irq(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A) {
        if (current_time - last_time_A > DEBOUNCE_TIME) {
            button_a_pressed = !button_a_pressed;
            last_time_A = current_time;
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

void setup_button(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL, true, &buttons_irq);
}

int main() {
    setup_button(BUTTON_A);
    setup_button(BUTTON_B);

    stdio_init_all();

    xTaskCreate(vLedsTask, "Leds Task", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatrixTask, "Matrix Task", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display Task", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vButtonTask, "Button Task", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
