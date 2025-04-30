La Libreria Heltec_ESP32_Dev-Boards se añade ya que para el funcionamiento del programa se han tenido que hacer lo siguientes cambios:

  1- Heltec_ESP32_Dev-Boards\src\driver\sx126x.c
    + #include <Arduino.h> // para delay()
    + #include <string.h>  // para memset()

  2- Heltec_ESP32_Dev-Boards\src\driver\sx126x.c
    - lora_printf("spi timeout\r\n");
    + log_printf("spi timeout\r\n");

  3- Heltec_ESP32_Dev-Boards\src\radio\radio.c
    + #include <Arduino.h>  // Para declarar delay()
