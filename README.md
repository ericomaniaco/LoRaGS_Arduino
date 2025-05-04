Este repositorio contiene la implementación de un sistema de envío de telecomandos LoRa

Estructura del repositorio:

  - HTCC-AB01_NoBlock:
      Versión funcional para placas de la familia CubeCell (testeado en HTCC-AB01) utilizando la librería LoRaWan_APP.h con eventos y callbacks.
      Esto significa que el programa no bloquea el MCU mientras esta ejecutando la transmisión o recepción de telecomandos.
    
  - HWL32V3_Block:
      Adaptación para Heltec WiFi LoRa 32 (V3) (ESP32-S3), usando la librería RadioLib en modo bloqueante.
      Esto significa que mientras se estan transmitiendo o recibiendo el MCU esta bloqueado y no puedo ejecutar otras acciones.
