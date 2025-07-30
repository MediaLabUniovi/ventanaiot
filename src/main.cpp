/*
 Media_Lab 2025. A. Vilas
*/

//////////////////// LIBRERÍAS ////////////////////
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <AccelStepper.h>
#include <AiEsp32RotaryEncoder.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <time.h>
#include <ArduinoJson.h>
///////////////////////////////////////////////////

////////////////// CONFIGURACIÓN //////////////////

const char* ssid = "MediaLab guest";
const char* password = "medialab2019";

// Telegram
const char* botToken = "6097867978:AAFH3dlqEK6yNDej_ZDNbydNy6D-acbLDFs";
const char* vilas = "915362689";

// URLs sensores
const char* url = "https://www.medialab-uniovi.es/co2/ultimovalordelmedialabCO2.php";
const char* urltemp = "https://www.medialab-uniovi.es/co2/ultimovalordelmedialabT.php";

// valor temperatura exterior estación
const char* ttn_url = "https://eu1.cloud.thethings.network/api/v3/as/applications/estacionmeteorologica-miguel/devices/estacionmiguel/packages/storage/uplink_message?last=10m";
const char* ttn_api_key = "NNSXS.DQE2PFRZR62ZFUUGFH5Y7VTP43WNHT6KNUBONAA.LGKMGIK4FEUSOITSI5JIXXU25RUYOZ2L3KIKBQWXSAJGY2CEXCBQ";

const unsigned long BOT_MTBS = 2000;
unsigned long bot_lasttime;

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

// Variables control
float tmax = 50;
float tmin = 5;
bool  automatic = false;
int   estado = 0; // 1-cerrado, 2-abierto, 3-cerrando, 4-abriendo
bool  accionadoencoder = false;

// ---------------------
// CONFIGURACIÓN MQTT
// ---------------------
const char* mqtt_server = "ventanaiot.cloud.shiftr.io";
const int   mqtt_port = 1883;
const char* mqtt_user = "ventanaiot";              // Tu instancia
const char* mqtt_password = "4orRA8Ga2donb2x5";    // Token generado en el dashboard

WiFiClient espClient;
PubSubClient clientMQTT(espClient);

// Topics
const char* topic_cmd = "casa/ventana/cmd";
const char* topic_modo = "casa/ventana/modo";
const char* topic_estado = "casa/ventana/estado";
const char* topic_estado_modo = "casa/ventana/modo/estado";
const char* topic_horario = "casa/ventana/horario/#";
const char* topic_peso = "casa/ventana/peso/#";

// Variables de estado
int  posicionVentana = 0;
bool autoMode = false;
bool calibrar = false;

// Configuración de hora
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600; // UTC+1
const int   daylightOffset_sec = 3600; // Horario de verano

// Configuración adicional
bool horarioActivo = false;
int  horaInicio = 7;
int  minutoInicio = 0;
int  horaFin = 21;
int  minutoFin = 0;

int pesoTemperatura = 25;
int pesoCO2 = 50;
int pesoEnergia = 25;

// Pasos motor
int PASOS_180_GRADOS = 20000; //2700

////////////////// OBJETOS GLOBALES //////////////////
HTTPClient http;
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
AiEsp32RotaryEncoder rotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_STEPS);



////////////////// FUNCIONES LED //////////////////
void ledoff() {
    analogWrite(pinRojo, 0);
    analogWrite(pinVerde, 0);
    analogWrite(pinAzul, 0);
}

void ledrojo() {
    analogWrite(pinRojo, 128);
    analogWrite(pinVerde, 0);
    analogWrite(pinAzul, 0);
}

void ledverde() {
    analogWrite(pinRojo, 0);
    analogWrite(pinVerde, 128);
    analogWrite(pinAzul, 0);
}

void lednaranja() {
    analogWrite(pinRojo, 60);
    analogWrite(pinVerde, 128);
    analogWrite(pinAzul, 0);
}

Ticker led(ledoff, 15000, 1, MILLIS);  //ticker que apaga el led tras 15 segundos

void enviarnoti(std::string texto)  //funcion para enviar notificaciones debug a telegram
{
  bot.sendMessage(vilas, texto.c_str());
}

////////////////// FUNCIONES MOTOR //////////////////

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

                  /*#define IN1 2 //nocerrar
                  #define IN2 4 //nccerrar
                  #define BP1 16 //noabrir
                  #define BP2 17 //ncabrir*/

    if (pasosDestino > pasosActual) {
        // Movimiento antihorario
        Serial.println("🔄 Dirección: antihorario");
        estado = 4; //ABRIR
        digitalWrite(BP1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(BP2, HIGH);
        digitalWrite(IN1, HIGH);
        //int moverPasos = pasosDestino - pasosActual;
    } else if (pasosDestino < pasosActual) {
        // Movimiento horario
        Serial.println("🔁 Dirección: horario");
        estado = 3; //CERRAR
        digitalWrite(BP1, HIGH);
        digitalWrite(IN2, HIGH);
        digitalWrite(BP2, HIGH);
        digitalWrite(IN1, LOW);
        //int moverPasos = -(pasosActual - pasosDestino);
    } else {
        Serial.println("✅ Ya está en la posición deseada");
        return;
    }

    lednaranja();
    int moverPasos = pasosDestino - pasosActual;
    stepper.move(moverPasos); //ponemos un - para invertir la direccion moveto
    //estado = 5;  // Nuevo estado personalizado si lo necesitas

    if (!accionadoencoder) posicionVentana = *nuevaPosicion; 
     // Actualizar la posición global
}


