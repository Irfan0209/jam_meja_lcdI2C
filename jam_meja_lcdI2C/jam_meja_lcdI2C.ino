
#include <Wire.h>
#include <RtcDS3231.h>
#include <LiquidCrystal_I2C.h>

#include "PrayerTimes.h"

//create object
LiquidCrystal_I2C lcd(0x27, 16, 2);
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;
double times[sizeof(TimeName)/sizeof(char*)];

uint8_t    trigJam       = 17;
uint8_t    trigMenit     = 30;


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

void setup() {
  lcd.begin();
  lcd.backligt();

}

void loop() {
  // put your main code here, to run repeatedly:

}
