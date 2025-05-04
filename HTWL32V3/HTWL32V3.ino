#include <Arduino.h>
#include <RadioLib.h>

//=============== Parámetros de configuración ===============
#define SERIAL_MONITOR_BR 115200
#define RF_FREQUENCY      868.0   // MHz [150.0 to 960.0]
#define TX_OUTPUT_POWER   10      // dBm
#define LORA_BANDWIDTH    125.0   // Allowed values in kHz:
                                  // [7.8, 10.4, 15.6, 20.8, 31.25, 
                                  //  41.7, 62.5, 125.0, 250.0, 500.0]
#define LORA_SPREADING_FACTOR 11  // [SF7..SF12]
#define LORA_CODING_RATE  5       // [4/5, 4/6, 4/7, 4/8]
#define PACKET_SIZE       48

// Configuración de pines para Heltec WiFi LoRa 32 V3
#define LORA_CS    8   // NSS
#define LORA_RST   12  // Reset
#define LORA_BUSY  13  // Busy
#define LORA_DIO1  14  // DIO1
#define LORA_SCK   9   // SCK
#define LORA_MISO 11   // MISO
#define LORA_MOSI 10   // MOSI

// Crear instancia del módulo SX1262
SX1262 lora = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Definición de telecomandos
typedef enum {
    ERR,                      // 0
    PING,                     // 1
    TRANSIT_TO_NM,            // 2
    TRANSIT_TO_CM,            // 3
    TRANSIT_TO_SSM,           // 4
    TRANSIT_TO_SM,            // 5
    UPLOAD_ADCS_CALIBRATION,  // 6
    UPLOAD_ADCS_TLE,          // 7
    UPLOAD_COMMS_CONFIG,      // 8
    COMMS_UPLOAD_PARAMS,      // 9
    UPLOAD_UNIX_TIME,         // 10
    UPLOAD_EPS_TH,            // 11
    UPLOAD_PL_CONFIG,         // 12
    DOWNLINK_CONFIG,          // 13
    EPS_HEATER_ENABLE,        // 14
    EPS_HEATER_DISABLE,       // 15
    POL_PAYLOAD_SHUT,         // 16
    POL_ADCS_SHUT,            // 17
    POL_BURNCOMMS_SHUT,       // 18
    POL_HEATER_SHUT,          // 19
    POL_PAYLOAD_ENABLE,       // 20
    POL_ADCS_ENABLE,          // 21
    POL_BURNCOMMS_ENABLE,     // 22
    POL_HEATER_ENABLE,        // 23
    CLEAR_PL_DATA,            // 24
    CLEAR_FLASH,              // 25
    CLEAR_HT,                 // 26
    COMMS_STOP_TX,            // 27
    COMMS_RESUME_TX,          // 28
    COMMS_IT_DOWNLINK,        // 29
    COMMS_HT_DOWNLINK,        // 30
    PAYLOAD_SCHEDULE,         // 31
    PAYLOAD_DEACTIVATE,       // 32
    PAYLOAD_SEND_DATA,        // 33
    OBC_HARD_REBOOT,          // 34
    OBC_SOFT_REBOOT,          // 35
    OBC_PERIPH_REBOOT,        // 36
    OBC_DEBUG_MODE            // 37
} telecommandIDS;

//================= VARIABLES GLOBALES =================
uint8_t txPacket[PACKET_SIZE];
uint8_t rxPacket[PACKET_SIZE];
uint8_t tcPacket[PACKET_SIZE] = {
  0xC8, 0x9D, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x83, 0x1E, 0x19, 0xDC, 0x63,
  0x53, 0xC4
}; // Cabecera fija para cada comando
uint8_t encodedPacket[PACKET_SIZE];

String tcInput;
int tcNumber = 0;


//===================== FUNCIONES =====================

void setup() {
  Serial.begin(SERIAL_MONITOR_BR);
  delay(2000);
  Serial.println("Inicializando SX1262...");

  // Inicializar el módulo LoRa
  int state = lora.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SX1262 inicializado correctamente.");
  } else {
    Serial.print("Error al inicializar SX1262, código: ");
    Serial.println(state);
    while (true);
  }

  // Configurar parámetros de transmisión
  lora.setFrequency(RF_FREQUENCY);
  lora.setBandwidth(LORA_BANDWIDTH);
  lora.setSpreadingFactor(LORA_SPREADING_FACTOR);
  lora.setCodingRate(LORA_CODING_RATE);
  lora.setOutputPower(TX_OUTPUT_POWER);
  lora.setCRC(true);

  Serial.println("Listo para recibir comandos. Introduce número:");
}

