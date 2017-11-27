#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <Wire.h>
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

uint64_t ultimaAmostra = 0;
uint64_t ultimaMedidaAmostragem = 0;
bool conectado = false;

int ciclosXY = 0;
int ciclosDemais = 0;


static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
<head>
<meta charset="UTF-8">

<style> body {padding: 0; margin: 0;} </style>
<script>

var tilt = true;
var X = 0;
var Y = 0;
var tam = 20;  // Tamanho
var sat = 255; // Saturação
var opa = 1;   // Opacidade/Alpha

function setup() {
  createCanvas(1024, 1024);
  background(0);
  colorMode(HSB);  // para usar HSB em vez de RGB!
  frameRate(30);
  noStroke();
  
  var connection = new WebSocket('ws://' + window.location.host + ':81/', ['arduino']);
  connection.onmessage = function (msg) {
    valor = msg.data.split(' ').map(Number);
    console.log(msg.data);
    tilt = valor[0] == 1;
    X = valor[1];
    Y = valor[2];
    tam = valor[3];
    sat = valor[4];
    opa = valor[5];
  };
}

function draw() {
  if (tilt) {
    background(0);  // limpa o canvas com preto
  }
  else {
    var F = frameCount; 
    // Note modo HSB no setup! (Matiz, Saturação, Brilho, Alfa)
    fill(F % 255, sat, 255, opa/255);
    ellipse(X, Y, tam, tam);
  }
}
</script>
<script language="javascript" type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/p5.js/0.5.16/p5.min.js"></script>
</head>
</html>
)rawliteral";

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

  ads.setGain(GAIN_TWOTHIRDS);
  ads.begin();

  Serial.print("Iniciando access point... ");
  Serial.println(WiFi.softAP(nome) ? "ok" : "erro!");

  Serial.print("Endereço IP do access point... ");
  Serial.println(WiFi.softAPIP());  
  
  Serial.print("Conectando a ");
  Serial.print(ssid);
  WiFi.begin(ssid, senha);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" ok");

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
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  MDNS.addService("ws", "tcp", websocket_porta);
  Serial.printf("WebService disponível em ws://%s.local:%d ou ws://%s:%d\n",
                nome, websocket_porta, WiFi.localIP().toString().c_str(), websocket_porta);

  server.on("/", HTTP_GET, []() {
    Serial.println("server: /");
    server.send(200, "text/html", INDEX_HTML);
  });
  server.begin();
  MDNS.addService("http", "tcp", webserver_porta);
  Serial.printf("Serviço HTTP disponível em http://%s.local:%d ou http://%s:%d\n",
                nome, webserver_porta, WiFi.localIP().toString().c_str(), webserver_porta);

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
  server.handleClient();

  if (conectado) {
    uint64_t agora = millis();

    uint16_t X = readADC(0, 1023);
    uint16_t Y = readADC(3, 1023);
    
    if (agora - ultimaAmostra > intervaloAmostra) {
      ultimaAmostra = agora;

      tilt = digitalRead(tilt_pin);
      tam = readADC(2, 100);
      opa = readADC(1, 255);
      sat = map(analogRead(0), 0, 973, 0, 255);
      ciclosDemais++;
    }

    String dado = String(tilt) + " " +
                  String(X) + " " + String(Y) + " " +
                  String(tam) + " " + String(sat) + " " + String(opa);
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

