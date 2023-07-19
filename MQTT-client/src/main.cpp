#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/adc.h>
#include <driver/gpio.h>

#define SAMPLES_PERIOD 250
#define SAMPLES_NUMBER 16

TaskHandle_t xHandle_http_task = NULL;
TaskHandle_t xHandle_adc_task = NULL;
TaskHandle_t xHandle_mqtt_sender_task = NULL;

QueueHandle_t adcQueue;
WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = 2;
const int queueSize = 4;
long lastMsg = 0;
static int wifiConnected = 0;
static int blinkMode = 0;
const char* mqtt_server = "public.mqtthq.com";

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client-carlosjmarq")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("carlosjmarq/esp32");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  printf("Message arrived on topic: %s", topic);
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  printf(". Message: %s \n", messageTemp);

  if (String(topic) == "carlosjmarq/esp32") {
    printf("Changing blink mode \n");
    blinkMode += 1;
    if (blinkMode == 3) {
      blinkMode = 0;
    }    
  }
}

void blinkLed(void *pvParameters) {
  pinMode(ledPin, OUTPUT);
  while (true) {
    int *wifiConnected = (int *)pvParameters;
    TickType_t delayTime;
    if (*wifiConnected) {
      if (blinkMode == 0) {
        delayTime = pdMS_TO_TICKS(250);
      } else if (blinkMode == 1) {
        delayTime = pdMS_TO_TICKS(175);
      } else if (blinkMode == 2) {
        delayTime = pdMS_TO_TICKS(100);
      }
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
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void sendMQTTmessage(void *pvParameters) {
  while (true) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    long now = millis();
    uint32_t data;
    if (now - lastMsg > 10000 && uxQueueMessagesWaiting(adcQueue) > 0) {
      lastMsg = now;
      xQueueReceive(adcQueue, &data, portMAX_DELAY);
      printf("data a enviar por MQTT: %d \n", data); //Se imprime valor obtenido
      char tempString[8];
      String str;
      str = String(data);
      str.toCharArray(tempString, 5);
      


      printf("MQTT tempString: %d \n", tempString); //Se imprime valor obtenido
      client.publish("carlosjmarq/voltage-raw", tempString);
    }
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


void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(blinkLed, "Led blinking", configMINIMAL_STACK_SIZE, &wifiConnected, 5, &xHandle_http_task, 0);

  initWiFi(&wifiConnected);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  adcQueue = xQueueCreate(10, sizeof(uint32_t));

  while (adcQueue == NULL) {
    vTaskDelay(pdMS_TO_TICKS(200));
    Serial.println("esperando adc Queue");
  }


  xTaskCreatePinnedToCore(readAdcTask, "Adc Reader", configMINIMAL_STACK_SIZE * 8, NULL, 3, &xHandle_adc_task, 1);
  xTaskCreatePinnedToCore(sendMQTTmessage, "MQTT Sender", configMINIMAL_STACK_SIZE * 8, NULL, 3, &xHandle_mqtt_sender_task, 1);

}

void loop() {
}

