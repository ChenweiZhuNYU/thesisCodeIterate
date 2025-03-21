#include <ArduinoJson.h>

// 按钮引脚
#define BUTTON_1 9
#define BUTTON_2 10

// 灯光引脚
#define RED_1 2
#define GREEN_1 3
#define RED_2 4
#define GREEN_2 5

// 继电器引脚
#define RELAY_PIN 13

// 视频时长（ms）
const unsigned long VIDEO1_DURATION = 53000;
const unsigned long VIDEO2_DURATION = 15000;
const unsigned long LIGHT_DURATION = 2000;
const unsigned long RELAY_DELAY = 5000;
const unsigned long RELAY_DURATION = 5000;

// 播放状态
bool isPlaying1 = false;
bool isPlaying2 = false;
unsigned long playStart1 = 0;
unsigned long playStart2 = 0;

// 灯状态
bool green1Active = false, red1Active = false;
bool green2Active = false, red2Active = false;
unsigned long green1Start = 0, red1Start = 0;
unsigned long green2Start = 0, red2Start = 0;

// 继电器状态
bool relayWaiting = false;
bool relayActive = false;
unsigned long relayWaitStart = 0;
unsigned long relayOnStart = 0;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  pinMode(RED_1, OUTPUT);
  pinMode(GREEN_1, OUTPUT);
  pinMode(RED_2, OUTPUT);
  pinMode(GREEN_2, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  clearAllLights();

  while (!Serial);
  Serial.println("✅ Arduino Ready");
}

void loop() {
  unsigned long now = millis();

  // 播放状态检查
  if (isPlaying1 && now - playStart1 >= VIDEO1_DURATION) {
    isPlaying1 = false;
  }
  if (isPlaying2 && now - playStart2 >= VIDEO2_DURATION) {
    isPlaying2 = false;
  }

  // 按钮 1
  if (digitalRead(BUTTON_1) == LOW) {
    if (!isPlaying1 && !green1Active && !red1Active) {
      green1Active = true;
      green1Start = now;
      digitalWrite(GREEN_1, HIGH);
      isPlaying1 = true;
      playStart1 = now;
    } else if (isPlaying1 && !green1Active && !red1Active) {
      red1Active = true;
      red1Start = now;
      digitalWrite(RED_1, HIGH);
    }
  }

  // 按钮 2
  if (digitalRead(BUTTON_2) == LOW) {
    if (!isPlaying2 && !green2Active && !red2Active) {
      green2Active = true;
      green2Start = now;
      digitalWrite(GREEN_2, HIGH);
      isPlaying2 = true;
      playStart2 = now;

      // 启动继电器延迟
      relayWaiting = true;
      relayWaitStart = now;
    } else if (isPlaying2 && !green2Active && !red2Active) {
      red2Active = true;
      red2Start = now;
      digitalWrite(RED_2, HIGH);
    }
  }

  // 自动熄灯
  if (green1Active && now - green1Start >= LIGHT_DURATION) {
    digitalWrite(GREEN_1, LOW);
    green1Active = false;
  }
  if (red1Active && now - red1Start >= LIGHT_DURATION) {
    digitalWrite(RED_1, LOW);
    red1Active = false;
  }
  if (green2Active && now - green2Start >= LIGHT_DURATION) {
    digitalWrite(GREEN_2, LOW);
    green2Active = false;
  }
  if (red2Active && now - red2Start >= LIGHT_DURATION) {
    digitalWrite(RED_2, LOW);
    red2Active = false;
  }

  // 继电器控制
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

  // 串口通信 - 不判断播放状态，直接报告按钮是否按下
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

void clearAllLights() {
  digitalWrite(RED_1, LOW);
  digitalWrite(GREEN_1, LOW);
  digitalWrite(RED_2, LOW);
  digitalWrite(GREEN_2, LOW);
}
