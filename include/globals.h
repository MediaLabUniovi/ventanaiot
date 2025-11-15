#ifndef VENTANA_GLOBALS_H
#define VENTANA_GLOBALS_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <AccelStepper.h>
#include <AiEsp32RotaryEncoder.h>
#include <PubSubClient.h>

// Objetos compartidos
extern HTTPClient http;
extern WiFiClientSecure client;
extern WiFiClient espClient;
extern UniversalTelegramBot bot;
extern AccelStepper stepper;
extern AiEsp32RotaryEncoder rotaryEncoder;
extern PubSubClient clientMQTT;

// Variables de estado compartidas
extern int posicionVentana;
extern bool autoMode;
extern bool calibrar;
extern int PASOS_180_GRADOS;
extern bool automatic;
extern int estado;
extern bool accionadoencoder;

// Pesos y horario
extern int pesoTemperatura;
extern int pesoCO2;
extern int pesoEnergia;

extern bool horarioActivo;
extern int horaInicio;
extern int minutoInicio;
extern int horaFin;
extern int minutoFin;

#endif // VENTANA_GLOBALS_H
