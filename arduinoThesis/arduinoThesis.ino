#include <ArduinoJson.h>

#define BUTTON_1 9
#define BUTTON_2 10
#define GREEN_1 3
#define GREEN_2 5
#define RELAY_PIN 13

const unsigned long VIDEO1_DURATION = 90000;    // 1m30s
const unsigned long VIDEO2_DURATION = 100000;   // 1m40s
const unsigned long RELAY_DELAY     = 94000;    // 1m34s
const unsigned long RELAY_DURATION  = 5000;     // 5s
const unsigned long RESET_HOLD_TIME = 5000;     // 长按 5s 重置
const unsigned long RESET_COOLDOWN  = 3000;     // 重置后 3s 冷却

// 状态变量
bool isPlaying1 = false;
bool isPlaying2 = false;
unsigned long playStart1 = 0;
unsigned long playStart2 = 0;

bool relayWaiting = false;
bool relayActive = false;
unsigned long relayWaitStart = 0;
unsigned long relayOnStart = 0;

// 长按检测相关
bool button1WasPressed = false;
bool button2WasPressed = false;
unsigned long button1PressStart = 0;
unsigned long button2PressStart = 0;
bool resetCooldownActive = false;
unsigned long lastResetTime = 0;

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

  // 播放结束后恢复绿灯
  if (isPlaying1 && now - playStart1 >= VIDEO1_DURATION) {
    isPlaying1 = false;
    digitalWrite(GREEN_1, HIGH);
  }
  if (isPlaying2 && now - playStart2 >= VIDEO2_DURATION) {
    isPlaying2 = false;
    digitalWrite(GREEN_2, HIGH);
  }

  // Reset 冷却控制
  if (resetCooldownActive && now - lastResetTime >= RESET_COOLDOWN) {
    resetCooldownActive = false;
  }

  // 按钮 1：播放或重置逻辑
  bool b1 = digitalRead(BUTTON_1) == LOW;
  if (b1) {
    if (!button1WasPressed) {
      button1PressStart = now;
      button1WasPressed = true;
    } else {
      unsigned long held = now - button1PressStart;
      if (!resetCooldownActive && held > RESET_HOLD_TIME) {
        // ✅ 长按 1 重置播放状态
        isPlaying1 = false;
        isPlaying2 = false;
        digitalWrite(GREEN_1, HIGH);
        digitalWrite(GREEN_2, HIGH);
        lastResetTime = now;
        resetCooldownActive = true;
      }
    }
  } else {
    if (button1WasPressed && !resetCooldownActive && (now - button1PressStart < RESET_HOLD_TIME)) {
      if (!isPlaying1) {
        isPlaying1 = true;
        playStart1 = now;
        digitalWrite(GREEN_1, LOW);
      }
    }
    button1WasPressed = false;
  }

  // 按钮 2：播放或继电器逻辑
  bool b2 = digitalRead(BUTTON_2) == LOW;
  if (b2) {
    if (!button2WasPressed) {
      button2PressStart = now;
      button2WasPressed = true;
    } else {
      unsigned long held = now - button2PressStart;
      if (!relayActive && !relayWaiting && held > RESET_HOLD_TIME) {
        digitalWrite(RELAY_PIN, HIGH);
        relayActive = true;
        relayOnStart = now;
      }
    }
  } else {
    if (button2WasPressed && (now - button2PressStart < RESET_HOLD_TIME)) {
      if (!isPlaying2) {
        isPlaying2 = true;
        playStart2 = now;
        digitalWrite(GREEN_2, LOW);

        relayWaiting = true;
        relayWaitStart = now;
      }
    }
    button2WasPressed = false;
  }

  // 继电器延迟控制
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

  // 串口通信
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
