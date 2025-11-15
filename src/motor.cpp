#include "config.h"
#include "globals.h"
#include "motor.h"
#include "leds.h"
#include <Arduino.h>

void moverMotorPorcentaje(int* nuevaPosicion) {
    if (*nuevaPosicion < 0 || *nuevaPosicion > 100) {
        Serial.println("⚠️ Porcentaje fuera de rango (0-100)");
        return;
    }

    int pasosDestino = (*nuevaPosicion * abs(PASOS_180_GRADOS)) / 100;
    int pasosActual = (posicionVentana * abs(PASOS_180_GRADOS)) / 100;

    if (posicionVentana == 0 && *nuevaPosicion == 100) calibrar = true;

    Serial.printf("🔢 Moviendo de %d%% a %d%% (%d pasos -> %d pasos)\n", 
                  posicionVentana, *nuevaPosicion, pasosActual, pasosDestino);

    if (pasosDestino > pasosActual) {
        // Movimiento antihorario
        Serial.println("🔄 Dirección: antihorario");
        estado = 4; //ABRIR
        digitalWrite(BP1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(BP2, HIGH);
        digitalWrite(IN1, HIGH);
    } else if (pasosDestino < pasosActual) {
        // Movimiento horario
        Serial.println("🔁 Dirección: horario");
        estado = 3; //CERRAR
        digitalWrite(BP1, HIGH);
        digitalWrite(IN2, HIGH);
        digitalWrite(BP2, HIGH);
        digitalWrite(IN1, LOW);
    } else {
        Serial.println("✅ Ya está en la posición deseada");
        return;
    }

    lednaranja();
    int moverPasos = pasosDestino - pasosActual;
    stepper.move(moverPasos);

    if (!accionadoencoder) posicionVentana = *nuevaPosicion; 
}

void mstop() {
    if (estado == 3 || estado == 4) estado = 0;
    Serial.println("motor parado");
    stepper.setSpeed(0);
    stepper.stop();
    stepper.moveTo(stepper.currentPosition());
    if (accionadoencoder) 
    {
      posicionVentana = (stepper.currentPosition()/abs(PASOS_180_GRADOS))*100;
      accionadoencoder = false;
    }
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(BP1, LOW);
    digitalWrite(BP2, LOW);
    ledoff();
    stepper.setAcceleration(100);
}

void barridoInicial() {
  // Cerrar completamente
  estado = 3; // CERRAR
  digitalWrite(BP1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(BP2, HIGH);
  digitalWrite(IN1, LOW);
  // Borrar cualquier posición anterior
  stepper.setCurrentPosition(0);
  // Girar hasta tope de cierre
  stepper.moveTo(-100000); // valor grande para garantizar toque
  while (digitalRead(swcierrepin) == HIGH) {
    stepper.run();
  }
  stepper.stop();
  stepper.setCurrentPosition(0);

}
