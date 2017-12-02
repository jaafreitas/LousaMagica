import websockets.*;

WebsocketClient wsc;

Boolean tilt = true;
int X = 0;
int Y = 0;
int tam = 20;   // Tamanho
int mat = 0;    // Matriz
int sat = 100;  // Saturação
int opa = 100;  // Opacidade/Alpha

void setup(){
  size(1024, 1024);
  background(0);
  // https://processing.org/reference/colorMode_.html
  colorMode(HSB, 360, 100, 100, 100);
  frameRate(30);
  noStroke();
  wsc = new WebsocketClient(this, "ws://lousa-magica.local:81");
}

void draw(){
  if (tilt) {
    background(0);  // limpa o canvas com preto
  }
  else {
    // Note modo HSB no setup! (Matiz, Saturação, Brilho, Alfa)
    fill(mat, sat, 100, opa);
    ellipse(X, Y, tam, tam);
  }
}

void webSocketEvent(String dado){
  int[] valor = int(split(dado, ' '));
  tilt = valor[0] == 1;
  X = valor[1];
  Y = valor[2];  
  tam = valor[3];
  mat = valor[4];
  sat = valor[5];
  opa = valor[6];
}