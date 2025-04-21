#include <ArduinoJson.h>

// 按钮引脚
#define BUTTON_1 9
#define BUTTON_2 10

// 绿灯引脚
#define GREEN_1 3
#define GREEN_2 5

// 继电器引脚
#define RELAY_PIN 13

// ⚠️ 视频和继电器定时（单位：毫秒）
const unsigned long VIDEO1_DURATION = 90000;    // 1 分 30 秒
const unsigned long VIDEO2_DURATION = 100000;   // 1 分 40 秒
const unsigned long RELAY_DELAY     = 94000;    // 1 分 34 秒
const unsigned long RELAY_DURATION  = 5000;     // 5 秒

// 播放状态
bool isPlaying1 = false;
bool isPlaying2 = false;
unsigned long playStart1 = 0;
unsigned long playStart2 = 0;

// 继电器状态
bool relayWaiting = false;
bool relayActive = false;
unsigned long relayWaitStart = 0;
unsigned long relayOnStart = 0;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  pinMode(GREEN_1, OUTPUT);
  pinMode(GREEN_2, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_1, HIGH); // 初始绿灯常亮
  digitalWrite(GREEN_2, HIGH);

  while (!Serial);
  Serial.println("✅ Arduino Ready");
}

void loop() {
  unsigned long now = millis();

  // 播放状态检查
  if (isPlaying1 && now - playStart1 >= VIDEO1_DURATION) {
    isPlaying1 = false;
    digitalWrite(GREEN_1, HIGH);  // 播放结束后亮绿灯
  }

  if (isPlaying2 && now - playStart2 >= VIDEO2_DURATION) {
    isPlaying2 = false;
    digitalWrite(GREEN_2, HIGH);  // 播放结束后亮绿灯
  }

  // 按钮 1
  if (digitalRead(BUTTON_1) == LOW) {
    if (!isPlaying1) {
      isPlaying1 = true;
      playStart1 = now;
      digitalWrite(GREEN_1, LOW);  // 播放时熄灭绿灯
    }
  }

  // 按钮 2
  if (digitalRead(BUTTON_2) == LOW) {
    if (!isPlaying2) {
      isPlaying2 = true;
      playStart2 = now;
      digitalWrite(GREEN_2, LOW);  // 播放时熄灭绿灯

      // 启动继电器延迟
      relayWaiting = true;
      relayWaitStart = now;
    }
  }

  // 继电器控制逻辑
  if (relayWaiting && now - relayWaitStart >= RELAY_DELAY) {
    relayWaiting = false;
    relayActive = true;
    relayOnStart = now;
    digitalWrite(RELAY_PIN, HIGH);
  }

  if (relayActive && now - relayOnStart >= RELAY_DURATION) {
    relayActive = false;
    digitalWrite(RELAY_PIN, LOW);
  }

  // 串口通信：按下时发送 true
  if (Serial.available() > 0) {
    int cmd = Serial.read();
    if (cmd == 0xAB) {
      StaticJsonDocument<200> doc;
      JsonObject data = doc.createNestedObject("data");

      bool b1 = digitalRead(BUTTON_1) == LOW;
      bool b2 = digitalRead(BUTTON_2) == LOW;

      data["button1"] = b1;
      data["button2"] = b2;

      String res;
      serializeJson(doc, res);
      Serial.println(res);
    }
  }

  delay(5);
}
