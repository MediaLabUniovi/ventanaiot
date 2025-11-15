#include "config.h"
#include "globals.h"
#include "mqtt.h"
#include <Arduino.h>

void publicarEstado() {
    clientMQTT.publish(topic_estado, String(posicionVentana).c_str(), true);
    clientMQTT.publish(topic_estado_modo, autoMode ? "auto" : "manual", true);
}
  
void mostrarConfiguracion() {
    Serial.println("📦 Configuración recibida:");
    Serial.printf("  Modo: %s\n", autoMode ? "AUTO" : "MANUAL");
    Serial.printf("  Horario activo: %s\n", horarioActivo ? "Sí" : "No");
    Serial.printf("  Horario: %02d:%02d - %02d:%02d\n", horaInicio, minutoInicio, horaFin, minutoFin);
    Serial.printf("  Pesos - Temperatura: %d | CO2: %d | Energía: %d\n", pesoTemperatura, pesoCO2, pesoEnergia);
}
  
void callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
  
    Serial.printf("📥 Topic: %s | Payload: %s\n", topic, msg.c_str());
  
    String t = String(topic);
  
    if (t == topic_cmd) {
      int valor = msg.toInt();

      if (valor >= 0 && valor <= 100) {
        Serial.printf("🪟 Nueva posición: %d%%\n", valor);
        moverMotorPorcentaje(&valor);
        publicarEstado();
      } else {
        Serial.println("⚠️ Valor de posición fuera de rango");
      }
    
    } else if (t == topic_modo) {
      if (msg == "auto") {
        autoMode = true;
        automatic = true;
        ledverde();
        leer.start();
        Serial.println("🔁 Modo automático ACTIVADO");
      } else if (msg == "manual") {
        autoMode = false;
        automatic = false;
        ledrojo();
        leer.stop();
        Serial.println("✋ Modo manual ACTIVADO");
      } else {
        Serial.println("⚠️ Modo no reconocido");
      }
      led.start();
      publicarEstado();

    } else if (t.endsWith("horario/enabled")) {
      horarioActivo = (msg == "true");
    } else if (t.endsWith("horario/inicio")) {
      horaInicio = msg.substring(0, 2).toInt();
      minutoInicio = msg.substring(3, 5).toInt();
    } else if (t.endsWith("horario/fin")) {
      horaFin = msg.substring(0, 2).toInt();
      minutoFin = msg.substring(3, 5).toInt();
    } else if (t.endsWith("peso/temperatura")) {
      pesoTemperatura = msg.toInt();
    } else if (t.endsWith("peso/co2")) {
      pesoCO2 = msg.toInt();
    } else if (t.endsWith("peso/energia")) {
      pesoEnergia = msg.toInt();
    }
  
    mostrarConfiguracion();
}
  
void reconnect() {
    while (!clientMQTT.connected()) {
      Serial.print("🔄 Conectando a MQTT... ");
      bot.sendSimpleMessage(vilas, "🔄 Conectando a MQTT... ", "");
      String clientId = "ESP32Client-" + String(random(0xffff), HEX);
      if (clientMQTT.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
        Serial.println("✅ Conectado a MQTT");
        bot.sendSimpleMessage(vilas, "✅ Conectado a MQTT", "");
  
        clientMQTT.subscribe(topic_cmd);
        clientMQTT.subscribe(topic_modo);
        clientMQTT.subscribe(topic_horario);
        clientMQTT.subscribe(topic_peso);
  
        publicarEstado();
      } else {
        Serial.print("❌ Fallo. Código: ");
        bot.sendSimpleMessage(vilas, "❌ ERROR conexión MQTT.", "");
        Serial.println(clientMQTT.state());
        delay(5000);
      }
    }
}
