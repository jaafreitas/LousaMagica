import websockets.*;
import java.util.Queue;
import java.util.ArrayDeque;

WebsocketClient wsc;

public class Ponto {
  Boolean tilt = true;
  int X = 0;
  int Y = 0;
  int tam = 20;   // Tamanho
  int mat = 0;    // Matiz
  int sat = 100;  // Saturação
  int bri = 100;  // Brilho
  int opa = 100;  // Opacidade/Alpha
  public Ponto(String dado) {
    int[] valor = int(split(dado, ' '));
    tilt = valor[0] == 1;
    mat = valor[4];
    X = valor[1];
    Y = valor[2];  
    tam = valor[3];
    sat = valor[5];
    opa = valor[6];
  }
}

Queue<Ponto> pontos = new ArrayDeque(100);

void setup(){
  size(1024, 1024);
  background(0);
  // https://processing.org/reference/colorMode_.html
  colorMode(HSB, 360, 100, 100, 100);
  frameRate(60);
  noStroke();
  wsc = new WebsocketClient(this, "ws://lousa-magica.local:81");
}

void draw(){
  if (pontos.size() > 0) {
    Ponto ponto = pontos.remove();
    if (ponto.tilt) {
      background(0);  // limpa o canvas com preto
    }
    else {        
      // Note modo HSB no setup! (Matiz, Saturação, Brilho, Alfa)
      fill(ponto.mat, ponto.sat, ponto.bri, ponto.opa);
      ellipse(ponto.X, ponto.Y, ponto.tam, ponto.tam);
    }
  }
}

void webSocketEvent(String dado){
  Ponto ponto = new Ponto(dado);
  pontos.add(ponto);
}
