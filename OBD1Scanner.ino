  
  #include "DataStreamDefs.h"
  #include <SoftwareSerial.h>
  
  int rxControl = 13;
  int payloadOffset = 3;
  const int dataStreamLength = 44;
  unsigned int watchdogTimeout = 12000;
  
  unsigned long baudRate = 8192;
  
  unsigned char dataStream[dataStreamLength];
  
  unsigned char mode1[] = {0xF4, 0x57, 0x01, 0x00, 0xB4};
  unsigned char mode2[] = {0xF4, 0x57, 0x01, 0x01, 0xB3};
  unsigned char clearCodes[] = {0xF4, 0x56, 0x0A, 0xAC};
  
  unsigned int mode1Length = sizeof(mode1) / sizeof(mode1[0]);
  unsigned int mode2Length = sizeof(mode2) / sizeof(mode2[0]);
  unsigned int clearCodesLength = sizeof(clearCodes) / sizeof(clearCodes[0]);
  
  SoftwareSerial softSerial(2, 3); // RX, TX
  
  void setup() {
    serialInit(baudRate);
    softSerial.begin(8192);
    pinMode(rxControl, OUTPUT);
  }

  
  void loop() {
    getAldlData();
    processData();
  }
  
  
  void processData()
  {
    unsigned char ignore;
    float temp;
    
    // Calculate coolant temperature C
    temp = (float)dataStream[4 + payloadOffset] * 0.75f - 40.0f;
    softSerial.print(temp );
    softSerial.print("\t");

    // Calculate coolant temperature F
    temp = (float)dataStream[4 + payloadOffset] * 1.35f - 40.0f;
    softSerial.print(temp);
    softSerial.print("\t");
    
    // Calculate RPM
    temp = dataStream[23 + payloadOffset] * 10;
    softSerial.print(temp );
    softSerial.print("\t");

    // Calculate MPH
    temp = dataStream[9 + payloadOffset];
    softSerial.print(temp );
    softSerial.print("\t");
  
    // Calculate Throttle Position Sensor (TPS) percent
    temp = dataStream[7 + payloadOffset];
    softSerial.print(temp );
    softSerial.print("\t");
  
    // Calculate Manifold Air Pressure (MAP)
    temp = (float)(dataStream[8 + payloadOffset] * 0.5f);
    softSerial.print(temp );
    softSerial.print("\t");

    // Calculate Battery voltage
    temp = (float)dataStream[5 + payloadOffset] / 10.0f;
    softSerial.print(temp);
    softSerial.println();
  
  }

  
  void getAldlData(void) {
    unsigned int watchdog = 0;
  
    digitalWrite(rxControl, HIGH);
    serialWriteBuff(clearCodes, clearCodesLength);
    digitalWrite(rxControl, LOW);
    
    for (unsigned char i = 0; i < dataStreamLength; i++) {
      watchdog = 0;
  
      while (!(UCSR0A & (1 << RXC0)) && (watchdog < watchdogTimeout)) {
        watchdog++;
      }
  
      if (watchdog >= watchdogTimeout) {
        dataStream[i] = 0;
      } else {
        dataStream[i] = UDR0;
      }
    }
    digitalWrite(rxControl, HIGH);
  }

  
  void serialInit(unsigned long baud)
  {
    //Calculate baud register values
    uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  
    //Set baudRate registers
    UBRR0H = baud_setting >> 8;
    UBRR0L = baud_setting;
  
    //Setup 2x speed
    UCSR0A = 1 << U2X0;
  
    // Enable receive pin
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  
    // Set the USART to asynchronous at 8 bits no parity and 1 stop bits
    UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) | (0 << UPM01) | (0 << UPM00) | (0 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00) | (0 << UCPOL0);
  }

  
  void serialWriteBuff(unsigned char data[], unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
      while (!(UCSR0A & (1 << UDRE0)));
      UDR0 = data[i];
      while (!(UCSR0A & (1 << UDRE0)));
    }
  }