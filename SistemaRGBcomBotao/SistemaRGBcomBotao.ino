#define LED_B 32

#define RGB_R 27
#define RGB_G 26
#define RGB_B 25

#define PIN_BTN 0

const long INTERVALO = 500; // tempo entre mudanças (ms)
bool ligado = false;        // estado da sequência
int ledAtual = 0;           // 0 = R, 1 = G, 2 = B

unsigned long tempAnterior = 0;

// Variáveis para debounce
bool estadoBotao = HIGH;
bool ultimoEstadoBotao = HIGH;
unsigned long ultimoDebounce = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);

  pinMode(LED_B, OUTPUT);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  // Começa todos apagados (HIGH, pois é ativo em LOW)
  digitalWrite(RGB_R, HIGH);
  digitalWrite(RGB_G, HIGH);
  digitalWrite(RGB_B, HIGH);
}

void loop() {
  int leitura = digitalRead(PIN_BTN);

  // Verifica mudança de estado do botão
  if (leitura != ultimoEstadoBotao) {
    ultimoDebounce = millis();
  }

  // Faz debounce
  if ((millis() - ultimoDebounce) > debounceDelay) {
    if (leitura != estadoBotao) {
      estadoBotao = leitura;

      // Botão pressionado (nível LOW)
      if (estadoBotao == LOW) {
        ligado = !ligado;
        Serial.println(ligado ? "Sequencia LIGADA" : "Sequencia DESLIGADA");
      }
    }
  }

  ultimoEstadoBotao = leitura;

  // Sequência de cores
  if (ligado && millis() - tempAnterior >= INTERVALO) {
    tempAnterior = millis();

    // Apaga todos
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);

    delay(5); // evita ghosting

    // Liga apenas a cor certa
    if (ledAtual == 0) {
      digitalWrite(RGB_R, HIGH);
      Serial.println("RGB_R ON");
    } else if (ledAtual == 1) {
      digitalWrite(RGB_G, HIGH);
      Serial.println("RGB_G ON");
    } else if (ledAtual == 2) {
      digitalWrite(RGB_B, HIGH);
      Serial.println("RGB_B ON");
    }

    // Próxima cor
    ledAtual = (ledAtual + 1) % 3;
  }
}
