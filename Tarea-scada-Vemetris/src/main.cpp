#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/adc.h>
#include <driver/gpio.h>
#include <cJSON.h>
#include "ArduinoJson-v6.21.2.h"

#define SAMPLES_PERIOD 250
#define SAMPLES_NUMBER 16

TaskHandle_t xHandle_http_task = NULL;
TaskHandle_t xHandle_adc_task = NULL;
TaskHandle_t xHandle_sender_task = NULL;
TaskHandle_t xHandle_wheather_task = NULL;
const int ledPin = 2;
const int queueSize = 4;
QueueHandle_t adcQueue;
QueueHandle_t wheatherQueue;
static int wifiConnected = 0;
String wheatherAPIUrl = "http://api.weatherapi.com/v1/current.json?key=e4b6b5f00cd64991ae412225232806&q=Caracas&aqi=yes";

typedef struct
{
  float feelsLike_c;
  float temperature_c;
  uint32_t humidity;
} WheatherInterface;


void blinkLed(void *pvParameters) {
  pinMode(ledPin, OUTPUT);
  while (true) {
    int *wifiConnected = (int *)pvParameters;
    TickType_t delayTime;
    if (*wifiConnected) {
      delayTime = pdMS_TO_TICKS(250);
    } else {
      delayTime = pdMS_TO_TICKS(600);
    }
    digitalWrite(ledPin, HIGH); // Encender el LED
    vTaskDelay(delayTime);
    digitalWrite(ledPin, LOW); // Apagar el LED
    vTaskDelay(delayTime);
  }
}

void readAdcTask(void *pvParameters) {
  gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);

  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  adc1_channel_t channel = ADC1_CHANNEL_0;

  adc_bits_width_t adcWidth = ADC_WIDTH_BIT_12;

  uint32_t muestras[SAMPLES_NUMBER], promedio;
  int i = 0;
  int adcValue = adc1_get_raw(ADC1_CHANNEL_0); // Leer el valor del ADC1 del canal 0

  for (int j = 0; j < SAMPLES_NUMBER; j++)
        muestras[j] = adcValue;

  while (true) {
    adcValue = adc1_get_raw(ADC1_CHANNEL_0); // Leer el valor del ADC1 del canal 0
    printf("ADC Value raw: %d \n", adcValue); //Se imprime valor obtenido
    muestras[i++] = adcValue;
    promedio = 0;
    for (int j = 0; j < SAMPLES_NUMBER; j++)
      promedio = promedio + muestras[j] / SAMPLES_NUMBER;
    xQueueSend(adcQueue, &promedio, portMAX_DELAY);
    if (i >= SAMPLES_NUMBER)
      i = 0;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void consultWeatherAPI(void *pvParameters) {
  HTTPClient client;
  client.begin(wheatherAPIUrl);
  int httpResponseCode = client.GET();
  if (httpResponseCode > 0) {
      String payload = client.getString(); // Obtener la respuesta de la API
      
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);

      // Verificar si hay errores de análisis
      if (error) {
        Serial.print("Error al analizar el JSON: ");
        Serial.println(error.c_str());
        return;
      }

      const char* location = doc["location"]["name"];



      Serial.println(location);
  } else {
    Serial.println("Error en la solicitud");
  }
  client.end();
  vTaskDelay(pdMS_TO_TICKS(10000));

}

void sendHttpData(void *pvParameters) {
  String ScadaVemetris = "http://137.184.178.17:21486/httpds?__device=adc_carlos";
  while (true) {
    uint32_t data;
    if (uxQueueMessagesWaiting(adcQueue) > 0) {
      xQueueReceive(adcQueue, &data, portMAX_DELAY);
      printf("en http data: %d \n", data); //Se imprime valor obtenido

      HTTPClient http;

      String queryString = ScadaVemetris + "&adcValue=" + data;

      http.begin(queryString); 
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        String payload = http.getString();
        // Serial.println(httpResponseCode);
        // Serial.println(payload);           
      }
      else
      {
        Serial.println("Error enviadno la trama");
      }
      http.end(); // Se libera el cliente
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void initWiFi(int *wifiConnected)
{
  const char *network = "Itadori";
  const char *password = "jujutsu2023";
  WiFi.begin(network, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Conectando a red wifi...");
    Serial.println(WiFi.status());
  }
  *wifiConnected = 1;
}

void consultApi(String url) {
  HTTPClient client;
  client.begin(url);
  int httpResponseCode = client.GET();
  if (httpResponseCode > 0) {
      String payload = client.getString(); // Obtener la respuesta de la API
      Serial.println(httpResponseCode);
      Serial.println(payload);
  } else {
    Serial.println("Error en la solicitud");
  }
  client.end();
}

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(blinkLed, "Led blinking", configMINIMAL_STACK_SIZE, &wifiConnected, 5, &xHandle_http_task, 0);

  initWiFi(&wifiConnected);

  adcQueue = xQueueCreate(10, sizeof(uint32_t));
  wheatherQueue = xQueueCreate(10, sizeof(WheatherInterface));

  while (adcQueue == NULL) {
    vTaskDelay(pdMS_TO_TICKS(200));
    Serial.println("esperando adc Queue");
  }

  xTaskCreatePinnedToCore(readAdcTask, "Adc Reader", configMINIMAL_STACK_SIZE * 8, NULL, 3, &xHandle_adc_task, 1);
  // xTaskCreatePinnedToCore(consultWeatherAPI, "wheather api consult", configMINIMAL_STACK_SIZE * 16, NULL, 3, &xHandle_wheather_task, 1);
  xTaskCreatePinnedToCore(sendHttpData, "http data sender", configMINIMAL_STACK_SIZE * 8, NULL, 1, &xHandle_sender_task, 0);

  // consultApi(serverName);
}

void loop() {
}

