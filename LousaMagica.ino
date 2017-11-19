#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

// Configurações
static const char ssid[] = "seu-ssid";
static const char senha[] = "sua-senha";
static const char nome[] = "lousa-magica";
static const int porta = 81;
static const unsigned int intervaloAmostra = 50;  // Em milisegundos

WebSocketsServer webSocket = WebSocketsServer(porta);
uint64_t ultimaAmostra = 0;
bool conectado = false;

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
      break;
  }
}

void setup() {
  Serial.begin(74880);
  Serial.println();
  Serial.println("Iniciando Lousa mágica");

  Serial.print("Conectando a ");
  Serial.print(ssid);
  WiFi.begin(ssid, senha);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" Ok");

  if (MDNS.begin(nome)) {
    Serial.println("MDNS iniciado");
  }
  else {
    Serial.println("MDNS falhou");
  }

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  MDNS.addService("ws", "tcp", 81);

  Serial.printf("WebService disponível em ws://%s.local:%d ou ws://%s:%d\n",
                nome, porta, WiFi.localIP().toString().c_str(), porta);
  Serial.println("Aguardando conexões");
}

void loop() {
  webSocket.loop();
  if (conectado) {
    uint64_t agora = millis();
    if (agora - ultimaAmostra > intervaloAmostra) {
      ultimaAmostra = agora;
      String dado = String(analogRead(0));
      webSocket.broadcastTXT(dado);
    }
  }
}

