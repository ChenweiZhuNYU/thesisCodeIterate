#include <ArduinoJson.h>

// 按钮引脚
#define BUTTON_1 9
#define BUTTON_2 10

// 绿灯引脚
#define GREEN_1 3
#define GREEN_2 5

// 继电器引脚
#define RELAY_PIN 13

// 时间配置（单位：毫秒）
const unsigned long VIDEO1_DURATION = 90000;    // 1 分 30 秒
const unsigned long VIDEO2_DURATION = 100000;   // 1 分 40 秒
const unsigned long RELAY_DELAY     = 94000;    // 1 分 34 秒
const unsigned long RELAY_DURATION  = 5000;     // 5 秒
const unsigned long MANUAL_RELAY_HOLD_TIME = 5000; // 手动触发继电器的长按时间

// 状态变量
bool isPlaying1 = false;
bool isPlaying2 = false;
unsigned long playStart1 = 0;
unsigned long playStart2 = 0;

// 继电器状态
bool relayWaiting = false;
bool relayActive = false;
unsigned long relayWaitStart = 0;
unsigned long relayOnStart = 0;

// 长按检测
bool button2WasPressed = false;
unsigned long button2PressStart = 0;
bool manualRelayTriggered = false;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(GREEN_1, OUTPUT);
  pinMode(GREEN_2, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_1, HIGH);
  digitalWrite(GREEN_2, HIGH);

  while (!Serial);
  Serial.println("✅ Arduino Ready");
}

void loop() {
  unsigned long now = millis();

  // 检查播放是否结束
  if (isPlaying1 && now - playStart1 >= VIDEO1_DURATION) {
    isPlaying1 = false;
    digitalWrite(GREEN_1, HIGH);
  }

  if (isPlaying2 && now - playStart2 >= VIDEO2_DURATION) {
    isPlaying2 = false;
    digitalWrite(GREEN_2, HIGH);
  }

  // 按钮 1 逻辑
  if (digitalRead(BUTTON_1) == LOW) {
    if (!isPlaying1) {
      isPlaying1 = true;
      playStart1 = now;
      digitalWrite(GREEN_1, LOW);
    }
  }

  // 按钮 2 正常逻辑 + 长按逻辑
  bool button2State = digitalRead(BUTTON_2) == LOW;

  if (button2State && !button2WasPressed) {
    // 初次按下
    button2WasPressed = true;
    button2PressStart = now;

    if (!isPlaying2) {
      isPlaying2 = true;
      playStart2 = now;
      digitalWrite(GREEN_2, LOW);
      relayWaiting = true;
      relayWaitStart = now;
    }
  }

  if (!button2State && button2WasPressed) {
    // 松开按钮
    button2WasPressed = false;
    manualRelayTriggered = false;
  }

  if (button2WasPressed && !manualRelayTriggered && now - button2PressStart >= MANUAL_RELAY_HOLD_TIME) {
    // 长按 5 秒触发备用继电器逻辑
    digitalWrite(RELAY_PIN, HIGH);
    relayOnStart = now;
    relayActive = true;
    manualRelayTriggered = true;
  }

  // 正常继电器触发逻辑
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

  // 串口通信（按钮状态）
  if (Serial.available() > 0) {
    int cmd = Serial.read();
    if (cmd == 0xAB) {
      StaticJsonDocument<200> doc;
      JsonObject data = doc.createNestedObject("data");

      data["button1"] = digitalRead(BUTTON_1) == LOW;
      data["button2"] = digitalRead(BUTTON_2) == LOW;

      String res;
      serializeJson(doc, res);
      Serial.println(res);
    }
  }

  delay(5);
}
