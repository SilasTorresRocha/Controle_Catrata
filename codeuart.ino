#include <SoftwareSerial.h>

// RX, TX — só  RX para ler do leitor
SoftwareSerial scannerSerial(4, 5); // 4 = RX, 5 = TX (não usado)

void setup() {
  Serial.begin(115200); // Para debug no PC
  scannerSerial.begin(9600); // Baud rate do GM861S

  Serial.println("Esperando leitura do scanner...");
}

void loop() {
  if (scannerSerial.available()) {
    String codigo = scannerSerial.readStringUntil('\n');
    Serial.print("Código lido: ");
    Serial.println(codigo);
  }
}