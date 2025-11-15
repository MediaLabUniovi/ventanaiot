#include "config.h"
#include "globals.h"

HTTPClient http;
WiFiClientSecure client;
WiFiClient espClient;
UniversalTelegramBot bot(botToken, client);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
AiEsp32RotaryEncoder rotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_STEPS);
PubSubClient clientMQTT(espClient);

int posicionVentana = 0;
bool autoMode = false;
bool calibrar = false;
int PASOS_180_GRADOS = 20000;
bool automatic = false;
int estado = 0; // 1-cerrado, 2-abierto, 3-cerrando, 4-abriendo
bool accionadoencoder = false;

// Pesos y horario (valores iniciales)
int pesoTemperatura = 25;
int pesoCO2 = 50;
int pesoEnergia = 25;

bool horarioActivo = false;
int horaInicio = 7;
int minutoInicio = 0;
int horaFin = 21;
int minutoFin = 0;
