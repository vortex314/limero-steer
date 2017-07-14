// file: main.cpp
#include <LedBlinker.h>
#include <Mqtt.h>
#include <MqttJson.h>
#include <Sys.h>
#include <Wifi.h>
#include <cJSON.h>
#include <main_labels.h>
#include "Arduino.h"

Uid uid(200);
EventBus eb(4096, 1024);
Log logger(256);

LedBlinker led;
Wifi wifi("wifi");
Mqtt mqtt("mqtt", 1024);
MqttJson gateway("gateway", 1024);

#define PIN_MOTOR_IN1 22
#define PIN_MOTOR_IN2 23

void motor_setup() {
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  digitalWrite(PIN_MOTOR_IN1, 1);
  digitalWrite(PIN_MOTOR_IN2, 1);
}

class Motor : public Actor {
 public:
  int state;
  Motor() : Actor("motor") { state = 0; }
  void setup() {
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);
    digitalWrite(PIN_MOTOR_IN1, 1);
    digitalWrite(PIN_MOTOR_IN2, 1);
    timeout(1000);
    eb.onDst(id()).call(this);
  }

  void turnLeft() {
    INFO("L");
    digitalWrite(PIN_MOTOR_IN1, 0);
    digitalWrite(PIN_MOTOR_IN2, 1);
  }

  void turnRight() {
    INFO("R");
    digitalWrite(PIN_MOTOR_IN1, 1);
    digitalWrite(PIN_MOTOR_IN2, 0);
  }

  void neutral() {
    INFO("N");
    digitalWrite(PIN_MOTOR_IN1, 1);
    digitalWrite(PIN_MOTOR_IN2, 1);
  }

  void onEvent(Cbor &msg) {
    if (state == 0) {
      turnLeft();
      timeout(4000);
    } else if (state == 1) {
      neutral();
      timeout(1000);
    } else if (state == 2) {
      turnRight();
      timeout(4000);
    } else {
      neutral();
      timeout(1000);
    }
    state++;
    if (state == 4) state = 0;
  }
};

Motor motor;

void jsontest() {
  char jsonLine[] =
      "{    \"name\": \"Jack (\\\"Bee\\\") Nimble\",    \"format\": {        "
      "\"type\":       \"rect\",        \"width\":      1920,        "
      "\"height\":     "
      "1080,        \"interlace\":  false,        \"frame rate\": 24    }}";
  cJSON *root = cJSON_Parse(jsonLine);
  cJSON *format = cJSON_GetObjectItem(root, "format");
  cJSON *framerate_item = cJSON_GetObjectItem(format, "frame rate");
  double framerate = 0;
  cJSON_DeleteItemFromObject(format, "width");
  char *output = cJSON_PrintBuffered(format, 100, 1);
  if (framerate_item->type == cJSON_Number) {
    framerate = framerate_item->valuedouble;
    INFO(" framerate : %f", framerate);
    INFO(" pretty : %s ", output);
  }
}

#define LED_BUILTIN 2
void setup() {
  Serial.begin(115200);

  Sys::init();
  uid.add(labels, LABEL_COUNT);
  Str hostname(40);
  char hn[20];
  sprintf(hn, "ESP%X", (uint32_t)ESP.getEfuseMac());  // The chip ID is
                                                      // essentially its MAC
                                                      // address(length: 6
                                                      // bytes).

  //    sprintf(hn,"ESP%X",ESP.getChipId());
  hostname = hn;

  //    system_update_cpu_freq(160);

  Sys::hostname(hn);

  Str ssid(40);
  ssid = "Merckx3";
  Str password(60);
  password = "LievenMarletteEwoutRonald";

  wifi.setConfig(ssid, password, hostname);
  wifi.setup();

  mqtt.setup();
  gateway.setMqttId(mqtt.id());
  gateway.setup();

  motor.setup();

  led.setMqtt(mqtt.id());
  led.setWifi(wifi.id());
  led.setup();

  logger.setLogLevel('D');
  eb.onAny().call([](Cbor &msg) {  // Log all events
    Str str(256);
    eb.log(str, msg);
    DEBUG("%s", str.c_str());
  });
  //  mqtt.setLog(true);
  jsontest();
}

void loop() {
  eb.eventLoop();
  wifi.loop();
  mqtt.loop();
}