/*void mhorario() {
    if (estado != 1) {
        estado = 3;
        Serial.println("motor moviendo horario");
        digitalWrite(BP1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(BP2, HIGH);
        digitalWrite(IN1, LOW);
        lednaranja();
        stepper.moveTo(2700);
    }
}

void mantih() {
    if (estado != 2) {
        estado = 4;
        Serial.println("motor moviendo antihorario");
        digitalWrite(BP1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(BP2, HIGH);
        digitalWrite(IN1, LOW);
        lednaranja();
        stepper.moveTo(-2700);
    }
}*/

void mstop() {
    if (estado == 3 || estado == 4) estado = 0;
    Serial.println("motor parado");
    //stepper.stop();
    //stepper.setCurrentPosition(0);  // Resetear la referencia de posición
    //stepper.moveTo(0);  // poner pasos a 0
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

////////////////// FUNCIONES DE SENSOR //////////////////
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

// Función que obtiene y devuelve la temperatura desde TTN
float obtenerTemperatura() {
  WiFiClientSecure clientt;
  clientt.setInsecure(); // Ignorar certificado SSL (inseguro pero funcional en pruebas)

  HTTPClient httpst;
  httpst.begin(clientt, ttn_url);
  httpst.addHeader("Authorization", String("Bearer ") + ttn_api_key);
  httpst.addHeader("Accept", "application/json");

  int httpCode = httpst.GET();
  float temperatura = NAN; // Valor por defecto si falla

  if (httpCode == 200) {
    String fullPayload = httpst.getString();
    int lastIndex = fullPayload.lastIndexOf("{\"result\"");

    if (lastIndex != -1) {
      String lastJson = fullPayload.substring(lastIndex);
      const size_t capacity = 6 * 1024;
      JsonDocument doc;
      //doc.garbageCollect(); // Recomendado si se reutiliza
      DeserializationError error = deserializeJson(doc, lastJson);

      if (!error) {
        temperatura = doc["result"]["uplink_message"]["decoded_payload"]["temperature"];
      }
    }
  }

  httpst.end(); // Cerrar conexión HTTP
  return temperatura;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void calcApertura();
Ticker leer(calcApertura, 15000, 0, MILLIS);  //ticker para leer periodicamente valores y estado ventana
void reiniciar();
Ticker esperar(reiniciar, 60000, 0, MILLIS);  //ticker para esperar una vez decide abrir o cerrar

void calcApertura() 
{
    int co2;
    float T_int;
    leertemp(&T_int);  // Temperatura interior
    float T_ext = obtenerTemperatura();  // Temperatura exterior

    leerco2(&co2);  // Concentración de CO2

    Serial.println("📡 Chequeo inteligente de apertura ventana.");
    Serial.print("🌡️ Interior: "); Serial.println(T_int);
    Serial.print("🌡️ Exterior: "); Serial.println(T_ext);
    Serial.print("🫁 CO2: "); Serial.println(co2);

    // --- Normalización de pesos ---
    float totalPeso = pesoTemperatura + pesoCO2 + pesoEnergia;
    float K1 = pesoTemperatura / totalPeso;
    float K2 = pesoCO2 / totalPeso;
    float K3 = pesoEnergia / totalPeso;

    // --- f: actúa solo fuera de [17, 27], y según si el exterior ayuda o no ---
    float f = 0.0;
    if (T_int < 17.0) {
        if (T_ext > T_int) {
            f = (T_ext - T_int) / 10.0;  // exterior calienta → bonifica
        } else {
            f = -(17.0 - T_int) / 10.0;  // exterior enfría más → penaliza
        }
    } else if (T_int > 27.0) {
        if (T_ext < T_int) {
            f = (T_int - T_ext) / 10.0;  // exterior enfría → bonifica
        } else {
            f = -(T_int - 27.0) / 10.0;  // exterior calienta más → penaliza
        }
    }

    // --- g: apertura inducida por alto CO2 ---
    float g = (co2 > 400) ? (co2 - 400.0) / 800.0 : 0.0;

    // --- h: intercambio energético para converger a 22°C ---
    float direccion = T_ext - T_int;
    float delta = T_int - 22.0;
    float h = ((delta < 0 && direccion > 0) || (delta > 0 && direccion < 0))
              ? -fabs(direccion) / 20.0   // bonifica si ayuda a 22
              : fabs(direccion) / 20.0;   // penaliza si aleja

    // --- Cálculo final ---
    float apertura = K1 * f + K2 * g - K3 * h;
    apertura = fmax(0.0, fmin(1.0, apertura)) * 100.0;  // Normaliza al rango [0,100]

    Serial.printf("🔧 Apertura calculada: %.1f%%\n", apertura);

    // --- Ejecutar movimiento del motor ---
    int porcentajeFinal = static_cast<int>(round(apertura));
    moverMotorPorcentaje(&porcentajeFinal);

    // --- Enviar notificación y gestionar ciclo ---
    std::string mensaje = "Ajuste automático de ventana:\n\nInterior: " + std::to_string(T_int) +
                          " ºC\nExterior: " + std::to_string(T_ext) +
                          " ºC\nCO2: " + std::to_string(co2) +
                          " ppm\nApertura: " + std::to_string(porcentajeFinal) + "%";

    enviarnoti(mensaje.c_str());
    /////////////// temporizadores //////////////
    leer.stop();
    esperar.start();
}


void reiniciar() //funcion para arrancar el chequeo ciclico una vez termina el tiempo de espera
{
    if (automatic == true) {
        leer.start();  //ticker de chequeo ciclico
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
        leer.stop(); //ticker de chequeo ciclico
    } else {
        automatic = true;
        Serial.print("modo automático activado");
        ledverde();
        leer.start();  //ticker de chequeo ciclico
    }
    led.start();  //ticker que apaga el led de referencia
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

////////////////// FUNCIONES MQTT //////////////////

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
        leer.start();  //ticker de chequeo ciclico
        Serial.println("🔁 Modo automático ACTIVADO");
      } else if (msg == "manual") {
        autoMode = false;
        automatic = false;
        ledrojo();
        leer.stop(); //ticker de chequeo ciclico
        Serial.println("✋ Modo manual ACTIVADO");
      } else {
        Serial.println("⚠️ Modo no reconocido");
      }
      led.start();  //ticker que apaga el led de referencia
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

///////////////////// CONTROL HORARIO //////////////////////////

// Función que verifica si estamos dentro del rango horario configurado
bool verificarRangoHorario(int horaActual, int minutoActual) {
  int minutosActuales = horaActual * 60 + minutoActual;
  int minutosInicio = horaInicio * 60 + minutoInicio;
  int minutosFin = horaFin * 60 + minutoFin;

  return (minutosActuales >= minutosInicio && minutosActuales <= minutosFin);
}

// Función que determina si se debe ejecutar la lógica principal
bool ComprobarHora() {
  if (!horarioActivo) {
    return true; // Si el horario está desactivado, siempre se ejecuta
  }
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo la hora");
    return false; // Si hay error al obtener hora, no se ejecuta
  }

  return verificarRangoHorario(timeinfo.tm_hour, timeinfo.tm_min);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void rotary_onButtonClick()
{
    // static unsigned long lastTimePressed = 0; // Soft debouncing
    // if (millis() - lastTimePressed < 500)
    // {   
    //     Serial.print("pulsado muy rapido no hacer nada");
    //         return;
    // }
    // lastTimePressed = millis();
    //Serial.print("boton centro encoder presionado");
    //Serial.print(millis());
    //Serial.println(" milliseconds after restart");
    //Serial.println(lastTimePressed);
    mautomatic();  //activar desactivar modo automatico
}

void rotary_loop() {
    if (rotaryEncoder.encoderChanged()) {
        int val = rotaryEncoder.readEncoder();
        Serial.print("Value: ");
        Serial.println(val);
        int abrir = 100;
        int cerrar = 0;
        if (val == 2) moverMotorPorcentaje(&abrir), accionadoencoder=true; // mantih();
        else if (val == -1) moverMotorPorcentaje(&cerrar), accionadoencoder=true; //mhorario();
        else if (val == 0 || val == 1) mstop(), accionadoencoder=true;;
    }
    if (rotaryEncoder.isEncoderButtonClicked()) rotary_onButtonClick();
}

void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

///////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  /////////////// Inicialización del sistema ///////////////
  Serial.begin(115200); // Iniciar comunicación serie para depuración

  /////////////// Configuración de pines LED ///////////////
  pinMode(pinRojo, OUTPUT);
  pinMode(pinVerde, OUTPUT);
  pinMode(pinAzul, OUTPUT);

  /////////////// Configuración de pines del motor ///////////////
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(BP1, OUTPUT);
  pinMode(BP2, OUTPUT);

  // Asegurar estado inicial bajo de control de motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(BP1, LOW);
  digitalWrite(BP2, LOW);

  /////////////// Configuración del motor paso a paso ///////////////
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Habilitar driver
  
  stepper.setPinsInverted(true, false, false); //invertir rotación
  stepper.setMaxSpeed(700);      // Velocidad máxima del stepper  300
  stepper.setAcceleration(100);   // Aceleración del stepper  50

  /////////////// Configuración de finales de carrera ///////////////
  pinMode(swaperturapin, INPUT);
  pinMode(swcierrepin, INPUT);

  /////////////// Conexión WiFi ///////////////
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Conectando a la red Wi-Fi...");

  }
  Serial.println("Conexión Wi-Fi establecida.");

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
    


