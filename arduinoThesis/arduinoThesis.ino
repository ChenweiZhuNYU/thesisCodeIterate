#include <ArduinoJson.h>

// **超声波传感器 1**
#define TRIG_PIN_1 9
#define ECHO_PIN_1 10

// **超声波传感器 2**
#define TRIG_PIN_2 12
#define ECHO_PIN_2 11

// **小灯泡**
#define LIGHT_PIN 13

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.0343 / 2;  // 计算厘米

  // **如果数据异常，返回 999**
  if (distance <= 0 || distance > 400) {
    return 999;
  }
  return distance;
}

void sendData() {
  StaticJsonDocument<200> resJson;
  JsonObject data = resJson.createNestedObject("data");

  // **读取两个超声波传感器的数据**
  float distance1 = readDistance(TRIG_PIN_1, ECHO_PIN_1);
  float distance2 = readDistance(TRIG_PIN_2, ECHO_PIN_2);

  data["distance1"] = distance1;  // 传感器 1 数据
  data["distance2"] = distance2;  // 传感器 2 数据

  String resTxt = "";
  serializeJson(resJson, resTxt);
  Serial.println(resTxt);  // **发送 JSON 数据**

  // **如果 distance2 < 5cm，等待 5 秒后点亮小灯泡 5 秒**
  if (distance2 > 0 && distance2 < 5) {
    delay(5000);          // **等待 5 秒**
    digitalWrite(LIGHT_PIN, HIGH);  // **打开小灯泡**
    delay(5000);          // **保持亮 5 秒**
    digitalWrite(LIGHT_PIN, LOW);   // **关闭小灯泡**
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);  // **初始化小灯泡为关闭状态**

  while (!Serial) {}

  Serial.println("🚀 Arduino 已启动！"); // **只在启动时打印**
}

void loop() {
  if (Serial.available() > 0) {
    int byteIn = Serial.read();

    if (byteIn == 0xAB) {  
      sendData();
    }
  }
  delay(50);  // **增加稳定性，防止数据刷太快**
}
