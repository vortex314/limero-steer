// file: main.cpp
#include <LedBlinker.h>
#include <Mqtt.h>
#include <MqttJson.h>
#include <Sys.h>
#include <System.h>
#include <Wifi.h>
#include <cJSON.h>
#include <main_labels.h>
#include "Arduino.h"

Uid uid(200);
EventBus eb(32000, 1024);
Log logger(256);

LedBlinker led;
System sys("system");
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
      timeout(3000);
    } else if (state == 1) {
      neutral();
      timeout(1000);
    } else if (state == 2) {
      turnRight();
      timeout(3000);
    } else {
      neutral();
      timeout(1000);
    }
    state++;
    if (state == 4) state = 0;
  }
};

Motor motor;

#define LED_BUILTIN 2
void setup() {
  Serial.begin(921600);

  Sys::init();
  uid.add(labels, LABEL_COUNT);
  eb.setup();
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

  logger.setLogLevel('I');
  eb.onAny().call([](Cbor &msg) {  // Log all events
    Str str(256);
    eb.log(str, msg);
    DEBUG("%s", str.c_str());
  });

  sys.setup();

  wifi.setConfig(ssid, password, hostname);
  wifi.setup();

  mqtt.setWifiId(wifi.id());
  mqtt.setup();

  gateway.setMqttId(mqtt.id());
  gateway.setup();

  motor.setup();

  led.setMqtt(mqtt.id());
  led.setWifi(wifi.id());
  led.setup();
}

void measure(const char *s) {
  static uint64_t startTime;
  uint32_t delta;
  delta = Sys::millis() - startTime;
  startTime = Sys::millis();
  if (delta > 20) WARN(" slow %s :  %d msec", s, delta);
}

extern "C" {
#include <esp_task_wdt.h>
}

void loop() {
  measure(" outside loop ");

  eb.eventLoop();
  measure(" eventLoop ");

  wifi.loop();
  measure(" wifi loop");

  mqtt.loop();
  measure("mqtt loop");

  vTaskDelay(1);

  //  esp_task_wdt_feed();
}