// ------------------- Bibliotecas -------------------
#include "BluetoothSerial.h"
#include <LiquidCrystal.h>

// ------------------- Verificações -------------------
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth não está habilitado! Verifique a configuração do ESP32.
#endif

// ------------------- Objetos -------------------
BluetoothSerial SerialBT;
LiquidCrystal lcd(32, 33, 23, 22, 21, 18);

// ------------------- Definições -------------------
#define TRIG_PIN 13
#define ECHO_PIN 35

#define RGB_R 25
#define RGB_G 26
#define RGB_B 27 

#define BTN_1 0   // Recipiente A
#define BTN_2 4   // Recipiente C
#define BTN_3 16  // Recipiente B
#define BTN_4 17  // Alternar modo

bool modoAutomatico = false;
int recipienteSelecionado = -1; // -1 = nenhum, 0 = A, 1 = B, 2 = C
String comando = "";

// ------------------- Funções -------------------
void mostrarTitulo() {
  lcd.clear();
  lcd.setCursor(3, 0);  
  lcd.print("Sensor de");
  lcd.setCursor(5, 1);  
  lcd.print("Pecas");
  delay(2000);
}

float lerDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW); 
  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

void atualizarRGB(int r, int g, int b) {
  digitalWrite(RGB_R, r);
  digitalWrite(RGB_G, g);
  digitalWrite(RGB_B, b);
}

void enviarBT(String msg) {
  SerialBT.println(msg);
  Serial.println(msg); // também no Serial Monitor
}

// Função que checa mudança de modo
void verificarMudancaModo() {
  // Botão físico
  if (digitalRead(BTN_4) == LOW) {
    modoAutomatico = !modoAutomatico;
    enviarBT(modoAutomatico ? "Modo Automatico" : "Modo Manual");
    delay(300); // debounce
  }

  // Comando Bluetooth
  if (SerialBT.available()) {
    String comando = SerialBT.readStringUntil('\n');
    comando.trim();
    if (comando.startsWith("M")) {  // qualquer M enviado
      modoAutomatico = !modoAutomatico;
      enviarBT(modoAutomatico ? "Modo Automatico" : "Modo Manual");
    }
  }
}
// ------------------- Modos -------------------
void modoAutomaticoFunc() {
  bluetoothComando();

  verificarMudancaModo();

  float distancia = lerDistancia();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: Auto");

  if (distancia > 20) {  
    atualizarRGB(LOW, LOW, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Sem objeto   ");
    enviarBT("Auto: Sem objeto");
  }
  else if (distancia >= 3 && distancia <= 7) {  
    atualizarRGB(HIGH, LOW, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Recipiente: A");
    enviarBT("Auto: Recipiente A");
  }
  else if (distancia >= 8 && distancia <= 12) { 
    atualizarRGB(LOW, HIGH, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Recipiente: B");
    enviarBT("Auto: Recipiente B");
  }
  else if (distancia >= 13 && distancia <= 17) { 
    atualizarRGB(LOW, LOW, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Recipiente: C");
    enviarBT("Auto: Recipiente C");
  }
  else {
    atualizarRGB(LOW, LOW, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Fora faixa   ");
    enviarBT("Auto: Fora da faixa");
  }
  
   //modoAutomatico = !modoAutomatico;
     // enviarBT(modoAutomatico ? "Modo Automatico" : "Modo Manual");
}

void bluetoothComando(){
   if (Serial.available()) {
    SerialBT.write(Serial.read());  // Transfere o dado diretamente do Serial para o Bluetooth
  }

  // Verifica se há dados recebidos via Bluetooth
  if (SerialBT.available()) {
    comando = "";  // Variável para armazenar o comando recebido

    // Enquanto houver dados no buffer Bluetooth, armazena cada caractere na variável `comando`
    while (SerialBT.available()) {
      char c = SerialBT.read();  // Lê um caractere
      comando += c;              // Adiciona ao comando
      delay(2);                  // Pequeno atraso para garantir leitura completa
    }

    comando.trim();  // Remove espaços em branco antes/depois do texto
    Serial.print("Comando recebido: ");
    Serial.println(comando);
  }
}

void modoManualFunc() {

  verificarMudancaModo();
  
  // Seleção pelos botões físicos
  if (digitalRead(BTN_1) == LOW) { recipienteSelecionado = 0; delay(200); }
  else if (digitalRead(BTN_2) == LOW) { recipienteSelecionado = 2; delay(200); }
  else if (digitalRead(BTN_3) == LOW) { recipienteSelecionado = 1; delay(200); }

bluetoothComando();

    if (comando.startsWith("R")) recipienteSelecionado = 0;   // Recipiente A
    else if (comando.startsWith("G")) recipienteSelecionado = 1; // Recipiente B
    else if (comando.startsWith("B")) recipienteSelecionado = 2; // Recipiente C
    else if (comando.startsWith("M")) {
      modoAutomatico = !modoAutomatico;
      enviarBT(modoAutomatico ? "Modo Automatico" : "Modo Manual");
    }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: Manual");

  switch (recipienteSelecionado) {
    case 0: // Vermelho - A
      atualizarRGB(HIGH, LOW, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Recipiente: A ");
      enviarBT("Manual: Recipiente A");
      break;

    case 1: // Verde - B
      atualizarRGB(LOW, HIGH, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Recipiente: B ");
      enviarBT("Manual: Recipiente B");
      break;

    case 2: // Azul - C
      atualizarRGB(LOW, LOW, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Recipiente: C ");
      enviarBT("Manual: Recipiente C");
      break;

    default:
      atualizarRGB(LOW, LOW, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Selec. botao  ");
      break;
  }
}

// ------------------- Setup -------------------
void setup() {
  lcd.begin(16, 2);
  Serial.begin(115200);
  SerialBT.begin("ESP32_SELETOR");  
  Serial.println("Bluetooth iniciado. Conecte-se a ESP32_SELETOR");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);

  mostrarTitulo();
}

// ------------------- Loop -------------------
void loop() {
  // Alternar modo com botão físico BTN_4
  if (digitalRead(BTN_4) == LOW) {
    modoAutomatico = !modoAutomatico;
    enviarBT(modoAutomatico ? "Modo Automatico" : "Modo Manual");
    delay(300); // debounce
  }

  if (modoAutomatico) {
    modoAutomaticoFunc();
  } else {
    modoManualFunc();
  }

  delay(150);
}