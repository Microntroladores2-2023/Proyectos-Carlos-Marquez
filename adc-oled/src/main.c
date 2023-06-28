#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "esp_adc_cal.h"
// #include "Adafruit_SSD1306/Adafruit_SSD1306.h"

static esp_adc_cal_characteristics_t adc2_chars;
static int led_on = 0;
static int adc_display_value = 0;

#define LED_PIN GPIO_NUM_2
#define SDA_PIN 21
#define SCL_PIN 22
#define BLINK_PERIOD 500
#define SAMPLES_PERIOD 250
#define SAMPLES_NUMBER 16

#define I2C_ADDRESS 0x3C   // Dirección I2C de la pantalla OLED
#define I2C_MASTER_SCL_IO 22   // GPIO22 es el pin del reloj SCL
#define I2C_MASTER_SDA_IO 21   // GPIO21 es el pin de datos SDA
#define I2C_MASTER_NUM I2C_NUM_0   // I2C número 0 del ESP32
#define I2C_MASTER_FREQ_HZ 100000   // Frecuencia de operación del I2C


void ledBlink(void *pvParams) {
    for (;;) {
        int *ON = (int *)pvParams;
        *ON = !(*ON);
        printf("el valor de On es: %d\n", *ON);
        gpio_set_level(LED_PIN, *ON);
        vTaskDelay(BLINK_PERIOD/ portTICK_PERIOD_MS);
    }
}

void readADC(void *pvParams) {
    //Se fija la atenuacion y voltaje de referencia
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc2_chars);

    adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_DB_11);
    // adc2_set_width(ADC_WIDTH_BIT_12);

    int *adc_display_value = (int *)pvParams;

    int muestras[SAMPLES_NUMBER], promedio;
    int i = 0;
    uint32_t adc_raw_value;

    adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &adc_raw_value); //Se toma medicion de valor en canal 4
    int adc_value = (int)adc_raw_value;
    
    for (int j = 0; j < SAMPLES_NUMBER; j++)
        muestras[j] = adc_value;
    
    printf("Voy a entrar al loop infinito \n");

    for (;;) 
    {
        adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &adc_raw_value);
        adc_value = (int)adc_raw_value;

        printf("ADC Value raw: %d \n", adc_value); //Se imprime valor obtenido
        muestras[i++] = adc_value;
        promedio = 0;

        for (int j = 0; j < SAMPLES_NUMBER; j++)
            promedio = promedio + muestras[j]; //Se calcula promedio con muestras obtenidas

        printf("ADC Value promedio: %d \n", promedio / SAMPLES_NUMBER);
        *adc_display_value = promedio;

        if (i >= SAMPLES_NUMBER)
            i = 0;
    }
}

void I2cWrite(void *pvPramas) {
    #define I2C_ADDRESS 0x68    // Dirección del dispositivo I2C

    uint8_t reg_addr = 0x00;    // Dirección del registro que deseas leer
    uint8_t data = 0;           // Variable donde se almacenará el valor leído

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    printf("Valor leído del registro 0x%02x: 0x%02x\n", reg_addr, data);
}

void app_main() {
    // led config
    esp_rom_gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    // i2c config
    
    
    // // Configurar el controlador I2C
    // i2c_config_t conf;
    // conf.mode = I2C_MODE_MASTER;
    // conf.sda_io_num = I2C_MASTER_SDA_IO;
    // conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    // conf.scl_io_num = I2C_MASTER_SCL_IO;
    // conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    // conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    // i2c_param_config(I2C_MASTER_NUM, &conf);
    // i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // // Inicializar la pantalla OLED
    // ssd1306_init();

    // // Borrar la pantalla
    // ssd1306_clear_screen();

    // // Escribir texto en la pantalla
    // char* message = "Hola, mundo!";
    // ssd1306_draw_string(0, 0, message);

    // // Actualizar la pantalla
    // ssd1306_refresh_gram();





    // invoking tasks
    xTaskCreatePinnedToCore(ledBlink, "Led Blink", 1024 * 2, &led_on, 5, NULL, 0);
    xTaskCreatePinnedToCore(readADC, "TareaADC", 1024 * 10, &adc_display_value, 1, NULL, 0);
    // xTaskCreatePinnedToCore(I2cWrite, "Tarea I2C", 1024 * 10, &adc_display_value, 1, NULL, 0);
}