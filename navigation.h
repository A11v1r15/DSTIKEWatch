#pragma once
#include "utils.h"

class Face {
public:
  // ponteiros para as faces
  Face* next;
  Face* previous;
  Face* selected;

  // Construtor
  Face(Face* n = nullptr, Face* p = nullptr, Face* s = nullptr) {
    next = n;
    previous = p;
    selected = s;
  }

  Face* addNext(Face* n) {
    next = n;
    n->previous = this;
    return n;
  }

  Face* previousFace() {
    exit();
    previous->enter();
    return previous;
  }

  Face* nextFace() {
    exit();
    next->enter();
    return next;
  }

  virtual Face* handleUp(bool wasHold) {
    return previousFace();
  }

  virtual Face* handleDown(bool wasHold) {
    return nextFace();
  }

  virtual void enter() {
    tone(BUZZER_PIN, 600, 100);
  }

  // Método para executar a face atual
  virtual void show() {}
  virtual void action(bool wasHold) {}
  virtual void exit() {}
  virtual void update() {}
};