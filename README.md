Um brinquedo para desenhar com potenciômetros.

Este projeto é uma versão sem fio do projeto [Lousa mágica](https://villares.github.io/lousa-magica/) de
[Alexandre B A Villares](https://villares.github.io/) utilizando o microcontrolador ESP8266 ao invés do Arduino.

Para utilizá-lo, você só precisa conectar-se a rede WiFi chamada lousa-magica. 
Após a conexão acesse o endereço http://lousa-magica.local e comece a desenhar!

### Exemplos
![Transparência](imagens/01.png)
![Colorido](imagens/02.png)

### Lista de materiais
* 1 microcontrolador ESP8266 (NodeMCU ou WeMos)
* 5 potenciômetros lineares de 10k
* 5 knobs para potenciômetros de cores sortidas
* 1 conversor analógico digital ADS1115
* 1 sensor de inclinação
* kit de fios jumpers
* caixa para montagem

### Ferramentas
* Ferro de solda
* Cola quente
* Estilete

### Montagem
1. Comece pela a furação na caixa de montagem para o encaixe dos potenciômetros;
2. Após fixar os potenciômetros e os knobs, conecte todos os terminais positivos e negativos dos potenciômetros;
3. Fixe o ESP8266 na caixa de montagem deixando uma abertura externa para a conexão do cabo USB ao microcontrolador;
4. Fixe o conversor analógico digital na caixa de montagem e conecte os pinos SCL e SDA aos pinos D1 e D2 do ESP8266 respectivamente;
5. Conecte 4 potenciômetros ao conversor analógico digital e 1 potenciômetro a porta analógica A0 do ESP8266;
6. Fixe o sensor de inclinação na caixa de montagem e conecte o pino de sinal ao pino D0 do ESP8266
7. Faça a ligação dos terminais negativos e positivos dos potenciômetros, conversor analógico digital e sensor de inclinação ao ESP8266;

### Referências
* [ESP8266](https://github.com/esp8266/Arduino)
* [ADS1115](https://github.com/adafruit/Adafruit_ADS1X15)
* [Processing](https://processing.org/)
* [p5.js](https://p5js.org/)
* [WebSockets](https://github.com/Links2004/arduinoWebSockets)
* [ESP8266 file system](http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html)
