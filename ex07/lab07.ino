#include <WiFi.h>
#include <WebServer.h>

// AP模式配置
const char* ap_ssid = "ESP32-LAB";
const char* ap_pass = "12345678";

// 三个LED引脚定义
const int LED_RED = 4;
const int LED_YELLOW = 5;
const int LED_GREEN = 18;

// PWM配置
const int LEDC_FREQ = 5000;
const int LEDC_RES = 10;

// ========== 每个LED单独的最大亮度值（让三个灯100%时看起来亮度一致）==========
const int MAX_RED = 1023;      // 红色最大PWM
const int MAX_YELLOW = 800;    // 黄色降低（人眼对黄光最敏感）
const int MAX_GREEN = 900;     // 绿色略微降低

// 存储亮度百分比（0-100）
int redPercent = 0;
int yellowPercent = 0;
int greenPercent = 0;

WebServer server(80);

// 百分比转实际PWM值
int percentToPWM(int percent, int maxBrightness) {
  return map(percent, 0, 100, 0, maxBrightness);
}

// 设置红色亮度
void setRedBrightness(int percent) {
  redPercent = constrain(percent, 0, 100);
  int pwmValue = percentToPWM(redPercent, MAX_RED);
  ledcWrite(LED_RED, pwmValue);
}

void setYellowBrightness(int percent) {
  yellowPercent = constrain(percent, 0, 100);
  int pwmValue = percentToPWM(yellowPercent, MAX_YELLOW);
  ledcWrite(LED_YELLOW, pwmValue);
}

void setGreenBrightness(int percent) {
  greenPercent = constrain(percent, 0, 100);
  int pwmValue = percentToPWM(greenPercent, MAX_GREEN);
  ledcWrite(LED_GREEN, pwmValue);
}

// 手动拼接JSON（不依赖ArduinoJson库）
String getStatusJSON() {
  String json = "{\"red\":";
  json += redPercent;
  json += ",\"yellow\":";
  json += yellowPercent;
  json += ",\"green\":";
  json += greenPercent;
  json += "}";
  return json;
}

