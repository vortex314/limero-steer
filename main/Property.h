/*
 * Property.h
 *
 *  Created on: 23-jun.-2013
 *      Author: lieven2
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_
#include <Cbor.h>
#include <Uid.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "Erc.h"
#include "EventBus.h"
#include "Str.h"
#include "Sys.h"

enum Type {
  T_UINT8,
  T_UINT16,
  T_UINT32,
  T_UINT64,
  T_INT8,
  T_INT16,
  T_INT32,
  T_INT64,
  T_BOOL,
  T_FLOAT,
  T_DOUBLE,
  T_BYTES,
  T_ARRAY,
  T_MAP,
  T_STR,
  T_OBJECT
};

enum Mode { M_READ, M_WRITE };

enum Qos { QOS_0, QOS_1, QOS_2 };

class PropertyBase {
  static PropertyBase* _first;
  PropertyBase* _next;
  static PropertyBase* _current;
  uid_t _service;
  uid_t _property;
  uint32_t _interval;
  uint64_t _timeout;
  bool _isReady;

 public:
  PropertyBase(uid_t service, uid_t property, uint32_t interval) {
    //    _var = var;
    _service = service;
    _property = property;
    _interval = interval;
    _timeout = 0;
    _isReady = false;
    _next = 0;
    add(this);
  }
  void add(PropertyBase* p) {
    if (_first == 0) {
      _first = p;
    } else {
      PropertyBase* cursor;
      for (cursor = _first; cursor; cursor = cursor->_next) {
        if (cursor->_next == 0) {
          cursor->_next = p;
          break;
        }
      };
    }
  }

  void setReady(bool b) { _isReady = b; }

  bool isReady() {
    if (_isReady == true) {
      return true;
    }
    if (Sys::millis() > _timeout) {
      _timeout = Sys::millis() + _interval;
      return true;
    }
    return false;
  }

  uid_t service() { return _service; }

  uid_t property() { return _property; }

  static PropertyBase* nextReady() {
    if (_current == 0) {
      _current = _first;
    } else {
      _current = _current->_next;
    };
    while (_current) {
      if (_current && (_current->isReady())) return _current;
      _current = _current->_next;
    }
    return _current;
  }

  virtual void addEventCbor(Cbor& cbor) = 0;
};

template <typename T>
class Property : public PropertyBase {
  typedef T (*Func)(void);
  typedef enum { PROP_STATIC, PROP_FUNC } PropertyType;
  union {
    T& _var;
    Func _func;
  };
  PropertyType _type;

 private:
  Property(T& var, uid_t service, uid_t property, uint32_t interval)
      : PropertyBase(service, property, interval),
        _var(var),
        _type(PROP_STATIC) {}
  Property(Func func, uid_t service, uid_t property, uint32_t interval)
      : PropertyBase(service, property, interval),
        _func(func),
        _type(PROP_FUNC) {}

 public:
  static Property* build(T& var, uid_t service, uid_t property,
                         uint32_t interval) {
    return new Property(var, service, property, interval);
  }
  static Property* build(Func func, uid_t service, uid_t property,
                         uint32_t interval) {
    return new Property(func, service, property, interval);
  }
  void addEventCbor(Cbor& cbor) {
    if (_type == PROP_FUNC) {
      cbor.addKeyValue(EB_SRC, service())
          .addKeyValue(EB_EVENT, property())
          .addKeyValue(property(), _func())
          .addKeyValue(H("public"), true);
    } else {
      cbor.addKeyValue(EB_SRC, service())
          .addKeyValue(EB_EVENT, property())
          .addKeyValue(property(), _var)
          .addKeyValue(H("public"), true);
    }
    setReady(false);
  }

  virtual ~Property() {}
};

#endif /* PROPERTY_H_ */
