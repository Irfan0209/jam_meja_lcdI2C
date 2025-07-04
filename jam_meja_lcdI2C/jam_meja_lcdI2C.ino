
#include <Wire.h>
#include <RtcDS3231.h>
#include <LiquidCrystal_I2C.h>
#include "TimeLib.h"

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>

#include "PrayerTimes.h"

#define BUZZ  D4 // PIN BUZZER

//create object
LiquidCrystal_I2C lcd(0x27, 16, 2);
//RtcDS3231<TwoWire> Rtc(Wire);
//RtcDateTime now;
double times[sizeof(TimeName)/sizeof(char*)];

const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
NTPClient Clock(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

struct Config {
  uint8_t chijir;
  uint8_t durasiadzan;
  uint8_t ihti;
  float latitude = -7.364057;
  float longitude = 112.646222;
  uint8_t zonawaktu = 7;
};

struct TanggalDanWaktu
{
  uint8_t detik;
  uint8_t menit;
  uint8_t jam;
  uint8_t hari;
  uint8_t tanggal;
  uint8_t bulan;
  uint8_t tahun;
};

struct Tanggal
{
  uint8_t tanggal;
  uint8_t bulan;
  uint16_t tahun;
};

struct TanggalJawa
{
  uint8_t pasaran;
  uint8_t wuku;
  uint8_t tanggal;
  uint8_t bulan;
  uint16_t tahun;
};

struct JamDanMenit
{
  uint8_t jam;
  uint8_t menit;
};

TanggalDanWaktu tanggalMasehi;
Tanggal tanggalHijriah;
TanggalJawa tanggalJawa;
JamDanMenit waktuMagrib;
Config config;

uint8_t dataIhty[]      = {3,0,3,3,0,3,2};
uint8_t   sholatNow     = -1;
bool      reset_x       = 0; 
uint8_t   suhu          = 30; 
bool      adzan         = 0;
bool      stateBuzzer   = 1;

enum Show{
  ANIM_HOME,
  ANIM_ADZAN,
  ANIM_NOTIF_CON
};

Show show = ANIM_HOME;


void setup() {
  Serial.begin(115200);
  pinMode(BUZZ,OUTPUT);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(1,0);
  dwCtr(0,"CONNECTING....");
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("JAM JWS"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } 
  else {
    Clock.begin();//NTP
    Clock.update();
    setTime(Clock.getHours(),Clock.getMinutes(),Clock.getSeconds(),13,04,2025); 
    Serial.println("connected...yeey :)");
  }
  
//   int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
//    if (rtn != 0) {
//      Serial.println(F("I2C bus error. Could not clear"));
//      if (rtn == 1) {
//        Serial.println(F("SCL clock line held low"));
//      } else if (rtn == 2) {
//        Serial.println(F("SCL clock line held low by slave clock stretch"));
//      } else if (rtn == 3) {
//        Serial.println(F("SDA data line held low"));
//      }
//    } 
//    else { // bus clear, re-enable Wire, now can start Wire Arduino master
//      Wire.begin();
//    }
//  
//  Rtc.Begin();
//  Rtc.Enable32kHzPin(false);
//  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
JadwalSholat();
lcd.clear();
}

void loop() {
islam();
check();

switch(show){
  case ANIM_HOME :
    showDisplay();
    jadwalSholat();
  break;

  case ANIM_ADZAN :
    drawAzzan();
  break;

  case ANIM_NOTIF_CON :

  break;
};

}



void clearDay(){
  for(uint8_t i = 6; i <= 16; i++){ lcd.setCursor(i,0); lcd.print(" "); }
}

void clearJadwal(){
  for(uint8_t i = 0; i <= 12; i++){ lcd.setCursor(i,1); lcd.print(" "); }
}

void clearAll(){
  for(uint8_t i = 0; i <= 16; i++){ lcd.setCursor(i,0); lcd.print(" "); }
  for(uint8_t i = 0; i <= 16; i++){ lcd.setCursor(i,1); lcd.print(" "); }
}

int TIMER(int limit,int Delay){
  static uint32_t saveTimer =0;
  uint32_t tmr = millis();
  static uint8_t counter = 0,lastCounter = 0;

  if((tmr - saveTimer) > Delay){
    saveTimer = tmr;
    
    counter = (counter + 1) % limit; 
    if(lastCounter != counter) clearDay(); lastCounter = counter;
  }
  return counter;
}

void Buzzer(uint8_t state)
  {
   
    switch(state){
      case 0 :
        digitalWrite(BUZZ,HIGH);
      break;
      case 1 :
        digitalWrite(BUZZ,LOW);
      break;
      case 2 :
        for(int i = 0; i < 2; i++){
          digitalWrite(BUZZ,LOW);
          delay(80);
          digitalWrite(BUZZ,HIGH);
          delay(80);
        }
      break;
    };
  }

  void dwCtr(int row, String Msg) {
  int len = Msg.length();
  int col = (16 - len) / 2;
  if (col < 0) col = 0; // Cegah posisi negatif jika teks lebih dari 16 karakter
  lcd.setCursor(col, row); // row: 0 untuk baris atas, 1 untuk baris bawah
  lcd.print(Msg);
}
//----------------------------------------------------------------------
// I2C_ClearBus menghindari gagal baca RTC (nilai 00 atau 165)

int I2C_ClearBus() {
  
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}
