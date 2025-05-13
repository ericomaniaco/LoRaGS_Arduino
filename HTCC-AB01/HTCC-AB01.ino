#include <Arduino.h>
//#include "heltec.h"
#include "LoRaWan_APP.h"

#define SERIAL_MONITOR_BAUD_RATE                    115200    // Baud rate
#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             10        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       11        // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,  2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         100       // Timeout en symbolos
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            4000      // Tiempo de escucha antes de reiniciar
#define PACKET_SIZE                                 48        // Define the packet size here

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


//___________________________________________________________
//  VARIABLES GLOBALES
uint8_t txPacket[PACKET_SIZE];  // Payload final tras encoding
uint8_t rxData[PACKET_SIZE];
uint8_t tcPacket[PACKET_SIZE] = {0xC8,0x9D,0x00,0x00,0x00,0x00,0x03,0x83,0x1E,0x19,0xDC,0x63,0x53,0xC4}; // Cabecera fija para cada comando
uint8_t encodedPacket[48];     // Resultado del interleave

volatile bool loraIdle = true;
static RadioEvents_t RadioEvents;

String tcInput;
int tcNumber=0;
int16_t rssi, rxSize;
int sendData_TLC_sent=0;
//___________________________________________________________

void setup() {
    Serial.begin(SERIAL_MONITOR_BAUD_RATE);
    rssi=0;

    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                            LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                            true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    RadioEvents.TxDone = OnTxDone; // standby
    RadioEvents.RxDone = OnRxDone;
}

void loop() {
  tcNumber=0;
  if (Serial.available() > 0)
  {
    tcInput=Serial.readStringUntil('\n');
    tcNumber=tcInput.toInt();
    Serial.printf("Telecomand %d\n", tcNumber);
    SendTC(tcNumber);
    delay(1800);
  
  }
  
  if(loraIdle)
  {
  	turnOffRGB();
    loraIdle = false;
    Radio.Rx(0);
  }
}

