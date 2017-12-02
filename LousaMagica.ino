#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <Wire.h>
#include <FS.h>
#include <Adafruit_ADS1015.h>

// Conexões no NodeMCU/WeMos
static const uint8_t tilt_pin = D0;
// ADS1115 CLK: D1
// ADS1115 SDA: D2
// Potenciômetro Saturação: A0

// Configurações
static const char ssid[] = "seu-ssid";
static const char senha[] = "sua-senha";
static const char nome[] = "lousa-magica";
static const int websocket_porta = 81;
static const int webserver_porta = 80;
static const unsigned int intervaloAmostra = 50;  // Em milisegundos
unsigned int tentativasConexao = 0;
static const unsigned int maxTentativasConexao = 15;

DNSServer dnsServer;
WebSocketsServer webSocket = WebSocketsServer(websocket_porta);
ESP8266WebServer server(webserver_porta);

// Pode ser necessário recalibrar conforme tensão máxima
// aplicada nas portas analógicas.
// Tensão máxima suportada: +/- 6.144V / resolução 1 bit: 0.1875mV
static const int ads_max_valor = 3.30 / 0.1875 * 1000;
Adafruit_ADS1115 ads;

bool tilt = false;
uint16_t tam;
uint16_t sat;
int opa;
uint16_t mat = 0;

uint64_t ultimaAmostra = 0;
uint64_t ultimaMedidaAmostragem = 0;
bool conectado = false;

int ciclosXY = 0;
int ciclosDemais = 0;

String getContentType(String filename){
  if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void handleFileRead(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    // Slow performance of FSBrowser on Chrome and Firefox
    // https://github.com/esp8266/Arduino/issues/1027
    size_t sent = server.streamFile(file, contentType);
    file.close();
  }
  else {
    server.send(404, "text/plain", "FileNotFound");
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] desconectado\n", num);
      conectado = false;
      break;
    case WStype_CONNECTED:
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] %s conectado\n", num, WiFi.localIP().toString().c_str());
      conectado = true;
      ultimaMedidaAmostragem = millis();
      break;
  }
}

void setup() {
  Serial.begin(74880);
  Serial.println();
  Serial.println();
  Serial.println("Iniciando Lousa Mágica");

  pinMode(tilt_pin, INPUT);

  Serial.print("Abrindo sistema de arquivos... ");
  SPIFFS.begin();
  Serial.println("ok");

  ads.setGain(GAIN_TWOTHIRDS);
  ads.begin();

  Serial.print("Iniciando access point... ");
  Serial.println(WiFi.softAP(nome) ? "ok" : "erro!");

  Serial.print("Endereço IP do access point... ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(53, nome + String(".local"), WiFi.softAPIP());

  Serial.print("Conectando a ");
  Serial.print(ssid);
  WiFi.begin(ssid, senha);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    tentativasConexao++;
    if (tentativasConexao > maxTentativasConexao) {
      WiFi.mode(WIFI_AP);
      break;
    }
  }
  if (tentativasConexao <= maxTentativasConexao) {
    Serial.println(" ok");
  }
  else {
    Serial.println(" erro!");
  }

  if (MDNS.begin(nome)) {
    Serial.println("MDNS iniciado");
  }
  else {
    Serial.println("MDNS falhou");
  }

  if (WEBSOCKETS_NETWORK_TYPE != NETWORK_ESP8266_ASYNC) {
    Serial.println("ATENÇÃO: Configure a biblioteca WebSockets para utilizar NETWORK_ESP8266_ASYNC");
    Serial.println("ou a usabilidade poderá ficar comprometida!");
  }
  webSocket.onEvent(webSocketEvent);
  webSocket.begin();
  MDNS.addService("ws", "tcp", websocket_porta);
  Serial.printf("WebService disponível em ws://%s.local:%d\n", nome, websocket_porta);
                
  // Para não precisar ficar mapeando cada um dos arquivos.
  server.onNotFound([](){
    Serial.println(server.uri());
    handleFileRead(server.uri());
  });  
  server.begin();
  MDNS.addService("http", "tcp", webserver_porta);
  Serial.printf("Serviço HTTP disponível em http://%s.local:%d\n", nome, webserver_porta);

  Serial.println("Aguardando conexões");
}

uint16_t readADC(uint8_t channel, uint16_t max_valor) {
  // Cuidado! constraint é uma macro.
  int16_t value = ads.readADC_SingleEnded(channel);
  return map(constrain(value, 0, ads_max_valor), 0, ads_max_valor, 0, max_valor);
}

void loop() { 
#if (WEBSOCKETS_NETWORK_TYPE != NETWORK_ESP8266_ASYNC)
  webSocket.loop();
#endif
  dnsServer.processNextRequest();
  server.handleClient();

  if (conectado) {
    uint64_t agora = millis();

    mat++;
    if (mat > 360) {
      mat = 0;
    }
    
    uint16_t X = readADC(0, 1023);
    uint16_t Y = readADC(3, 1023);
    
    if (agora - ultimaAmostra > intervaloAmostra) {
      ultimaAmostra = agora;

      tilt = digitalRead(tilt_pin);
      tam = readADC(2, 100);
      opa = readADC(1, 100);
      sat = map(analogRead(0), 0, 973, 0, 100);
      ciclosDemais++;
    }

    String dado = String(tilt) + " " +
                  String(X) + " " + String(Y) + " " + String(tam) + " " +
                  String(mat) + " " + String(sat) + " " + String(opa);
    webSocket.broadcastTXT(dado);

    if (agora - ultimaMedidaAmostragem > 1000) {
      ultimaMedidaAmostragem = agora;
      Serial.printf("Amostragem XY/demais: %d/%d\n", ciclosXY, ciclosDemais);
      ciclosXY = 0;
      ciclosDemais = 0;
    }
    ciclosXY++;
  }
}

