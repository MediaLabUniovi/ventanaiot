#include "globals.h"
#include "encoder.h"
#include "motor.h"
#include "sensors.h"
#include <Arduino.h>

void rotary_onButtonClick()
{
    mautomatic();
}

void rotary_loop() {
    if (rotaryEncoder.encoderChanged()) {
        int val = rotaryEncoder.readEncoder();
        Serial.print("Value: ");
        Serial.println(val);
        int abrir = 100;
        int cerrar = 0;
        if (val == 2) moverMotorPorcentaje(&abrir), accionadoencoder=true;
        else if (val == -1) moverMotorPorcentaje(&cerrar), accionadoencoder=true;
        else if (val == 0 || val == 1) mstop(), accionadoencoder=true;
    }
    if (rotaryEncoder.isEncoderButtonClicked()) rotary_onButtonClick();
}

void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}
