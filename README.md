La Libreria Heltec_ESP32_Dev-Boards debe ser editada para el funcionamiento del programa. Se han hecho los siguientes cambios:

  1- Heltec_ESP32_Dev-Boards\src\driver\sx126x.c
  
    + #include <Arduino.h> // para delay()
    + #include <string.h>  // para memset()

  2- Heltec_ESP32_Dev-Boards\src\driver\sx126x.c
  
    - lora_printf("spi timeout\r\n");
    + log_printf("spi timeout\r\n");

  3- Heltec_ESP32_Dev-Boards\src\radio\radio.c
  
    + #include <Arduino.h>  // Para declarar delay()
