// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2;

void setup() {
  // 初始化串口通信，设置波特率为115200
  Serial.begin(115200);
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // ---- S：短闪3次（200ms亮，200ms灭）----
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    Serial.println("S - ON (short)");
    delay(200);
    digitalWrite(ledPin, LOW);
    Serial.println("S - OFF");
    delay(200);
  }
  delay(500);  // 字母之间的间隔

  // ---- O：长闪3次（600ms亮，200ms灭）----
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    Serial.println("O - ON (long)");
    delay(600);
    digitalWrite(ledPin, LOW);
    Serial.println("O - OFF");
    delay(200);
  }
  delay(500);  // 字母之间的间隔

  // ---- S：短闪3次（200ms亮，200ms灭）----
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    Serial.println("S - ON (short)");
    delay(200);
    digitalWrite(ledPin, LOW);
    Serial.println("S - OFF");
    delay(200);
  }
  delay(2000); // 单词间隔，然后重复
}