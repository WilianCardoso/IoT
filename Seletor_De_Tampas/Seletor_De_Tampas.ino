#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal.h>

// ----------- Configurações WiFi -----------
const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";

// ----------- Configuração do servidor -----------
WebServer server(80);

// ----------- Configuração dos pinos -----------
#define RGB_R 25
#define RGB_G 26
#define RGB_B 27
#define PIN_BTN2 0
#define PIN_BTN3 4
#define PIN_BTN4 16
#define STEP 5   
#define DIR 22    
#define TRIG_PIN 13 
#define ECHO_PIN 12 

bool posicao = false;
int passos;
unsigned long ultimoPasso = 0;
const unsigned long intervaloPasso = 500; 

// ----------- Últimos valores salvos -----------
int lastPos = 0;
bool lastBtn2State = false;
bool lastBtn3State = false;
bool lastBtn4State = false;

// ----------- Endpoints -----------
void handleRoot() {
  server.send(200, "text/plain", "API REST ESP32 Ativa!");
}

void handleLastResult() {
  String jsonResponse = "{";
  jsonResponse += "\"status\": \"POS ATINGIDA\",";
  jsonResponse += "\"pos_solicitada\": " + String(lastPos) + ",";
  jsonResponse += "\"pos_atual\": " + String(lastPos) + ",";
  jsonResponse += "\"fim_curso1\": " + String(lastBtn2State ? 1 : 0) + ",";
  jsonResponse += "\"fim_curso2\": " + String(lastBtn3State ? 1 : 0) + ",";
  jsonResponse += "\"fim_curso3\": " + String(lastBtn4State ? 1 : 0);
  jsonResponse += "}";
  server.send(200, "application/json", jsonResponse);
}

void handleMovePos() {
  if (server.hasArg("pos")) {
    String posStr = server.arg("pos");
    int pos = posStr.toInt();

    Serial.print("Recebi posicao: ");
    Serial.println(pos);

    int pinSelecionado;
    if (pos == 1) pinSelecionado = PIN_BTN2;
    else if (pos == 2) pinSelecionado = PIN_BTN3;
    else if (pos == 3) pinSelecionado = PIN_BTN4;
    else {
      server.send(400, "text/plain", "Posicao invalida");
      return;
    }

    server.send(200, "text/plain", "Posicao recebida: " + posStr);

    passos = 0;
    ultimoPasso = 0;

    bool botaoPressionado = false;
    bool executando = true;
    
    digitalWrite(DIR, LOW); 
    
    while (executando) {
      if (digitalRead(pinSelecionado) == HIGH) {
        if (!botaoPressionado) {
          botaoPressionado = true;
          digitalWrite(RGB_B, LOW);
          digitalWrite(RGB_R, LOW);
          digitalWrite(RGB_G, HIGH);
        }

        unsigned long tempoAtual = millis();
        if (tempoAtual - ultimoPasso >= intervaloPasso) {
          digitalWrite(STEP, HIGH);  
          delayMicroseconds(100);
          digitalWrite(STEP, LOW);   
          delayMicroseconds(100); 
          passos++;
          ultimoPasso = tempoAtual;
          Serial.print("Passo: "); Serial.println(passos);
        }
      } else {
        if (botaoPressionado) {
          botaoPressionado = false;
          executando = false;
        }
      }
      delay(10);
      server.handleClient(); // mantém servidor ativo
    }

    // Atualiza último resultado
    lastPos = pos;
    lastBtn2State = (digitalRead(PIN_BTN2) == LOW);
    lastBtn3State = (digitalRead(PIN_BTN3) == LOW);
    lastBtn4State = (digitalRead(PIN_BTN4) == LOW);

    digitalWrite(RGB_B, HIGH);
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    Serial.println("Chave pressionada");
  } else {
    server.send(400, "text/plain", "Parametro nao enviado");
  }
}

// ----------- Setup -----------
void setup() {
  pinMode(STEP, OUTPUT);     
  pinMode(DIR, OUTPUT);      
  pinMode(TRIG_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT);  
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(PIN_BTN2, INPUT);
  pinMode(PIN_BTN3, INPUT);
  pinMode(PIN_BTN4, INPUT);
  Serial.begin(115200);

  // MOTOR SEMPRE ANTI-HORÁRIO
  digitalWrite(DIR, LOW);

  // Tenta conectar no WiFi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delayMicroseconds(500);
    Serial.print(".");
    digitalWrite(RGB_B, LOW);
    digitalWrite(RGB_R, HIGH);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_B, HIGH);
  } else {
    Serial.println("\nFalha ao conectar. Rodando em modo OFFLINE...");
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
  }

  // Endpoints
  server.on("/", handleRoot);
  server.on("/api/move_pos", HTTP_GET, handleMovePos);
  server.on("/api/move_pos", HTTP_POST, handleMovePos);
  server.on("/api/last_result", HTTP_GET, handleLastResult);

  server.begin();
  Serial.println("Servidor HTTP iniciado!");
}

// ----------- Loop -----------
void loop() {
  server.handleClient();
}
