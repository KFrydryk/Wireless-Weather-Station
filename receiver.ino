#include <SPI.h>
#include <dht.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <RH_ASK.h>

//obiekty
RH_ASK driver;
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);
dht DHT;

//definicje
#define DHT11_PIN 8
#define BUTTON1 A2
#define BUTTON2 A1

//debouncing
bool button1reading;
bool button2reading;
bool button1State;
bool button2State;
bool lastButton1State;
bool lastButton2State;
unsigned long lastDebounce1Time = 0;
unsigned long lastDebounce2Time = 0;
unsigned long debounceDelay = 50;

bool temphum = 0;
//zmienne globalne
////czujniki
float tempout;
float humout;
float errout;
float tempin;
float humin;
float errin;
// 
unsigned long t;

////zegar
int over = 0;
unsigned long timeNow = 0;
unsigned long timeLast = 0;
int startingHour = 18;
unsigned long seconds = 0;
int minutes = 54;
int hours = startingHour;
int dailyErrorFast = 0; //blad dzienny do przodu
int dailyErrorBehind = 0; // blad dzienny do tylu
bool correctedToday = 1; // czy juz dzisiaj byla wprowadzana korekta


void setup()
{
 pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  
  Serial.begin(9600); // Debugging only
  display.begin();
  display.setContrast(60);
  if (!driver.init()){
   display.println("init failed");
   delay(1000);
  }

}

void loop()
{
buttonpressed();
radiodane();
zegar();
if(millis()>t+5000)
{
  lokaldane();
  t = millis();
    temphum = !temphum;;
}       
   wyswietldane();   
   if(temphum ==1)
     printtemp();
   else
     printhum();
}

void radiodane()
{
      uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      int i;
      // Message with a good checksum received, dump it.
      float konwert[buflen-1];
      float wynik = 0;
      for(int i=1; i<buflen-1; i++)
      {
        konwert[i]=buf[i-1]-48;
        if (konwert[i]<0 || konwert[i]>9)
          konwert[i]=0;
        wynik*=10;
        wynik+=konwert[i];
      }
      if(buf[0] =='-')
        wynik*=-1;
      wynik*=0.1;


      switch (buf[buflen-1])
      {
        case 't':
        tempout = wynik;
        break;
        case 'h':
        humout = wynik;
        break;
        case 'e':
        errout = wynik;
        break;
        }      
    }
}

void lokaldane(){
  errin = senscheck();
  long tempinlokal = DHT.temperature;
  long huminlokal = DHT.humidity;
  if(tempinlokal<40 & tempinlokal>-30){
  errin = senscheck();
  tempin = tempinlokal;
  humin = huminlokal;
  }
}

int senscheck()
{
    int chk = DHT.read11(DHT11_PIN);
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
    case DHTLIB_INVALID_VALUE: 
    return 3;

    default: 
    return 6; 
    break;
  }
  }

void printtemp(){
 display.clearDisplay();
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("IN        TEMP");
  display.setTextSize(2);
  display.println(tempin, 1);
  display.setTextSize(1);
  display.println("OUT");
  display.setTextSize(2);
  if(errout==0)
  {
  display.print(tempout, 1);
  }
  else
   {
    display.print("ERR");
   display.print((int)errout);
   }
  display.setCursor(54,39);
    display.setTextSize(1);
    display.print(hours);    display.print(":");   
    if(minutes<10){display.print("0");}
    display.print(minutes);
  display.display();
}

void printhum(){
 display.clearDisplay();
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("IN         HUM");
  display.setTextSize(2);
  display.print(humin,0);
  display.println("%");
  display.setTextSize(1);
  display.println("OUT");
  display.setTextSize(2);
    if(errout==0)
  {
  display.print(humout,0);
    display.print("%");
  }
  else
   {
    display.print("ERR");
   display.print((int)errout);
   }
  display.setCursor(54,39);
    display.setTextSize(1);
    display.print(hours);    display.print(":");   
    if(minutes<10){display.print("0");}
    display.print(minutes);
  display.display();
}