void loop() {
  // Verificar si se ha introducido un comando por el serial
  if (Serial.available() > 0) {
    tcInput = Serial.readStringUntil('\n');
    tcNumber = tcInput.toInt();
    Serial.printf("Telecomando recibido: %d\n", tcNumber);
    SendTC(tcNumber);
  }

  // Recepción pasiva constante
  uint8_t tempBuf[PACKET_SIZE];
  int state = lora.receive(tempBuf, PACKET_SIZE);

  // Obtener el tamaño real del paquete recibido
  if (state == RADIOLIB_ERR_NONE) {
    int len = lora.getPacketLength();
    memcpy(rxPacket, tempBuf, len);
    deinterleave(rxPacket, len);

    Serial.print("Paquete recibido: ");
    for (int i = 0; i < len; i++) {
      printHex(rxPacket[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

//=============== FUNCIONES AUXILIARES ===============

// Función para imprimir en hexadecimal
void printHex(uint8_t num) {
  char hexCar[3];
  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

// Función de entrelazado
void interleave(uint8_t *input, int size) {
  if (size % 6 != 0) return;
  int groupSize = size / 6;
  uint8_t temp[size];
  for (int i = 0; i < groupSize; i++) {
    for (int j = 0; j < 6; j++) {
      temp[i * 6 + j] = input[j * groupSize + i];
    }
  }
  memcpy(input, temp, size);
}

// Función de desenlazado
void deinterleave(uint8_t *input, int size) {
  if (size % 6 != 0) return;
  int groupSize = size / 6;
  uint8_t temp[size];
  for (int i = 0; i < groupSize; i++) {
    for (int j = 0; j < 6; j++) {
      temp[j * groupSize + i] = input[i * 6 + j];
    }
  }
  memcpy(input, temp, size);
}

// Función para enviar el comando
void SendTC(uint8_t TC) {
  tcPacket[2] = TC;
  switch (TC) {
    case PING:
      Serial.println("Executing: ping");
      break;
    case TRANSIT_TO_NM:
      Serial.println("Executing: transit to NM");
      break;
    case TRANSIT_TO_CM:
      Serial.println("Executing: transit to CM");
      break;
    case TRANSIT_TO_SSM:
      Serial.println("Executing: transit to SSM");
      break;
    case TRANSIT_TO_SM:
      Serial.println("Executing: transit to SM");
      break;
    case UPLOAD_ADCS_CALIBRATION:
      Serial.println("Executing: upload ADCS calibration TBD");
      break;
    case UPLOAD_ADCS_TLE:
      Serial.println("Executing: upload ADCS TLE TBD");
      break;
    case UPLOAD_COMMS_CONFIG:
      Serial.println("Executing: upload comms config TBD");
      tcPacket[3] = 128; // 128 == 868MHz
      tcPacket[4] = 11;  // Spreading factor
      tcPacket[5] = 1;   // Coding rate
      break;
    case COMMS_UPLOAD_PARAMS:
      Serial.println("Executing: comms upload params");
      tcPacket[3] = 10;  // rxtime in ms/100
      tcPacket[4] = 10;  // sleeptime in ms/100
      tcPacket[5] = 64;  // CADMODE
      tcPacket[6] = 255; // Packet window
      break;
    case UPLOAD_UNIX_TIME:
      Serial.println("Executing: upload UNIX time TBD");
      break;
    case UPLOAD_EPS_TH:
      Serial.println("Executing: upload EPS thresholds TBD");
      break;
    case UPLOAD_PL_CONFIG:
      Serial.println("Executing: upload payload config TBD");
      break;
    case DOWNLINK_CONFIG:
      Serial.println("Executing: downlink config TBD");
      break;
    case EPS_HEATER_ENABLE:
      Serial.println("Executing: enable EPS heater TBD");
      break;
    case EPS_HEATER_DISABLE:
      Serial.println("Executing: disable EPS heater TBD");
      break;
    case POL_PAYLOAD_SHUT:
      Serial.println("Executing: shut down payload TBD");
      break;
    case POL_ADCS_SHUT:
      Serial.println("Executing: shut down ADCS TBD");
      break;
    case POL_BURNCOMMS_SHUT:
      Serial.println("Executing: shut down burn comms TBD");
      break;
    case POL_HEATER_SHUT:
      Serial.println("Executing: shut down heater TBD");
      break;
    case POL_PAYLOAD_ENABLE:
      Serial.println("Executing: enable payload TBD");
      break;
    case POL_ADCS_ENABLE:
      Serial.println("Executing: enable ADCS TBD");
      break;
    case POL_BURNCOMMS_ENABLE:
      Serial.println("Executing: enable burn comms TBD");
      break;
    case POL_HEATER_ENABLE:
      Serial.println("Executing: enable heater TBD");
      break;
    case CLEAR_PL_DATA:
      Serial.println("Executing: clear payload data TBD");
      break;
    case CLEAR_FLASH:
      Serial.println("Executing: clear flash memory TBD");
      break;
    case CLEAR_HT:
      Serial.println("Executing: clear housekeeping telemetry TBD");
      break;
    case COMMS_STOP_TX:
      Serial.println("Executing: stop comms transmission");
      break;
    case COMMS_RESUME_TX:
      Serial.println("Executing: resume comms transmission");
      break;
    case COMMS_IT_DOWNLINK:
      Serial.println("Executing: initiate comms downlink");
      break;
    case COMMS_HT_DOWNLINK:
      Serial.println("Executing: initiate housekeeping telemetry downlink TBD");
      break;
    case PAYLOAD_SCHEDULE:
      Serial.println("Executing: schedule payload TBD");
      break;
    case PAYLOAD_DEACTIVATE:
      Serial.println("Executing: deactivate payload TBD");
      break;
    case PAYLOAD_SEND_DATA:
      Serial.println("Executing: send payload data");
      break;
    case OBC_HARD_REBOOT:
      Serial.println("Executing: hard reboot of OBC TBD");
      break;
    case OBC_SOFT_REBOOT:
      Serial.println("Executing: soft reboot of OBC TBD");
      break;
    case OBC_PERIPH_REBOOT:
      Serial.println("Executing: reboot peripherals of OBC TBD");
      break;
    case OBC_DEBUG_MODE:
      Serial.println("Executing: enable debug mode TBD");
      break;
    default:
      Serial.println("Unknown command");
      break;
  }
  // Preparar el paquete
  memcpy(txPacket, tcPacket, PACKET_SIZE);
  interleave(txPacket, PACKET_SIZE);
  memcpy(encodedPacket, txPacket, PACKET_SIZE);

  // Enviar el paquete
  int state = lora.transmit(encodedPacket, PACKET_SIZE);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Paquete enviado correctamente.");
  } else {
    Serial.print("Error al enviar paquete, código: ");
    Serial.println(state);
  }
}
