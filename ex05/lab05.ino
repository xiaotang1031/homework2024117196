// 作业5：多档位触摸调速呼吸灯（纯 analogWrite 版本）
// 不需要 ledc 库

#define TOUCH_PIN 4      // 触摸引脚 GPIO4
#define LED_PIN 2        // LED引脚 GPIO2
#define THRESHOLD 25     // 触摸阈值

const int speedLevels[3] = {20, 10, 5};
int currentLevel = 0;
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED_PIN, OUTPUT);
  
  // ESP32 需要先设置 PWM 频率和分辨率
  analogWriteFrequency(LED_PIN, 5000);  // 5kHz
  analogWriteResolution(LED_PIN, 8);     // 8位分辨率 0-255
  
  Serial.println("多档位触摸调速呼吸灯已启动");
}

bool isTouchPressed() {
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouch = (touchValue < THRESHOLD);
  
  if (currentTouch && !lastTouchState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (touchRead(TOUCH_PIN) < THRESHOLD) {
        lastDebounceTime = millis();
        lastTouchState = true;
        return true;
      }
    }
  } else if (!currentTouch) {
    lastTouchState = false;
  }
  return false;
}

void breathingLight(int speedDelay) {
  // 渐亮
  for (int duty = 0; duty <= 255; duty++) {
    analogWrite(LED_PIN, duty);
    delay(speedDelay);
  }
  // 渐暗
  for (int duty = 255; duty >= 0; duty--) {
    analogWrite(LED_PIN, duty);
    delay(speedDelay);
  }
}

void loop() {
  if (isTouchPressed()) {
    currentLevel = (currentLevel + 1) % 3;
    Serial.print("档位 ");
    Serial.println(currentLevel + 1);
    delay(100);
  }
  breathingLight(speedLevels[currentLevel]);
}