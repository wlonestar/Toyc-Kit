//! light weight Expected class

#ifndef EXPECTED_H
#define EXPECTED_H

#pragma once

#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace toyc {

enum ErrorCode {};

class Error {
private:
  ErrorCode code;
  std::string msg;

public:
  explicit Error(ErrorCode _code, std::string _msg)
      : code(_code), msg(std::move(_msg)) {}

  void log(std::ostream &os) const { os << msg; }
  int errorCode() const { return code; }
  const std::string message() const { return msg; }
};

static Error makeError(int code, std::string message) {
  return Error(static_cast<ErrorCode>(code), std::move(message));
}

template <typename T> class Expected {
private:
  bool hasVal;
  union {
    T value;
    Error error;
  };

public:
  Expected(const T &v) : hasVal(true), value(v) {}
  Expected(T &&v) : hasVal(true), value(std::move(v)) {}

  Expected(const Error &e) = delete;
  Expected(Error &&e) : hasVal(false), error(std::move(e)) {}

  Expected(const Expected<T> &other) = delete;
  Expected(Expected<T> &&other) : hasVal(other.hasVal) {
    if (hasVal) {
      new (&value) T(std::move(other.value));
    } else {
      error = std::move(other.error);
    }
  }

  ~Expected() {
    if (hasVal) {
      value.~T();
    }
  }

  explicit operator bool() const { return hasVal; }

  Expected<T> &operator=(const T &v) {
    if (hasVal) {
      value = v;
    } else {
      value.~T();
      new (&value) T(v);
      hasVal = true;
    }
    return *this;
  }

  Expected<T> &operator=(T &&v) {
    if (hasVal) {
      value.~T();
      new (&value) T(std::move(v));
    } else {
      value = std::move(v);
      hasVal = true;
    }
    return *this;
  }

  Expected<T> &operator=(Error &&e) {
    if (hasVal) {
      value.~T();
      new (&error) Error(std::move(e));
      hasVal = false;
    } else {
      error = std::move(e);
    }
    return *this;
  }

  Expected<T> &operator=(Expected<T> &&other) {
    if (this != &other) {
      if (hasVal) {
        value.~T();
      } else {
        error.~Error();
      }
      hasVal = other.hasVal;
      if (hasVal) {
        new (&value) T(std::move(other.value));
      } else {
        error = std::move(other.error);
      }
    }
    return *this;
  }

  const T &getValue() const {
    if (!hasVal) {
      throw std::logic_error("trying to access value of Expected with error");
    }
    return value;
  }

  T &&getValue() {
    if (!hasVal) {
      throw std::logic_error("trying to access value of Expected with error");
    }
    return std::move(value);
  }

  Error getError() {
    if (hasVal) {
      throw std::logic_error("trying to access error of Expected with value");
    }
    return error;
  }
};

} // namespace toyc

#endif
