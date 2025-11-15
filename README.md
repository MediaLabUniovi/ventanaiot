# Sistema embebido para apertura automatizada de ventanas

Proyecto TFG – Media_Lab 2025
Autor: A. Vilas

Implementación de la lógica principal de un sistema de ventilación natural mediante control de un motor paso a paso. El algoritmo calcula el grado de apertura óptimo en función de la temperatura interior, temperatura exterior y concentración de CO₂.

## Contenido
- Lectura de sensores ambientales
- Control de motor paso a paso
- Comunicación MQTT con aplicación móvil
- Gestión de seguridad mediante finales de carrera

## Requisitos
- Placa ESP32 compatible (p. ej. NodeMCU-32S)
- PlatformIO (VS Code o CLI)
- Broker MQTT accesible
- Token de Telegram Bot (opcional para notificaciones)
- (Opcional) TTN API key para temperatura exterior

## Pasos mínimos para desplegar

1. Copia la plantilla de secretos y edítala:

```bash
cp include/secrets.example.h include/secrets.h
```

2. Edita `include/secrets.h` y completa las credenciales:
- `ssid` y `password` (Wi‑Fi)
- `botToken` y `vilas` (Telegram)
- `ttn_api_key` (si usas TTN)
- `mqtt_user` y `mqtt_password` (si el broker lo requiere)

3. Si necesitas cambiar host/puerto MQTT, edita `include/config.h`:
- `mqtt_server` y `mqtt_port` (por defecto `ventanaiot.cloud.shiftr.io:1883`)

4. Compilar y subir con PlatformIO:

```bash
cd /ruta/al/proyecto
pio run
pio run -t upload
```

5. Monitor serie para verificar arranque y conexiones:

```bash
pio device monitor
```

## Topics MQTT por defecto
- Comando posición: `casa/ventana/cmd`
- Modo: `casa/ventana/modo`
- Estado: `casa/ventana/estado`

## Buenas prácticas
- No subas `include/secrets.h` al repositorio (está en `.gitignore`).
- Usa `include/secrets.example.h` como plantilla.

