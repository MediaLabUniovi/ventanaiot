#ifndef VENTANA_MQTT_H
#define VENTANA_MQTT_H

void publicarEstado();
void mostrarConfiguracion();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

#endif // VENTANA_MQTT_H
