#include "config.h"
#include "globals.h"
#include "sensors.h"
#include "leds.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void calcApertura();
void reiniciar();

void leerco2(int* co2Ptr) {
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        *co2Ptr = std::stoi(http.getString().c_str());
    } else {
        Serial.print("Error HTTP CO2: ");
        Serial.println(httpCode);
    }
    http.end();
}

void leertemp(float* tempePtr) {
    http.begin(urltemp);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        *tempePtr = http.getString().toFloat();
    } else {
        Serial.print("Error HTTP Temp: ");
        Serial.println(httpCode);
    }
    http.end();
}

float obtenerTemperatura() {
  WiFiClientSecure clientt;
  clientt.setInsecure();

  HTTPClient httpst;
  httpst.begin(clientt, ttn_url);
  httpst.addHeader("Authorization", String("Bearer ") + ttn_api_key);
  httpst.addHeader("Accept", "application/json");

  int httpCode = httpst.GET();
  float temperatura = NAN;

  if (httpCode == 200) {
    String fullPayload = httpst.getString();
    int lastIndex = fullPayload.lastIndexOf("{\"result\"");

    if (lastIndex != -1) {
      String lastJson = fullPayload.substring(lastIndex);
      const size_t capacity = 6 * 1024;
      StaticJsonDocument<6*1024> doc;
      DeserializationError error = deserializeJson(doc, lastJson);

      if (!error) {
        temperatura = doc["result"]["uplink_message"]["decoded_payload"]["temperature"];
      }
    }
  }

  httpst.end();
  return temperatura;
}

void calcApertura() 
{
    int co2;
    float T_int;
    leertemp(&T_int);
    float T_ext = obtenerTemperatura();

    leerco2(&co2);

    Serial.println("📡 Chequeo inteligente de apertura ventana.");
    Serial.print("🌡️ Interior: "); Serial.println(T_int);
    Serial.print("🌡️ Exterior: "); Serial.println(T_ext);
    Serial.print("🫁 CO2: "); Serial.println(co2);

    float totalPeso = pesoTemperatura + pesoCO2 + pesoEnergia;
    float K1 = pesoTemperatura / totalPeso;
    float K2 = pesoCO2 / totalPeso;
    float K3 = pesoEnergia / totalPeso;

    float f = 0.0;
    if (T_int < 17.0) {
        if (T_ext > T_int) {
            f = (T_ext - T_int) / 10.0;
        } else {
            f = -(17.0 - T_int) / 10.0;
        }
    } else if (T_int > 27.0) {
        if (T_ext < T_int) {
            f = (T_int - T_ext) / 10.0;
        } else {
            f = -(T_int - 27.0) / 10.0;
        }
    }

    float g = (co2 > 400) ? (co2 - 400.0) / 800.0 : 0.0;

    float direccion = T_ext - T_int;
    float delta = T_int - 22.0;
    float h = ((delta < 0 && direccion > 0) || (delta > 0 && direccion < 0))
              ? -fabs(direccion) / 20.0
              : fabs(direccion) / 20.0;

    float apertura = K1 * f + K2 * g - K3 * h;
    apertura = fmax(0.0, fmin(1.0, apertura)) * 100.0;

    Serial.printf("🔧 Apertura calculada: %.1f%%\n", apertura);

    int porcentajeFinal = static_cast<int>(round(apertura));
    moverMotorPorcentaje(&porcentajeFinal);

    std::string mensaje = "Ajuste automático de ventana:\n\nInterior: " + std::to_string(T_int) +
                          " ºC\nExterior: " + std::to_string(T_ext) +
                          " ºC\nCO2: " + std::to_string(co2) +
                          " ppm\nApertura: " + std::to_string(porcentajeFinal) + "%";

    enviarnoti(mensaje);

    leer.stop();
    esperar.start();
}

void reiniciar()
{
    if (automatic == true) {
        leer.start();
    }
    esperar.stop();
    bot.sendSimpleMessage(vilas, "Espera terminada", "");
}

void mautomatic() 
{   
    if (automatic == true){
        automatic = false;
        Serial.print("modo automático desactivado");
        ledrojo();
        leer.stop();
    } else {
        automatic = true;
        Serial.print("modo automático activado");
        ledverde();
        leer.start();
    }
    led.start();
}

Ticker leer(calcApertura, 15000, 0, MILLIS);
Ticker esperar(reiniciar, 60000, 0, MILLIS);
