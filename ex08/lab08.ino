#include <WiFi.h>
#include <WebServer.h>

// WiFi配置
const char* ssid = "OPPO Reno12 Pro";
const char* password = "12345678qwe";

// 引脚定义
#define TOUCH_PIN 4    // 触摸引脚 T0 (GPIO4)
#define LED_PIN   2    // 板载LED (GPIO2)

// 系统状态
bool armed = false;         // 布防标志
bool alarmActive = false;   // 报警激活标志（LED狂闪中）

// 触摸阈值（已修改为300）
int threshold = 300;

// Web服务器对象
WebServer server(80);

// 变量用于LED闪烁控制
unsigned long lastBlinkTime = 0;
const int blinkInterval = 100;  // 100ms闪烁一次（狂闪效果）

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  
  // 设置Web路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("系统已启动，请访问上方IP地址");
  Serial.print("当前触摸阈值: ");
  Serial.println(threshold);
}

void loop() {
  server.handleClient();  // 处理Web请求
  
  // 读取触摸值
  int touchValue = touchRead(TOUCH_PIN);
  
  // 串口打印触摸值（方便调试）
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("Touch Value: ");
    Serial.print(touchValue);
    Serial.print(" | Threshold: ");
    Serial.print(threshold);
    Serial.print(" | Armed: ");
    Serial.print(armed);
    Serial.print(" | Alarm: ");
    Serial.println(alarmActive);
  }
  
  // 布防状态下，检测触摸触发报警
  // 注意：触摸时数值会变小，所以当触摸值小于阈值时触发
  if (armed && !alarmActive) {
    if (touchValue < threshold) {
      alarmActive = true;
      Serial.println("⚠️⚠️⚠️ ALARM TRIGGERED! ⚠️⚠️⚠️");
    }
  }
  
  // 报警激活时的LED狂闪控制
  if (alarmActive) {
    unsigned long now = millis();
    if (now - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = now;
      static bool ledState = false;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } 
  // 非报警状态，LED保持熄灭
  else {
    digitalWrite(LED_PIN, LOW);
  }
  
  delay(50);  // 适当延时
}

// 主页面
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 安防系统</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; background: #f0f0f0; }
    .container { max-width: 500px; margin: 0 auto; background: white; padding: 30px; border-radius: 20px; box-shadow: 0 0 20px rgba(0,0,0,0.1); }
    button { font-size: 24px; padding: 15px 30px; margin: 10px; border: none; border-radius: 10px; cursor: pointer; transition: 0.3s; }
    .arm { background-color: #4CAF50; color: white; }
    .arm:hover { background-color: #45a049; transform: scale(1.05); }
    .disarm { background-color: #f44336; color: white; }
    .disarm:hover { background-color: #da190b; transform: scale(1.05); }
    .status { font-size: 24px; margin: 20px; padding: 15px; border-radius: 10px; }
    .armed { background-color: #4CAF50; color: white; }
    .disarmed { background-color: #f44336; color: white; }
    .alarm { background-color: #ff0000; color: white; font-weight: bold; animation: blink 0.5s step-end infinite; padding: 10px; border-radius: 10px; }
    @keyframes blink { 50% { opacity: 0; } }
    .info { margin-top: 30px; padding: 15px; background: #e3f2fd; border-radius: 10px; text-align: left; }
  </style>
  <script>
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          const statusDiv = document.getElementById('status');
          if (data.armed) {
            statusDiv.innerHTML = '🔒 系统状态：布防中 🔒';
            statusDiv.className = 'status armed';
          } else {
            statusDiv.innerHTML = '🔓 系统状态：撤防中 🔓';
            statusDiv.className = 'status disarmed';
          }
          if (data.alarmActive) {
            document.getElementById('alarm').innerHTML = '<div class="alarm">🚨 报警触发中！请立即撤防！ 🚨</div>';
          } else {
            document.getElementById('alarm').innerHTML = '';
          }
        });
    }
    function arm() {
      fetch('/arm').then(() => updateStatus());
    }
    function disarm() {
      fetch('/disarm').then(() => updateStatus());
    }
    setInterval(updateStatus, 1000);
    window.onload = updateStatus;
  </script>
</head>
<body>
  <div class="container">
    <h1>🏠 ESP32 触摸安防系统</h1>
    <div id="status" class="status">加载中...</div>
    <div id="alarm"></div>
    <button class="arm" onclick="arm()">🔒 布防</button>
    <button class="disarm" onclick="disarm()">🔓 撤防</button>
    <div class="info">
      <strong>📋 使用说明：</strong><br>
      1️⃣ 点击【布防】进入警戒状态<br>
      2️⃣ 触摸GPIO4引脚（杜邦线）→ LED狂闪报警<br>
      3️⃣ 手离开后报警仍持续<br>
      4️⃣ 点击【撤防】停止报警并解除警戒<br>
      <br>
      <strong>🔌 硬件连接：</strong><br>
      • GPIO4 → 杜邦线（触摸探头）<br>
      • GPIO2 → 板载LED（已集成）<br>
      <br>
      <strong>⚙️ 当前阈值: 300</strong><br>
      <strong>💡 触摸时数值会变小，小于300即触发报警</strong>
    </div>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// 布防动作
void handleArm() {
  armed = true;
  alarmActive = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("🔒 System Armed - 系统已布防");
  server.send(200, "text/plain", "Armed");
}

// 撤防动作
void handleDisarm() {
  armed = false;
  alarmActive = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("🔓 System Disarmed - 系统已撤防");
  server.send(200, "text/plain", "Disarmed");
}

// 返回JSON状态
void handleStatus() {
  String json = "{\"armed\":" + String(armed ? "true" : "false") +
                ",\"alarmActive\":" + String(alarmActive ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}