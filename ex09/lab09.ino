#include <WiFi.h>
#include <WebServer.h>

// WiFi配置
const char* ssid = "OPPO Reno12 Pro";
const char* password = "12345678qwe";

// 引脚定义
#define TOUCH_PIN 4    // 触摸引脚 T0 (GPIO4)

// Web服务器对象
WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("正在连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功");
  Serial.print("ESP32 IP地址: ");
  Serial.println(WiFi.localIP());
  
  // 设置Web路由
  server.on("/", handleRoot);
  server.on("/api/touch", handleTouchAPI);
  
  server.begin();
  Serial.println("Web服务器已启动");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 触摸传感器实时仪表盘</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container { max-width: 600px; margin: 0 auto; }
    h1 {
      text-align: center;
      color: white;
      margin-bottom: 30px;
      font-size: 1.5rem;
    }
    .sensor-card {
      background: white;
      border-radius: 20px;
      padding: 30px;
      box-shadow: 0 20px 40px rgba(0,0,0,0.3);
      text-align: center;
    }
    .sensor-title {
      font-size: 1.2rem;
      color: #666;
      margin-bottom: 20px;
    }
    .sensor-value {
      font-size: 80px;
      font-weight: bold;
      font-family: 'Courier New', monospace;
      transition: all 0.1s ease;
    }
    .sensor-unit {
      color: #999;
      margin-bottom: 20px;
      font-size: 14px;
    }
    .progress-bar-container {
      background: #e0e0e0;
      border-radius: 20px;
      height: 30px;
      overflow: hidden;
      margin: 20px 0;
    }
    .progress-bar {
      width: 0%;
      height: 100%;
      transition: width 0.1s ease;
      border-radius: 20px;
    }
    .status-text {
      padding: 12px;
      border-radius: 10px;
      margin-top: 15px;
      font-weight: bold;
      font-size: 16px;
    }
    .info {
      background: rgba(255,255,255,0.95);
      border-radius: 15px;
      padding: 15px;
      margin-top: 20px;
      font-size: 14px;
      color: #333;
    }
    .range-indicator {
      display: flex;
      justify-content: space-between;
      margin-top: 10px;
      font-size: 12px;
      color: #888;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>📊 触摸传感器实时仪表盘</h1>
    <div class="sensor-card">
      <div class="sensor-title">
        🖐️ GPIO4 触摸传感器
      </div>
      <div class="sensor-value" id="touchValue">---</div>
      <div class="sensor-unit">模拟量数值 (触摸后数值会变小↓)</div>
      <div class="progress-bar-container">
        <div class="progress-bar" id="progressBar"></div>
      </div>
      <div class="range-indicator">
        <span>🔴 触摸中 (≈120)</span>
        <span>🟡 靠近中</span>
        <span>🟢 未触摸 (≈1200)</span>
      </div>
      <div class="status-text" id="statusText">⏳ 等待数据...</div>
    </div>
    <div class="info">
      <strong>📖 使用说明：</strong><br>
      • 用手指触摸或靠近 GPIO4 引脚上的杜邦线<br>
      • 数值会<strong style="color:#ff4444;">变小</strong>（触摸时从1200下降到120）<br>
      • 手指离开后数值恢复<br>
      • 进度条会随触摸变长，颜色变红
    </div>
  </div>

  <script>
    const touchValueElem = document.getElementById('touchValue');
    const progressBar = document.getElementById('progressBar');
    const statusText = document.getElementById('statusText');
    
    let history = [];
    
    function calculatePercent(value) {
      let minVal = 100;
      let maxVal = 1300;
      
      if (value <= minVal) return 100;
      if (value >= maxVal) return 0;
      
      return ((maxVal - value) / (maxVal - minVal)) * 100;
    }
    
    function getColor(value) {
      if (value <= 150) return '#ff0000';
      if (value <= 300) return '#ff4400';
      if (value <= 500) return '#ff8800';
      if (value <= 800) return '#ffcc00';
      return '#4CAF50';
    }
    
    function getStatus(value) {
      if (value <= 150) return '🔴 手指正在触摸！数值很低 🔴';
      if (value <= 300) return '🟠 手指触摸中';
      if (value <= 500) return '🟡 手指靠近中...';
      if (value <= 800) return '🟢 轻微感应';
      return '🟢 无触摸';
    }
    
    function updateUI(value) {
      touchValueElem.textContent = value;
      
      let percent = calculatePercent(value);
      percent = Math.min(100, Math.max(0, percent));
      
      let color = getColor(value);
      let status = getStatus(value);
      
      progressBar.style.width = percent + '%';
      progressBar.style.background = color;
      touchValueElem.style.color = color;
      
      history.push(value);
      if (history.length > 20) history.shift();
      
      let sum = 0;
      let minVal = 9999, maxVal = 0;
      for(let i = 0; i < history.length; i++) {
        sum += history[i];
        if(history[i] < minVal) minVal = history[i];
        if(history[i] > maxVal) maxVal = history[i];
      }
      let avg = sum / history.length;
      
      statusText.innerHTML = status + '<br><span style="font-size:11px;">📊 最近20次 | 最小:' + minVal + ' 最大:' + maxVal + ' 平均:' + Math.round(avg) + '</span>';
    }
    
    function fetchTouchValue() {
      fetch('/api/touch')
        .then(response => response.text())
        .then(data => {
          let value = parseInt(data);
          if (!isNaN(value)) {
            updateUI(value);
          }
        })
        .catch(error => {
          touchValueElem.textContent = '错误';
          statusText.innerHTML = '❌ 连接失败';
        });
    }
    
    setInterval(fetchTouchValue, 100);
    
    window.onload = function() {
      fetchTouchValue();
    };
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleTouchAPI() {
  int touchValue = touchRead(TOUCH_PIN);
  String response = String(touchValue);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", response);
  
  static int lastValue = 0;
  if (abs(touchValue - lastValue) > 20) {
    Serial.print("触摸值: ");
    Serial.println(touchValue);
    lastValue = touchValue;
  }
}