void SendTC(uint8_t TC) {
    uint8_t *txData;
    // Coloca el byte de comando (TC) en la posición 2 del array tcpacket
    tcPacket[2]=TC;
    // Dependiendo del valor de TC, va rellenando el paquete con la información correspondiente
    switch (TC) {
        case PING:
            printf("Executing: ping\n");
            break;
        case TRANSIT_TO_NM:
            printf("Executing: transit to NM\n");
            break;
        case TRANSIT_TO_CM:
            printf("Executing: transit to CM\n");
            break;
        case TRANSIT_TO_SSM:
            printf("Executing: transit to SSM\n");
            break;
        case TRANSIT_TO_SM:
            printf("Executing: transit to SM\n");
            break;
        case UPLOAD_ADCS_CALIBRATION:
            printf("Executing: upload ADCS calibration TBD\n");
            //TBD
            break;
        case UPLOAD_ADCS_TLE:
            printf("Executing: upload ADCS TLE TBD\n");
            //TBD
            break;
        case UPLOAD_COMMS_CONFIG:
            printf("Executing: upload comms config TBD\n");
            tcPacket[3]=128; // 128 == 868MHz , 255 == 915 MHz
            tcPacket[4]=11; //Spreading factor
            tcPacket[5]=1; //Coding rate
            break;
        case COMMS_UPLOAD_PARAMS:
            printf("Executing: comms upload params\n");
            tcPacket[3]=10; //Define rxtime in ms/100
            tcPacket[4]=10; //Define sleeptime in ms/100
            tcPacket[5]=64; //CADMODE --> 0 to 128==ON 128 to 255==OFF
            tcPacket[6]=255; //Packet window for Payload data sending.
            break;
        case UPLOAD_UNIX_TIME:
            printf("Executing: upload UNIX time TBD\n");
            //TBD
            break;
        case UPLOAD_EPS_TH:
            printf("Executing: upload EPS thresholds TBD\n");
            //TBD
            break;
        case UPLOAD_PL_CONFIG:
            printf("Executing: upload payload config TBD\n");
            //TBD
            break;
        case DOWNLINK_CONFIG:
            printf("Executing: downlink config TBD\n");
            //TBD
            break;
        case EPS_HEATER_ENABLE:
            printf("Executing: enable EPS heater TBD\n");
            //TBD
            break;
        case EPS_HEATER_DISABLE:
            printf("Executing: disable EPS heater TBD\n");
            //TBD
            break;
        case POL_PAYLOAD_SHUT:
            printf("Executing: shut down payload TBD\n");
            break;
        case POL_ADCS_SHUT:
            printf("Executing: shut down ADCS TBD\n");
            break;
        case POL_BURNCOMMS_SHUT:
            printf("Executing: shut down burn comms TBD\n");
            break;
        case POL_HEATER_SHUT:
            printf("Executing: shut down heater TBD\n");
            break;
        case POL_PAYLOAD_ENABLE:
            printf("Executing: enable payload TBD\n");
            break;
        case POL_ADCS_ENABLE:
            printf("Executing: enable ADCS TBD\n");
            break;
        case POL_BURNCOMMS_ENABLE:
            printf("Executing: enable burn comms TBD\n");
            break;
        case POL_HEATER_ENABLE:
            printf("Executing: enable heater TBD\n");
            break;
        case CLEAR_PL_DATA:
            printf("Executing: clear payload data TBD\n");
            break;
        case CLEAR_FLASH:
            printf("Executing: clear flash memory TBD\n");
            break;
        case CLEAR_HT:
            printf("Executing: clear housekeeping telemetry TBD\n");
            break;
        case COMMS_STOP_TX:
            printf("Executing: stop comms transmission\n");
            break;
        case COMMS_RESUME_TX:
            printf("Executing: resume comms transmission\n");
            break;
        case COMMS_IT_DOWNLINK:
            printf("Executing: initiate comms downlink\n");
            break;
        case COMMS_HT_DOWNLINK:
            printf("Executing: initiate housekeeping telemetry downlink TBD\n");
            //TBD
            break;
        case PAYLOAD_SCHEDULE:
            printf("Executing: schedule payload TBD\n");
            //TBD
            break;
        case PAYLOAD_DEACTIVATE:
            printf("Executing: deactivate payload TBD\n");
            //TBD33
            break;
        case PAYLOAD_SEND_DATA:
            printf("Executing: send payload data\n");
            break;
        case OBC_HARD_REBOOT:
            printf("Executing: hard reboot of OBC TBD\n");
            //TBD
            break;
        case OBC_SOFT_REBOOT:
            printf("Executing: soft reboot of OBC TBRD\n");
            //TBRD
            break;
        case OBC_PERIPH_REBOOT:
            printf("Executing: reboot peripherals of OBC TBD\n");
            //TBD
            break;
        case OBC_DEBUG_MODE:
            printf("Executing: enable debug mode TBD\n");
            //TBD
            break;
        default:
            printf("Unknown command\n");
            break;
    }
    // Reserva dinámicamente un buffer de bytes para el envío
    txData = (uint8_t *) malloc(PACKET_SIZE);
    if (txData == NULL) {
        exit(EXIT_FAILURE);
        Serial.println("Error en la linea 258");
    }
    // Copia el contenido de tcpacket (48 bytes) al buffer txData
    memcpy(txData,tcPacket,PACKET_SIZE);
    Serial.println();
    // Aplica la función de interleaving al buffer txData
    interleave((uint8_t*) txData, PACKET_SIZE);
    // Copia el paquete entrelazado a encodedPacket
    memcpy(encodedPacket,txData,PACKET_SIZE);
    // Libera la memoria dinámica ya que no la necesitamos más
    free(txData);
    Serial.println();
    // Envía el paquete final por el módulo de radio
    Radio.Send(encodedPacket,sizeof(encodedPacket));
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {
    int i;
    uint8_t *rxPacket;

    turnOnRGB(COLOR_RECEIVED,0);
  	
    memset(rxData,0,sizeof(rxData));
    rxPacket =(uint8_t *) malloc(size);
    if (rxPacket == NULL) {
        exit(EXIT_FAILURE);
    }

    memcpy(rxPacket,payload,size);

    deinterleave((uint8_t*) rxPacket,size);
    memcpy(rxData,rxPacket,size);

    free(rxPacket);    
    Radio.Sleep();
    for(i=0; i<size; i++){
      printHex(rxData[i]);
    }
    Serial.println();
    loraIdle = true;
}

void OnTxDone()
{
  loraIdle = true;;
}

void interleave(uint8_t *input, int size) {
    int i , j;
    int groupSize;
    uint8_t *temp;

    // Check that the size is a multiple of 6.
    if (size % 6 != 0) {
        return;
    }

    groupSize = size / 6;

    // Allocate temporary array to hold the interleaved result.
    temp =(uint8_t *) malloc(size * sizeof(uint8_t));
    if (temp == NULL) {
        exit(EXIT_FAILURE);
    }

    // For each index within the groups,
    // pick one element from each of the 6 groups.
    for (i = 0; i < groupSize; i++) {
        for (j = 0; j < 6; j++) {
            temp[i * 6 + j] = input[j * groupSize + i];
        }
    }

    // Copy the interleaved elements back into the original array.
    memcpy(input, temp, size * sizeof(uint8_t));

    free(temp);
}

void deinterleave(uint8_t *input, int size) {
    int i , j;
    int groupSize;
    uint8_t *temp;

    if (size % 6 != 0) {
        return;
    }

    groupSize = size / 6;
    temp =(uint8_t *) malloc(size * sizeof(uint8_t));
    if (temp == NULL) {
        exit(EXIT_FAILURE);
    }

    // Reconstruct the original groups.
    // For each group index i and for each group j:
    // The interleaved array holds the jth element of group j at position i*6 + j.
    // We restore it to temp[ j * groupSize + i ].
    for (i = 0; i < groupSize; i++) {
        for (j = 0; j < 6; j++) {
            temp[j * groupSize + i] = input[i * 6 + j];
        }
    }

    memcpy(input, temp, size * sizeof(uint8_t));
    free(temp);
}

void printHex(uint8_t num) {
  char hexCar[2];
  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}
