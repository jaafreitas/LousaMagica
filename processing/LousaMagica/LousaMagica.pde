import websockets.*;

WebsocketClient wsc;

Boolean tilt = true;
int X = 0;
int Y = 0;
int tam = 20;   // Tamanho
int sat = 255;  // Saturação
int opa = 255;  // Opacidade/Alpha
  
void setup(){
  size(1024, 1024);
  background(0);
  colorMode(HSB);  // para usar HSB em vez de RGB!
  frameRate(30);
  noStroke();
  wsc = new WebsocketClient(this, "ws://lousa-magica.local:81");
}

void draw(){
  if (tilt) {
    background(0);  // limpa o canvas com preto
  }
  else {
    float F = frameCount;
    // Note modo HSB no setup! (Matiz, Saturação, Brilho, Alfa)
    fill(F % 255, sat, 255, opa);
    ellipse(X, Y, tam, tam);
  }
}

void webSocketEvent(String dado){
  int[] valor = int(split(dado, ' '));
  tilt = valor[0] == 1;
  X = valor[1];
  Y = valor[2];
  tam = valor[3];
  sat = valor[4];
  opa = valor[5];
}