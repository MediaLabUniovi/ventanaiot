#ifndef VENTANA_CONFIG_H
#define VENTANA_CONFIG_H

/* Configuración general y pines */
// WiFi (defaults)
// You can override these values in `include/secrets.h` (ignored)
extern const char* ssid;
extern const char* password;

// Telegram (defaults/externs)
extern const char* botToken;
extern const char* vilas;

// URLs sensores
const char* url = "https://www.medialab-uniovi.es/co2/ultimovalordelmedialabCO2.php";
const char* urltemp = "https://www.medialab-uniovi.es/co2/ultimovalordelmedialabT.php";

// TTN
const char* ttn_url = "https://eu1.cloud.thethings.network/api/v3/as/applications/estacionmeteorologica-miguel/devices/estacionmiguel/packages/storage/uplink_message?last=10m";
extern const char* ttn_api_key;

const unsigned long BOT_MTBS = 2000;

// Pines encoder
#define ROTARY_ENCODER_A_PIN 25 //clk
#define ROTARY_ENCODER_B_PIN 33 //dt
#define ROTARY_ENCODER_BUTTON_PIN 32 //sw
#define ROTARY_ENCODER_STEPS 4

// Pines motor DC
#define ENA 19
#define IN1 2 //nocerrar
#define IN2 4 //nccerrar
#define BP1 16 //noabrir
#define BP2 17 //ncabrir

// Pines stepper
#define STEP_PIN 5
#define DIR_PIN 18
#define ENABLE_PIN 15 //libre aux

// Pines LED RGB
const int pinVerde = 26;
const int pinRojo = 27;
const int pinAzul = 12;

// Pines finales de carrera
#define swaperturapin 35
#define swcierrepin 34

// Configuración MQTT
const char* mqtt_server = "ventanaiot.cloud.shiftr.io";
const int   mqtt_port = 1883;
extern const char* mqtt_user;
extern const char* mqtt_password;

// Topics MQTT
const char* topic_cmd = "casa/ventana/cmd";
const char* topic_modo = "casa/ventana/modo";
const char* topic_estado = "casa/ventana/estado";
const char* topic_estado_modo = "casa/ventana/modo/estado";
const char* topic_horario = "casa/ventana/horario/#";
const char* topic_peso = "casa/ventana/peso/#";

// NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600; // UTC+1
const int   daylightOffset_sec = 3600; // Horario de verano

#endif // VENTANA_CONFIG_H
