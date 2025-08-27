// Inclui a biblioteca necessária para comunicação via Bluetooth Serial no ESP32
#include "BluetoothSerial.h"

// Verifica se o Bluetooth está habilitado no ESP32
// Se não estiver, interrompe a compilação com uma mensagem de erro
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth não está habilitado! Por favor verifique as configurações.
#endif

// Define os pinos conectados aos LEDs RGB
#define LED_R 25  // LED vermelho no pino 25
#define LED_G 26  // LED verde no pino 26
#define LED_B 27  // LED azul no pino 27

// Cria um objeto SerialBT para controlar a comunicação Bluetooth Serial
BluetoothSerial SerialBT;

void setup(){
  // Inicia a comunicação serial com o computador (via USB) na velocidade de 115200 bps
  Serial.begin(115200);

  // Inicia a comunicação Bluetooth com o nome "ESP32_TECDES_05"
  SerialBT.begin("ESP32_SW");
  Serial.println("Bluetooth iniciado. Aguarde emparelhamento...");

  // Configura os pinos dos LEDs como saída
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Garante que todos os LEDs comecem apagados
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
}

void loop(){
  // Se houver dados recebidos pela porta serial USB, encaminha-os para o Bluetooth
  if (Serial.available()) {
    SerialBT.write(Serial.read());  // Transfere o dado diretamente do Serial para o Bluetooth
  }

  // Verifica se há dados recebidos via Bluetooth
  if (SerialBT.available()) {
    String comando = "";  // Variável para armazenar o comando recebido

    // Enquanto houver dados no buffer Bluetooth, armazena cada caractere na variável `comando`
    while (SerialBT.available()) {
      char c = SerialBT.read();  // Lê um caractere
      comando += c;              // Adiciona ao comando
      delay(2);                  // Pequeno atraso para garantir leitura completa
    }

    comando.trim();  // Remove espaços em branco antes/depois do texto
    Serial.print("Comando recebido: ");
    Serial.println(comando);

    // Verifica se o comando tem exatamente dois caracteres (ex: R1, G0, B1)
    if (comando.length() == 2) {
      char led = comando.charAt(0);    // Primeiro caractere indica o LED (R, G ou B)
      char estado = comando.charAt(1); // Segundo caractere indica se liga (1) ou desliga (0)

      // Converte o caractere de estado para HIGH ou LOW
       int valor = (estado == '1') ? HIGH : LOW;

      // Verifica qual LED deve ser controlado e executa o comando
      if (led == 'R') {
        Serial.println(valor);  // Imprime o valor (1 ou 0)
        digitalWrite(LED_R, valor);  // Liga/desliga o LED vermelho
        Serial.println(valor == HIGH ? "LED VERMELHO ligado" : "LED VERMELHO desligado");
      }else if (led == 'G') {
        Serial.println(valor);
        digitalWrite(LED_G, valor);  // Liga/desliga o LED verde
        Serial.println(valor == HIGH ? "LED VERDE ligado" : "LED VERDE desligado");
      }else if (led == 'B') {
        Serial.println(valor);
        digitalWrite(LED_B, valor);  // Liga/desliga o LED azul
        Serial.println(valor == HIGH ? "LED AZUL ligado" : "LED AZUL desligado");
      }else {
        // Se o primeiro caractere não for R, G ou B
        Serial.println("Comando inválido! Use R, G ou B.");
      }
    }else {
      // Se o comando não tiver 2 caracteres, avisa o usuário
      Serial.println("Formato de comando inválido. Use por exemplo: R1, G0, B1");
    }
  }

  delay(20);

}