void zegar(){
  Serial.println("aaa");
if(millis()<timeLast)
{
  over = 0xFFFFFFFF-timeLast;
  timeLast=0;
}
timeNow = millis(); // calkowita liczba milisekund
seconds = timeNow + over - timeLast; //liczba milisekund, ktora minela od ostatniej minuty

if (seconds >= 60000) //jesli minela minuta, to wyzeruj sekundy i dodaj jeden do liczby minut
{  
  timeLast = timeNow;
  over = 0;
  minutes = minutes + 1;
}

if (minutes == 60) //analogicznie dla zmiany godziny
{
  minutes = 0;
  hours = hours + 1; 
}
//korekcja
if (hours ==(24 - startingHour) && correctedToday == 0)
{
  delay(dailyErrorFast*1000);
  seconds = seconds + dailyErrorBehind;
  correctedToday = 1; 
}
if (hours == 24 - startingHour + 2) 
{
  correctedToday = 0; 
}
}


void wyswietldane()
{
          Serial.print("Temperatura: ");
        Serial.println(tempout);
                Serial.print("Wilgotnosc: ");
        Serial.println(humout);
                Serial.print("Kod bledu: ");
        Serial.println((int)errout);
        Serial.println("------");

                  Serial.print("Temperatura: ");
        Serial.println(tempin);
                Serial.print("Wilgotnosc: ");
        Serial.println(humin);
                Serial.print("Kod bledu: ");
        Serial.println((int)errin);
        Serial.println("--------------------------------------");
}

void buttonpressed()
{
 buttonstates();
  if(button1State == LOW & button2State == LOW)
  {
    lastButton1State = HIGH;
    delay(50);
  changetime();
  }
  else
  digitalWrite(12, LOW);
 
}

void changetime()
{
  bool finished = 0;
  int hoursmin = 0;
  int temphours = hours;
  int minutesdek = minutes/10;
  int minutesjed = minutes-minutesdek*10;


  while(finished ==0)
  {
        delay(50);
    buttonstates();

    if(button1State != lastButton1State & button1State == LOW)
      hoursmin += 1;
    switch(hoursmin)
    {
      case 0:
      if(button2State != lastButton2State & button2State == LOW)
    {
      temphours+=1;
      if (temphours>24)
        temphours =0;
    }
    break;
    case 1:
        if(button2State != lastButton2State & button2State == LOW)
    {
      minutesdek+=1;
      if (minutesdek>5)
        minutesdek =0;
    }
    break;
    
     case 2:
        if(button2State != lastButton2State & button2State == LOW)
    {
      minutesjed+=1;
      if (minutesjed>9)
        minutesjed =0;
    }
    break;
    case 3:
     hoursmin = 0;
     hours = temphours;
     minutes = 10*minutesdek+minutesjed;
     timeLast = millis();
     finished = 1;
    break;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.print("ustaw godzine");
    display.setTextSize(2);
    display.setCursor(13,15);
    display.print(temphours);
    display.print(":");
    display.print(minutesdek);
    display.println(minutesjed);
    display.setCursor(14,27);
    switch(hoursmin){
    case 0:
    if(temphours>9)
     display.print("--");
     else
      display.print("-");
    break;
    case 1:
    if(temphours>9)
     display.print("   -");
     else
      display.print("  -");
    break;
    case 2:
        if(temphours>9)
     display.print("    -");
     else
      display.print("   -");
    break;
  }
  display.display();
  }

}

void buttonstates()
{
button1reading = digitalRead(BUTTON1);
 button2reading = digitalRead(BUTTON2); 
 //debouncing button1
 if (button1reading != lastButton1State) 
 {
  lastDebounce1Time = millis();
 }
  if ((millis() - lastDebounce1Time) > debounceDelay) {
    if (button1reading != button1State) {
      button1State = button1reading;
   }
 }
 //debouncing button2
  if (button2reading != lastButton2State) 
 {
  lastDebounce2Time = millis();
 }
  if ((millis() - lastDebounce2Time) > debounceDelay) {
    if (button2reading != button2State) {
      button2State = button2reading;
   }
 }
lastButton1State = button1reading;
lastButton2State = button2reading;
}
