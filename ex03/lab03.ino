const int ledPin = 2;
enum State { SHORT_ON, SHORT_OFF, LONG_ON, LONG_OFF, PAUSE };
State state = SHORT_ON;
int flashCount = 0;
int shortCounter = 0;
int longCounter = 0;
unsigned long previousMillis = 0;

// 时间参数（毫秒）
const int SHORT_DUR = 200;   // 短闪亮的时间
const int SHORT_GAP = 200;   // 短闪之间的灭的时间
const int LONG_DUR = 600;    // 长闪亮的时间
const int LONG_GAP = 200;    // 长闪之间的灭的时间（可与短闪间隔一致）
const int LETTER_GAP = 500;  // 字母之间（S-O 或 O-S）的熄灭间隔
const int WORD_GAP = 2000;   // 单词结束（一次SOS完成）后的长停顿

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  previousMillis = millis();
  // 开始第一个短闪
  state = SHORT_ON;
  flashCount = 0;
  shortCounter = 0;
  longCounter = 0;
}

void loop() {
  unsigned long now = millis();

  switch (state) {
    case SHORT_ON:
      digitalWrite(ledPin, HIGH);
      if (now - previousMillis >= SHORT_DUR) {
        previousMillis = now;
        digitalWrite(ledPin, LOW);
        shortCounter++;
        if (shortCounter < 3) {
          state = SHORT_OFF;   // 还没完成3次短闪，进入短闪间的熄灭
        } else {
          // 完成3次短闪
          shortCounter = 0;
          if (flashCount == 0) {
            // 刚完成第一个 S，准备进入 O
            state = PAUSE;
            previousMillis = now; // 重新计时，等待字母间隔
          } else {
            // 第二个 S，实际上是 SOS 的最后一个 S，完成后进入 WORD_GAP
            state = PAUSE;
          }
        }
      }
      break;

    case SHORT_OFF:
      digitalWrite(ledPin, LOW);
      if (now - previousMillis >= SHORT_GAP) {
        previousMillis = now;
        state = SHORT_ON;
      }
      break;

    case LONG_ON:
      digitalWrite(ledPin, HIGH);
      if (now - previousMillis >= LONG_DUR) {
        previousMillis = now;
        digitalWrite(ledPin, LOW);
        longCounter++;
        if (longCounter < 3) {
          state = LONG_OFF;
        } else {
          longCounter = 0;
          state = PAUSE;
        }
      }
      break;

    case LONG_OFF:
      digitalWrite(ledPin, LOW);
      if (now - previousMillis >= LONG_GAP) {
        previousMillis = now;
        state = LONG_ON;
      }
      break;

    case PAUSE:
      digitalWrite(ledPin, LOW);
      unsigned long pauseDuration;
      if (flashCount == 0) {
        // 第一个 S 完成后，进入 O 之前等待 LETTER_GAP
        pauseDuration = LETTER_GAP;
      } else if (flashCount == 1) {
        // O 完成后，进入第二个 S 之前等待 LETTER_GAP
        pauseDuration = LETTER_GAP;
      } else {
        // 完成整个 SOS，等待 WORD_GAP 后重新开始
        pauseDuration = WORD_GAP;
      }
      if (now - previousMillis >= pauseDuration) {
        previousMillis = now;
        if (flashCount == 0) {
          flashCount = 1;
          state = LONG_ON;   // 开始 O
        } else if (flashCount == 1) {
          flashCount = 2;
          state = SHORT_ON;  // 开始第二个 S
        } else {
          // 完成一个完整 SOS，重置状态
          flashCount = 0;
          state = SHORT_ON;
        }
      }
      break;
  }
}