#include <WiFi.h>
#include <WebServer.h>

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

// ----------- Variáveis globais -----------
volatile int lastPos = 0;
volatile int posSolicitada = 0;
volatile bool fimCurso1 = false, fimCurso2 = false, fimCurso3 = false;
volatile bool executarMotor = false;
TaskHandle_t motorTaskHandle;

// ----------- Função para LEDs -----------
void setLedEstado(const char* estado) {
  if (strcmp(estado, "INICIO") == 0) {       // Vermelho
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
  } 
  else if (strcmp(estado, "WIFI_CONECTADO") == 0) { // Azul
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
  } 
  else if (strcmp(estado, "MOVENDO") == 0) { // Verde
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
  }
}

// ----------- Task do Motor (Core 1) -----------
void motorTask(void *pvParameters) {
  int pinSelecionado;
  bool botaoPressionado = false;
  unsigned long passos = 0;
  const unsigned long intervaloPasso = 500; 
  unsigned long ultimoPasso = 0;

  for (;;) {
    if (executarMotor) {
      setLedEstado("MOVENDO"); // LED verde ao iniciar curso

      // Seleciona pino do botão de acordo com a posição solicitada
      if (posSolicitada == 1) pinSelecionado = PIN_BTN2;
      else if (posSolicitada == 2) pinSelecionado = PIN_BTN3;
      else if (posSolicitada == 3) pinSelecionado = PIN_BTN4;
      else {
        executarMotor = false; 
        continue;
      }

      digitalWrite(DIR, LOW); 
      botaoPressionado = false;
      passos = 0;
      ultimoPasso = 0;

      while (executarMotor) {
        if (digitalRead(pinSelecionado) == HIGH) {
          if (!botaoPressionado) botaoPressionado = true;

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
          if (botaoPressionado) executarMotor = false;
        }

        fimCurso1 = (digitalRead(PIN_BTN2) == LOW);
        fimCurso2 = (digitalRead(PIN_BTN3) == LOW);
        fimCurso3 = (digitalRead(PIN_BTN4) == LOW);

        if (!executarMotor) {
          if (fimCurso1) lastPos = 1;
          else if (fimCurso2) lastPos = 2;
          else if (fimCurso3) lastPos = 3;

          setLedEstado("WIFI_CONECTADO"); // volta para azul ao terminar curso
          Serial.println("Curso concluído!");
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ----------- Endpoints -----------
void handleRoot() {
  server.send(200, "text/plain", "API REST ESP32 Ativa!");
}

void handleMovePos() {
  if (server.hasArg("pos")) {
    posSolicitada = server.arg("pos").toInt();
    executarMotor = true;
    Serial.print("Recebi posicao: "); Serial.println(posSolicitada);
    server.send(200, "application/json",
                "{\"status\":\"MOVENDO\",\"pos_solicitada\":" + String(posSolicitada) + "}");
  } else {
    server.send(400, "application/json",
                "{\"erro\":\"Parametro 'pos' nao enviado\"}");
  }
}

void handleLastResult() {
  String json = "{";
  json += "\"status\":\"POS ATINGIDA\",";
  json += "\"pos_solicitada\":" + String(posSolicitada) + ",";
  json += "\"pos_atual\":" + String(lastPos) + ",";
  json += "\"fim_curso1\":" + String(fimCurso1 ? 1 : 0) + ",";
  json += "\"fim_curso2\":" + String(fimCurso2 ? 1 : 0) + ",";
  json += "\"fim_curso3\":" + String(fimCurso3 ? 1 : 0);
  json += "}";
  server.send(200, "application/json", json);
}

// ----------- Setup -----------
void setup() {
  Serial.begin(115200);

  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  pinMode(PIN_BTN3, INPUT_PULLUP);
  pinMode(PIN_BTN4, INPUT_PULLUP);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  setLedEstado("INICIO"); // LED vermelho até conectar WiFi

  // Conecta WiFi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    setLedEstado("WIFI_CONECTADO"); // LED azul
  } else {
    Serial.println("\nFalha ao conectar. Rodando OFFLINE");
    // opcional: LED vermelho ou verde se offline
  }

  // Endpoints
  server.on("/", handleRoot);
  server.on("/api/move_pos", handleMovePos);
  server.on("/api/last_result", handleLastResult);
  server.begin();

  // Cria task do motor no Core 1
  xTaskCreatePinnedToCore(
    motorTask,
    "MotorTask",
    4096,
    NULL,
    1,
    &motorTaskHandle,
    1
  );
}

// ----------- Loop -----------
void loop() {
  server.handleClient(); // roda no Core 0
}
