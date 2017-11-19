import websockets.*;

WebsocketClient wsc;

int X = 0;
int Y = 0;
int tam = 20;   // Tamanho
int sat = 255;  // Saturação
int opa = 255;  // Opacidade/Alpha
  
void setup(){
  size(1024, 1024);
  colorMode(HSB);  // para usar HSB em vez de RGB!
  frameRate(30);
  noStroke();
  background(0);
  wsc = new WebsocketClient(this, "ws://lousa-magica.local:81");
}

void draw(){
  float F = frameCount;
  // Note modo HSB no setup! (Matiz, Saturação, Brilho, Alfa)
  fill(F % 255, sat, 255, opa);
  ellipse(X, Y, tam, tam);
}

void webSocketEvent(String dado){
  int[] valor = int(split(dado, ' '));
  X = valor[0];
  Y = valor[1];
  tam = valor[2];
  sat = valor[3];
  opa = valor[4];
}