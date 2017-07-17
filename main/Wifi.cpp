/*
 * Wifi.cpp
 *
 *  Created on: Jun 27, 2016
 *      Author: lieven
 */

#include "Wifi.h"
#include <Config.h>

Wifi::Wifi(const char *name)
    : Actor(name), _ssid(30), _password(60), _hostname(30) {}

Wifi::~Wifi() {}

void Wifi::init() {
  _ssid = "SSID";
  _password = "PSWD";
}

void Wifi::switchState(int st) {
  if (st != Actor::state()) {
    Actor::state(st);
    if (st == H("connected")) {
      INFO(" Wifi Connected.");
      INFO(" ip : %s ", WiFi.localIP().toString().c_str());
      eb.event(id(), H("connected"));
      eb.send();
    } else {
      WARN(" Wifi Disconnected.");
      eb.event(id(), H("disconnected"));
      eb.send();
    }
  }
}

void Wifi::setup() {
  switchState(H("disconnected"));
#ifdef ESP8266
  WiFi.hostname(Sys::hostname());
#else
//  WiFi.begin(_hostname.c_str());
#endif

  //  WiFi.begin(_ssid.c_str(), _password.c_str());
  WiFi.enableSTA(true);
}

void Wifi::loop() {
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG("Connecting to %s ... ", _ssid.c_str());
    WiFi.begin(_ssid.c_str(), _password.c_str());

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      DEBUG(" wifi still connecting.");
      return;
    } else
      INFO(" wifi connected as host : '%s' on SSID : '%s' '%s' ", getHostname(),
           getSSID(), getPassword());
  }
  switchState(WiFi.status() == WL_CONNECTED ? H("connected")
                                            : H("disconnected"));
}

void Wifi::setConfig(Str &ssid, Str &password, Str &hostname) {
  _ssid = ssid;
  _password = password;
  _hostname = hostname;
}

const char *Wifi::getHostname() { return Sys::hostname(); }

const char *Wifi::getSSID() { return _ssid.c_str(); }

const char *Wifi::getPassword() { return _password.c_str(); }
