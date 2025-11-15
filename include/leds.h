#ifndef VENTANA_LEDS_H
#define VENTANA_LEDS_H

#include <Ticker.h>
#include <string>

void ledoff();
void ledrojo();
void ledverde();
void lednaranja();
void enviarnoti(const std::string &texto);

extern Ticker led;

#endif // VENTANA_LEDS_H
