//todo wgrac z dobrymi pinami zasilania
#include <dht.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <avr/sleep.h>
#include <avr/wdt.h>

/*
 * DHT ---------Atmega328
 * VCC 1 -------4(DP2)
 * DATA 2 ------11 (DP5)
 * GND 4 -------GND
 * 
 * RF433 -------Atmega328
 * ADAT 1-------18(DP12)
 * VCC 2--------5(DP3)
 * GND----------GND
 */

RH_ASK driver;
dht DHT;
#define DHT22_PIN 5
#define DHTPWR 6
#define RFPWR 13
//rf433 adat pin = 12
int cnt = 0; //licznik cykli snu

float temp;
float hum;
int err = 0;

// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect


void setup()
{
  driver.init();
   // if (!driver.init())
   //      Serial.println("init failed");
    pinMode(DHTPWR, OUTPUT); //zasilanie dht22
    pinMode(RFPWR, OUTPUT);
    for (byte i = 0; i <= A5; i++)
    {
    pinMode (i, OUTPUT);    // changed as per below
    digitalWrite (i, LOW);  //     ditto
    }
}

void loop()
{

  if(cnt<1)
cnt++;
else
{
  digitalWrite(DHTPWR, HIGH);
    digitalWrite(RFPWR, HIGH);
      delay(2000);
   program();
   cnt = 0;
}
 // disable ADC
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    //  set WDIE, and 1 second delay; WDP3 WDP0 = 8s
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();
}



void program()
{
err = senscheck();
temp = DHT.temperature;
hum = DHT.humidity;


//konwersja float na char
char tempBuffer[10];
 char *tempchar;
tempchar = dtostrf(temp*100, 6, 0, tempBuffer);
char humBuffer[10];
 char *humchar;
humchar = dtostrf(hum*100, 6, 0, humBuffer);
char errBuffer[10];
 char *errchar;
errchar = dtostrf(err*100, 1, 0, errBuffer);

  //wysłanie bledu
    char* msg = new char[sizeof(errchar)+1];
    strcpy(msg, errchar);
    strcat(msg, "e");
    Serial.println(msg);  
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent(); 
    delete[] msg;
    delay(100);
  //wysłanie temperatura
    msg = new char[sizeof(tempchar)+1];
    strcpy(msg, tempchar);
    strcat(msg, "t");
    Serial.println(msg);  
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    delete[] msg;delay(100);
  //wysłanie wilgotności
    msg = new char[sizeof(humchar)+1];
    strcpy(msg, humchar);
    strcat(msg, "h");
    Serial.println(msg);  
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    delete[] msg;
  digitalWrite(DHTPWR, LOW);
    digitalWrite(RFPWR, LOW);
}




int senscheck()
{
    int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
    return 0;
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    return 1;
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    return 2;
    break;
    case DHTLIB_ERROR_CONNECT: 
    return 3;
    break;
    case DHTLIB_ERROR_ACK_L: 
    return 4;
    break;
    case DHTLIB_ERROR_ACK_H: 
    return 5;
    break;


    
    default: 
    return 6; 
    break;
  }
  }
