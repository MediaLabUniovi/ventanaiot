/*
  main.cpp — organización visual y comentarios sin cambios funcionales
  - Mantiene la misma lógica y llamadas a funciones extraídas en módulos.
  - Solo ajusta formato, comentarios y orden de secciones para mejor lectura.
*/

#include <Arduino.h>

#include "config.h"
#include "globals.h"

#include "leds.h"
#include "motor.h"
#include "sensors.h"
#include "mqtt.h"
#include "encoder.h"
#include "control.h"

// ---------------------------------------------------------------------------
// setup()
// Inicialización de periféricos, comunicaciones y subsistemas.
// ---------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);

  // --- LEDs ---------------------------------------------------------------
  pinMode(pinRojo, OUTPUT);
  pinMode(pinVerde, OUTPUT);
  pinMode(pinAzul, OUTPUT);

  // --- Pines de control del motor DC ------------------------------------
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(BP1, OUTPUT);
  pinMode(BP2, OUTPUT);

  // Asegurar estado inicial seguro
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(BP1, LOW);
  digitalWrite(BP2, LOW);

  // --- Stepper -----------------------------------------------------------
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // habilita driver
  stepper.setPinsInverted(true, false, false);
  stepper.setMaxSpeed(700);
  stepper.setAcceleration(100);

  // --- Finales de carrera ------------------------------------------------
  pinMode(swaperturapin, INPUT);
  pinMode(swcierrepin, INPUT);

  // --- Conexión Wi-Fi / Telegram / MQTT ---------------------------------
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red Wi-Fi...");
  }
  Serial.println("Conexión Wi-Fi establecida.");

  clientMQTT.setServer(mqtt_server, mqtt_port);
  clientMQTT.setCallback(callback);

  // --- Barrido inicial y mensajes de estado -----------------------------
  barridoInicial();
  bot.sendSimpleMessage(vilas, "Ajuste inicial COMPLETADO", "");

  // --- Sincronización horaria (NTP) -------------------------------------
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", ntpServer);
  Serial.println("Sincronizando hora con NTP...");
  bot.sendSimpleMessage(vilas, "Hora sincronizada con servidor NTP", "");

  // --- Encoder rotatorio ------------------------------------------------
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);

  bool circleValues = false;
  rotaryEncoder.setBoundaries(-1, 2, circleValues);
  rotaryEncoder.setAcceleration(0); // sin aceleración

  bot.sendSimpleMessage(vilas, "Mecanismo Ventana INICIADO", "");
}


// ---------------------------------------------------------------------------
// loop()
// Bucle principal: lectura de finales de carrera, ejecución del motor,
// interacción MQTT/encoder y actualización de tickers.
// ---------------------------------------------------------------------------
void loop()
{
  // --- Lectura finales de carrera ----------------------------------------
  bool swapertura = digitalRead(swaperturapin);
  bool swcierre  = digitalRead(swcierrepin);

  // Ejecutar movimiento si hay pasos pendientes
  if (swapertura == HIGH || swcierre == HIGH) {
    stepper.run();
  }

  // Detección y parada por finales de carrera
  // estados: 1=cerrado, 2=abierto, 3=cerrando, 4=abriendo
  if (swcierre == LOW && estado == 3) {
    Serial.println("swcierre ACCIONADO");
    estado = 1; // cerrado
    bot.sendSimpleMessage(vilas, "swcierre ACCIONADO", "");
    mstop();
  } else if (swapertura == LOW && estado == 4) {
    Serial.println("swapertura ACCIONADO");
    estado = 2; // abierto
    bot.sendSimpleMessage(vilas, "swapertura ACCIONADO", "");

    if (calibrar) {
      PASOS_180_GRADOS = stepper.currentPosition();
      Serial.println(PASOS_180_GRADOS);
    }
    mstop();
  }

  // --- MQTT --------------------------------------------------------------
  if (!clientMQTT.connected()) {
    reconnect();
  }
  clientMQTT.loop();

  // --- Encoder input ----------------------------------------------------
  rotary_loop();

  // --- Tickers / temporizadores ----------------------------------------
  led.update();
  leer.update();
  esperar.update();

  // Si no hay movimientos pendientes pero está en proceso, detener motor
  if (stepper.distanceToGo() == 0 && (estado == 3 || estado == 4)) {
    mstop();
  }
}
    


