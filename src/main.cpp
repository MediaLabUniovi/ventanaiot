/* Refactorizado: main minimal - delega en módulos */

#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "leds.h"
#include "motor.h"
#include "sensors.h"
#include "mqtt.h"
#include "encoder.h"
#include "control.h"

void setup()
{
  Serial.begin(115200);

  // Pines LED
  pinMode(pinRojo, OUTPUT);
  pinMode(pinVerde, OUTPUT);
  pinMode(pinAzul, OUTPUT);

  // Pines del motor
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(BP1, OUTPUT);
  pinMode(BP2, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(BP1, LOW);
  digitalWrite(BP2, LOW);

  // Stepper
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);
  stepper.setPinsInverted(true, false, false);
  stepper.setMaxSpeed(700);
  stepper.setAcceleration(100);
  /////////////// Configuración MQTT ///////////////
  clientMQTT.setServer(mqtt_server, mqtt_port);
  clientMQTT.setCallback(callback);

  /////////////// Mensaje de inicio por Telegram ///////////////
  barridoInicial();
  bot.sendSimpleMessage(vilas, "Ajuste inicial COMPLETADO", "");

  /////////////// Sincronización horaria vía NTP ///////////////
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", ntpServer);
  Serial.println("Sincronizando hora con NTP...");
  bot.sendSimpleMessage(vilas, "Hora sincronizada con servidor NTP", "");

  /////////////// Inicialización del encoder rotatorio ///////////////
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);

  // Establecer límites del encoder (-1 a 2)
  bool circleValues = false;
  rotaryEncoder.setBoundaries(-1, 2, circleValues);

  // Configurar aceleración del encoder
  rotaryEncoder.setAcceleration(0); // 0 = sin aceleración

  // Opcional: deshabilitar aceleración
  // rotaryEncoder.disableAcceleration();
  bot.sendSimpleMessage(vilas, "Mecanismo Ventana INICIADO", ""); 
}


void loop()
{
  // Leer el estado de los sensores de apertura y cierre
  bool swapertura = digitalRead(swaperturapin); 
  bool swcierre = digitalRead(swcierrepin); 

  // Ejecutar el movimiento del motor
  if (swapertura == HIGH || swcierre == HIGH) stepper.run(); 
  
  ////////// Detección y parada por finales de carrera //////////
  if (swcierre == LOW && estado == 3) {
      Serial.println("swcierre ACCIONADO");
      estado = 1; // Estado: Cerrado
      bot.sendSimpleMessage(vilas, "swcierre ACCIONADO", ""); 
      mstop(); // Parar motor y colocar estado neutro
  } // 1-cerrado, 2-abierto, 3-cerrando, 4-abriendo
  else if (swapertura == LOW && estado == 4) {
      Serial.println("swapertura ACCIONADO");
      estado = 2; // Estado: Abierto
      bot.sendSimpleMessage(vilas, "swapertura ACCIONADO", "");

      if (calibrar) { 
        PASOS_180_GRADOS = stepper.currentPosition();  // Calibrar la referencia de recorrido
        Serial.println(PASOS_180_GRADOS);
      }
      mstop();    // Parar motor y colocar estado neutro
  }

  //////////////////// MQTT ////////////////////
  if (!clientMQTT.connected()) reconnect(); // Reintentar conexión MQTT
  clientMQTT.loop(); // Mantener conexión activa

  //////////////// Lectura del encoder //////////////////
  rotary_loop(); // Leer estado del encoder

  ////////////// Temporizadores //////////////////
  led.update();     
  leer.update();    
  esperar.update(); 
  ////////////////////////////////////////////////

  // Si no hay más pasos pendientes y está abriendo/cerrando, detener motor
  if (stepper.distanceToGo() == 0) {   
      if (estado == 3 || estado == 4) mstop(); // Parar motor y colocar estado neutro
  }
}
    


