Este repositorio contiene la implementación de un sistema de envío y recepción de paquetes y telecomandos LoRa. Ambas implementaciones trabajan en escucha pasiva y transmiten el telecomando que se introduce via serial monitor.

Estructura del repositorio:

  - HTCC-AB01:
    
      Versión funcional para placas de la familia CubeCell (testeado en HTCC-AB01) utilizando la librería LoRaWan_APP.h con eventos y callbacks.
    
  - HTWL32V3:
    
      Adaptación para Heltec WiFi LoRa 32 (V3) (ESP32-S3), usando la librería RadioLib.