// 主页面
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>三色LED控制</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 30px; background: #1a1a2e; }
    .container { max-width: 500px; margin: 0 auto; background: #16213e; padding: 20px; border-radius: 20px; color: white; }
    .led-card { margin: 20px 0; padding: 15px; border-radius: 15px; background: #0f3460; }
    .led-red { border-left: 5px solid #ff4444; }
    .led-yellow { border-left: 5px solid #ffcc00; }
    .led-green { border-left: 5px solid #44ff44; }
    input[type="range"] { width: 80%; margin: 10px 0; cursor: pointer; height: 6px; border-radius: 3px; }
    #redSlider { background: linear-gradient(90deg, #330000, #ff4444); }
    #yellowSlider { background: linear-gradient(90deg, #332200, #ffcc00); }
    #greenSlider { background: linear-gradient(90deg, #003300, #44ff44); }
    button { padding: 10px 25px; margin: 5px; border: none; border-radius: 8px; cursor: pointer; font-size: 16px; font-weight: bold; }
    .btn-on { background: #4CAF50; color: white; }
    .btn-off { background: #f44336; color: white; }
    .brightness { font-size: 28px; font-weight: bold; display: inline-block; min-width: 50px; }
    hr { border-color: #0f3460; margin: 20px 0; }
    .tip { background: #0f3460; padding: 10px; border-radius: 8px; font-size: 12px; margin-top: 15px; }
  </style>
</head>
<body>
<div class="container">
  <h1>🎨 三色LED控制</h1>
  <p>亮度优化版 | 三灯最大亮度一致</p>
  
  <div class="led-card led-red">
    <h2>🔴 红色LED</h2>
    <div><span class="brightness" id="redVal">0</span>%</div>
    <input type="range" id="redSlider" min="0" max="100" value="0" oninput="setRed(this.value)">
    <div>
      <button class="btn-on" onclick="setRed(100)">最大亮度</button>
      <button class="btn-off" onclick="setRed(0)">熄灭</button>
    </div>
  </div>
  
  <div class="led-card led-yellow">
    <h2>🟡 黄色LED</h2>
    <div><span class="brightness" id="yellowVal">0</span>%</div>
    <input type="range" id="yellowSlider" min="0" max="100" value="0" oninput="setYellow(this.value)">
    <div>
      <button class="btn-on" onclick="setYellow(100)">最大亮度</button>
      <button class="btn-off" onclick="setYellow(0)">熄灭</button>
    </div>
  </div>
  
  <div class="led-card led-green">
    <h2>🟢 绿色LED</h2>
    <div><span class="brightness" id="greenVal">0</span>%</div>
    <input type="range" id="greenSlider" min="0" max="100" value="0" oninput="setGreen(this.value)">
    <div>
      <button class="btn-on" onclick="setGreen(100)">最大亮度</button>
      <button class="btn-off" onclick="setGreen(0)">熄灭</button>
    </div>
  </div>
  
  <hr>
  <button style="background:#4CAF50;" onclick="setAll(100)">✨ 全部最大亮度</button>
  <button style="background:#f44336;" onclick="setAll(0)">🌙 全部熄灭</button>
  
  <div class="tip">
    💡 提示：如果某个LED不够亮，请检查：<br>
    1. 电阻是否过大？（建议220Ω或100Ω）<br>
    2. LED是否接反（长脚接GPIO，短脚接GND）<br>
    3. 可更换LED测试
  </div>
  <p>📡 热点: ESP32-LAB | 密码: 12345678</p>
</div>

<script>
function sendCommand(cmd) {
  fetch(cmd).catch(e => console.log(e));
}

function setRed(val) {
  document.getElementById('redVal').innerText = val;
  document.getElementById('redSlider').value = val;
  sendCommand('/red?val=' + val);
}

function setYellow(val) {
  document.getElementById('yellowVal').innerText = val;
  document.getElementById('yellowSlider').value = val;
  sendCommand('/yellow?val=' + val);
}

function setGreen(val) {
  document.getElementById('greenVal').innerText = val;
  document.getElementById('greenSlider').value = val;
  sendCommand('/green?val=' + val);
}

function setAll(val) {
  setRed(val);
  setYellow(val);
  setGreen(val);
}

function updateStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      document.getElementById('redVal').innerText = data.red;
      document.getElementById('redSlider').value = data.red;
      document.getElementById('yellowVal').innerText = data.yellow;
      document.getElementById('yellowSlider').value = data.yellow;
      document.getElementById('greenVal').innerText = data.green;
      document.getElementById('greenSlider').value = data.green;
    });
}
setInterval(updateStatus, 500);
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

void handleRed() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    setRedBrightness(val);
  }
  server.send(200, "text/plain", "OK");
}

void handleYellow() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    setYellowBrightness(val);
  }
  server.send(200, "text/plain", "OK");
}

void handleGreen() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    setGreenBrightness(val);
  }
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  server.send(200, "application/json", getStatusJSON());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("三色LED控制启动");
  
  // 初始化PWM
  ledcAttach(LED_RED, LEDC_FREQ, LEDC_RES);
  ledcAttach(LED_YELLOW, LEDC_FREQ, LEDC_RES);
  ledcAttach(LED_GREEN, LEDC_FREQ, LEDC_RES);
  
  setRedBrightness(0);
  setYellowBrightness(0);
  setGreenBrightness(0);
  
  Serial.print("红色最大PWM: ");
  Serial.println(MAX_RED);
  Serial.print("黄色最大PWM: ");
  Serial.println(MAX_YELLOW);
  Serial.print("绿色最大PWM: ");
  Serial.println(MAX_GREEN);
  Serial.println("=================================");
  
  // AP模式
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);
  
  Serial.print("热点: ");
  Serial.println(ap_ssid);
  Serial.print("密码: ");
  Serial.println(ap_pass);
  Serial.print("访问: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("=================================");
  
  server.on("/", handleRoot);
  server.on("/red", handleRed);
  server.on("/yellow", handleYellow);
  server.on("/green", handleGreen);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("服务器已启动");
}

void loop() {
  server.handleClient();
}