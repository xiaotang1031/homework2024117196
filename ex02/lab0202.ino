const int ledPin = 2;
unsigned long previousMillis = 0;
const long interval = 500;  // 500ms 翻转一次，周期 1Hz

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(ledPin, !digitalRead(ledPin));  // 翻转状态
  }
}