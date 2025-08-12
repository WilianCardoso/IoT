#define LED_R 14
#define LED_G 33
#define LED_B 32

#define RGB_R 27
#define RGB_G 26
#define RGB_B 25

#define PIN_BTN 0 

bool ligado = false;       
bool ultimoEstadoBotao = HIGH; 
unsigned long ultimoDebounce = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(PIN_BTN, INPUT_PULLUP); 
}

void loop() {
  
  int leitura = digitalRead(PIN_BTN);
  if (leitura != ultimoEstadoBotao) {
    ultimoDebounce = millis();
    ultimoEstadoBotao = leitura;
  }

  if ((millis() - ultimoDebounce) > debounceDelay) {
    if (leitura == LOW) { 
      ligado = !ligado;
      Serial.println(ligado ? "Sequencia LIGADA" : "Sequencia DESLIGADA");
      delay(200); 
    }
  }

  if (ligado) {
    digitalWrite(LED_R, HIGH);
    Serial.println("LED_R ON");
    delay(800); 
    digitalWrite(LED_R, LOW);
    Serial.println("LED_R OFF"); 
    delay(800);

    digitalWrite(LED_G, HIGH);
    Serial.println("LED_G ON");
    delay(800); 
    digitalWrite(LED_G, LOW);
    Serial.println("LED_G OFF"); 
    delay(800);

    digitalWrite(LED_B, HIGH);
    Serial.println("LED_B ON");
    delay(800); 
    digitalWrite(LED_B, LOW);
    Serial.println("LED_B OFF"); 
    delay(800);

    digitalWrite(RGB_R, HIGH);
    Serial.println("RGB_R ON");
    delay(800);
    digitalWrite(RGB_R, LOW);
    Serial.println("RGB_R OFF");

    digitalWrite(RGB_G, HIGH);
    Serial.println("RGB_G ON");
    delay(800);
    digitalWrite(RGB_G, LOW);
    Serial.println("RGB_G OFF");

    digitalWrite(RGB_B, HIGH);
    Serial.println("RGB_B ON");
    delay(800);
    digitalWrite(RGB_B, LOW);
    Serial.println("RGB_B OFF");
  }
}