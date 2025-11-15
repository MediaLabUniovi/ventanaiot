#ifndef VENTANA_SENSORS_H
#define VENTANA_SENSORS_H

void leerco2(int* co2Ptr);
void leertemp(float* tempePtr);
float obtenerTemperatura();
void calcApertura();
void reiniciar();
void mautomatic();

// Exponer tickers para que `loop()` pueda llamarlos
extern class Ticker leer;
extern class Ticker esperar;

#endif // VENTANA_SENSORS_H
