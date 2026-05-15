// 作业6：警车双闪灯效（双通道PWM）
// 两个LED交替渐变闪烁，一个变亮时另一个变暗

#define TOUCH_PIN 4      // 触摸引脚 GPIO4（用于开关）
#define LED_A_PIN 2      // 灯A GPIO2
#define LED_B_PIN 5      // 灯B GPIO5
#define THRESHOLD 25     // 触摸阈值

bool isFlashing = true;      // 是否运行双闪
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  pinMode(LED_A_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  digitalWrite(LED_A_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
  delay(500);
  Serial.println("警车双闪灯效启动");
  Serial.println("触摸一次暂停/恢复双闪");
}

// 检测触摸按下瞬间
bool isTouchPressed() {
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouch = (touchValue < THRESHOLD);
  
  if (currentTouch && !lastTouchState) {
    if (millis() - lastDebounceTime > debounceDelay) {
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

// 一次完整的双闪渐变（反相）
void oneDoubleFlash() {
  // 阶段1：A渐亮（0→255），B渐暗（255→0）
  for (int i = 0; i <= 255; i++) {
    // 控制 LED A（渐亮）
    digitalWrite(LED_A_PIN, HIGH);
    delayMicroseconds(i * 10);
    digitalWrite(LED_A_PIN, LOW);
    delayMicroseconds((255 - i) * 10);
    
    // 控制 LED B（渐暗）
    digitalWrite(LED_B_PIN, HIGH);
    delayMicroseconds((255 - i) * 10);
    digitalWrite(LED_B_PIN, LOW);
    delayMicroseconds(i * 10);
  }
  
  // 阶段2：A渐暗（255→0），B渐亮（0→255）
  for (int i = 255; i >= 0; i--) {
    // 控制 LED A（渐暗）
    digitalWrite(LED_A_PIN, HIGH);
    delayMicroseconds(i * 10);
    digitalWrite(LED_A_PIN, LOW);
    delayMicroseconds((255 - i) * 10);
    
    // 控制 LED B（渐亮）
    digitalWrite(LED_B_PIN, HIGH);
    delayMicroseconds((255 - i) * 10);
    digitalWrite(LED_B_PIN, LOW);
    delayMicroseconds(i * 10);
  }
}

void loop() {
  // 检测触摸切换运行状态
  if (isTouchPressed()) {
    isFlashing = !isFlashing;
    if (isFlashing) {
      Serial.println("▶ 双闪效果启动");
    } else {
      Serial.println("⏸ 双闪效果暂停");
      digitalWrite(LED_A_PIN, LOW);
      digitalWrite(LED_B_PIN, LOW);
    }
    delay(200);
  }
  
  // 如果处于运行状态，执行双闪
  if (isFlashing) {
    oneDoubleFlash();
  }
}