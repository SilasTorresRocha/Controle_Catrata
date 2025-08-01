#include <Arduino.h>

// Definindo o pino do LED
#define LED_PIN 15

// Task que vai controlar o piscar do LED
void blinkTask(void *pvParameters) {
  (void) pvParameters;
  pinMode(LED_PIN, OUTPUT);

  while (true) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));  // Espera 500ms
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));  // Espera 500ms
  }
}

void setup() {
  // Criação da task Blink com prioridade baixa
  xTaskCreate(
    blinkTask,       // Função da task
    "BlinkTask",     // Nome da task
    1024,            // Tamanho da stack
    NULL,            // Parâmetro passado
    1,               // Prioridade
    NULL             // Handle da task (não usado aqui)
  );
}

void loop() {
  // Nada a fazer aqui – tudo está sendo feito pela task
}
