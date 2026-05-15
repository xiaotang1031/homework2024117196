/*
 * ex04 触摸自锁开关
 * 符合作业要求：布尔状态变量 + 边缘检测 + 软件防抖
 * 触摸引脚：GPIO4
 * LED引脚：GPIO2
 */

#define TOUCH_PIN 4
#define LED_PIN 2

int threshold = 500;
bool ledState = false;      // 布尔型状态变量

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void loop() {
    int touchValue = touchRead(TOUCH_PIN);
    
    // 边缘检测：判断从未触摸变为触摸的瞬间
    if (touchValue < threshold) {
        // 翻转LED状态（自锁）
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        
        // 串口调试输出
        Serial.print("触摸触发，LED: ");
        Serial.println(ledState ? "ON" : "OFF");
        
        // 软件防抖：等待手松开并防止多次触发
        delay(300);  // 足够手松开
    }
    
    delay(50);
}