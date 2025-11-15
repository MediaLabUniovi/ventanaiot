#include "config.h"
#include "globals.h"
#include "leds.h"
#include <Arduino.h>

void ledoff() {
    analogWrite(pinRojo, 0);
    analogWrite(pinVerde, 0);
    analogWrite(pinAzul, 0);
}

void ledrojo() {
    analogWrite(pinRojo, 128);
    analogWrite(pinVerde, 0);
    analogWrite(pinAzul, 0);
}

void ledverde() {
    analogWrite(pinRojo, 0);
    analogWrite(pinVerde, 128);
    analogWrite(pinAzul, 0);
}

void lednaranja() {
    analogWrite(pinRojo, 60);
    analogWrite(pinVerde, 128);
    analogWrite(pinAzul, 0);
}

Ticker led(ledoff, 15000, 1, MILLIS);

void enviarnoti(const std::string &texto) {
  bot.sendMessage(vilas, texto.c_str());
}
