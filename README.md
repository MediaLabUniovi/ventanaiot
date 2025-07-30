Sistema embebido para apertura automatizada de ventanas  
Proyecto TFG – Media_Lab 2025  
Autor: A. Vilas

Implementación de la lógica principal de un sistema de ventilación natural
mediante control de un motor paso a paso. El algoritmo calcula el grado de
apertura óptimo en función de la temperatura interior, temperatura exterior
y concentración de CO₂.

El firmware incluye:
- Lectura de sensores ambientales
- Control de motor paso a paso
- Comunicación MQTT con aplicación móvil
- Gestión de seguridad mediante finales de carrera

Requiere plataforma ESP32, protocolo MQTT y periféricos descritos en la memoria del TFG.
