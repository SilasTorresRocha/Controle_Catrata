#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <DHT.h>

// Rede AP
const char* ap_ssid = "catraca";
const char* ap_pass = "20uss911";
// Rede STA
const char* sta_ssid = "iPhone de Silas ";
const char* sta_pass = "20uss911";

// Pinos e sensor
#define DHTPIN 1  //  X do ESP01
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Servidor HTTP
ESP8266WebServer server(80);

// Variáveis de média
float sumTemp = 0, sumHum = 0;
int countRead = 0;
unsigned long lastMinute = 0;

// Inicializa SPIFFS e cria arquivo vazio se não existir
void initFile() {
  SPIFFS.begin();
  if (!SPIFFS.exists("/data.txt")) {
    File f = SPIFFS.open("/data.txt", "w");
    f.close();
  }
}

// Obtém timestamp formatado no formato dd/mm/yyyy hh:mm:ss
String timestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "NTP nao sincronizado";
  }
  // Buffer com tamanho seguro para "dd/mm/yyyy hh:mm:ss" + null
  char buf[25]; 
  strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buf);
}

// Grava média no arquivo
void saveAverage(float t, float h) {
  File f = SPIFFS.open("/data.txt", "a");
  if (f) {
    // Agora usa o novo formato de timestamp
    f.printf("%s;%.2f;%.2f\n", timestamp().c_str(), t, h);
    f.close();
  }
}

// Gera HTML da página principal
String webPage() {
  float avgT = countRead ? sumTemp / countRead : 0;
  float avgH = countRead ? sumHum / countRead : 0;
  String page = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Logger DHT11</title>";
  page += "<style>body{font-family: sans-serif; text-align: center;} h2,h3{color: #333;} button{padding: 10px; margin: 5px; cursor: pointer;}</style>";
  page += "</head><body>";
  page += "<h1>Logger DHT11</h1>";
  
  // --- NOVO: Exibe a data e hora atuais ---
  page += "<h3>Data e Hora do Servidor</h3>";
  page += "<p>" + timestamp() + "</p><hr>";

  page += "<h2>Média Atual (último minuto)</h2>";
  page += "<p>Temperatura: " + String(avgT, 2) + " °C</p>";
  page += "<p>Umidade: " + String(avgH, 2) + " %</p><hr>";
  
  page += "<h3>Controles</h3>";
  page += "<button onclick=\"location.href='/download'\">Download .txt</button>";
  page += "<button onclick=\"location.href='/erase'\" style=\"background-color: #f44336; color: white;\">Apagar Dados</button>";
  page += "</body></html>";
  return page;
}

// Handlers HTTP
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", webPage());
}

void handleDownload() {
  File f = SPIFFS.open("/data.txt", "r");
  if (!f) {
    server.send(200, "text/plain", "Nenhum dado para baixar.");
    return;
  }
  server.sendHeader("Content-Type", "text/plain");
  server.sendHeader("Content-Disposition", "attachment; filename=data.txt");
  server.streamFile(f, "text/plain");
  f.close();
}

void handleErase() {
  SPIFFS.remove("/data.txt");
  initFile(); // Recria o arquivo vazio
  server.send(200, "text/plain", "Dados apagados com sucesso! O arquivo foi recriado.");
}

void setup() {
  //Serial.begin(115200);
  dht.begin();
  initFile();

  // Configura WiFi AP+STA
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_pass);
  WiFi.begin(sta_ssid, sta_pass);

  //Serial.println("Conectando ao WiFi...");
  // Espera STA conectar (opcional, não bloqueante)
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) { // Aumentado para 15s
    delay(500);
    //Serial.print(".");
  }
  
  if(WiFi.status() == WL_CONNECTED){
    //Serial.println("\nWiFi conectado!");
    //Serial.print("IP: ");
    //Serial.println(WiFi.localIP());
  } else {
    //Serial.println("\nNao foi possivel conectar ao WiFi STA. O servidor ainda funciona no modo AP.");
  }


  // NTP (Horário de Brasília)
  // Timezone: UTC-3, sem horário de verão
  configTime(-3 * 3600, 0, "a.st1.ntp.br", "pool.ntp.org", "time.nist.gov");


  // Rotas HTTP
  server.on("/", handleRoot);
  server.on("/download", handleDownload);
  server.on("/erase", handleErase);
  server.begin();
  //Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();

  // Leitura a cada 5s
  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 5000) {
    lastRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      sumTemp += t;
      sumHum += h;
      countRead++;
    }
  }

  // A cada minuto protocola média e reseta
  if (millis() - lastMinute >= 60000) {
    lastMinute = millis();
    if (countRead > 0) {
      float avgT = sumTemp / countRead;
      float avgH = sumHum / countRead;
      saveAverage(avgT, avgH);
      //Serial.printf("Media salva: Temp=%.2f, Umid=%.2f\n", avgT, avgH);
    }
    // Reseta as variáveis para o próximo minuto
    sumTemp = 0;
    sumHum = 0;
    countRead = 0;
  }
}