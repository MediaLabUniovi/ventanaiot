#include "globals.h"
#include "control.h"
#include <Arduino.h>
#include <time.h>

bool verificarRangoHorario(int horaActual, int minutoActual) {
  int minutosActuales = horaActual * 60 + minutoActual;
  int minutosInicio = horaInicio * 60 + minutoInicio;
  int minutosFin = horaFin * 60 + minutoFin;

  return (minutosActuales >= minutosInicio && minutosActuales <= minutosFin);
}

bool ComprobarHora() {
  if (!horarioActivo) {
    return true;
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo la hora");
    return false;
  }
  return verificarRangoHorario(timeinfo.tm_hour, timeinfo.tm_min);
}
