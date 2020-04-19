
#include <dht.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
float a = -123.123;
RH_ASK driver;
dht DHT;
#define DHT22_PIN 5

float temp;
float hum;
int err = 0;

void setup()
{
    Serial.begin(9600);   // Debugging only
    if (!driver.init())
         Serial.println("init failed");
         pinMode(9, OUTPUT);
}

void loop()
{
  digitalWrite(9, HIGH);
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
  digitalWrite(9, LOW);
    delay(5000);
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
    default: 
    return 3; 
    break;
  }
  }
