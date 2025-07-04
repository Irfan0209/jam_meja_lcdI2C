#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Wire.h>
#include <RtcDS3231.h>
#include <DHT.h>
#include <Prayer.h>
#include <avr/pgmspace.h>

// === Konfigurasi Pin dan Komponen ===
#define DHTPIN 3
#define DHTTYPE DHT11
#define BUZZER_PIN 13

DHT dht(DHTPIN, DHTTYPE);
//create object
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti 0x27 jika LCD kamu berbeda
// Constractor
Prayer JWS;
Hijriyah Hijir;

bool       adzan         = 0;

// === Alarm Settings ===
const int alarmHour = 6;
const int alarmMinute = 00;
bool alarmPlayed = false;

// === Waktu Looping ===
unsigned long lastDHTRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastBuzzToggle = 0;

const unsigned long DHT_INTERVAL = 600;
const unsigned long LCD_INTERVAL = 500;
const unsigned long BUZZ_INTERVAL = 300;

// === Data Sensor ===
int suhu = 0;
int lembab = 0;

// === Buzzer State ===
int buzzCount = 0;
bool buzzing = false;

uint8_t dataIhty[]      = {3,0,3,3,0,3};
//uint8_t dataIhty[]      = {3,0,3,3,0,3,2};
uint8_t   sholatNow     = -1;

struct Config {
  uint8_t durasiadzan = 40;
  uint8_t altitude = 10;
  double latitude = -7.364057;
  double longitude = 112.646222;
  uint8_t zonawaktu = 7;
  int16_t Correction = -1; //Koreksi tanggal hijriyah, -1 untuk mengurangi, 0 tanpa koreksi, 1 untuk menambah
};

Config config;


void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();

  dht.begin();
  int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
    if (rtn != 0) {
      Serial.println(F("I2C bus error. Could not clear"));
      if (rtn == 1) {
        Serial.println(F("SCL clock line held low"));
      } else if (rtn == 2) {
        Serial.println(F("SCL clock line held low by slave clock stretch"));
      } else if (rtn == 3) {
        Serial.println(F("SDA data line held low"));
      }
    } 
    else { // bus clear, re-enable Wire, now can start Wire Arduino master
      Wire.begin();
    }
  
  Rtc.Begin();
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  
   //islam();
   lcd.print("Jam Meja Arduino");
  delay(500);  // hanya di setup
  lcd.clear();
}

void loop() {

  unsigned long currentMillis = millis();

  // === Baca Sensor DHT tiap 2 detik ===
  if (currentMillis - lastDHTRead >= DHT_INTERVAL) {
    suhu = dht.readTemperature();
    lembab = dht.readHumidity();
    lastDHTRead = currentMillis;
  }

  // === Update LCD tiap 500ms ===
  if (currentMillis - lastLCDUpdate >= LCD_INTERVAL) {
    islam();
    lastLCDUpdate = currentMillis;
  }

  // === Buzzer tanpa delay ===
  if (buzzing && currentMillis - lastBuzzToggle >= BUZZ_INTERVAL) {
    toggleBuzzer();
    lastBuzzToggle = currentMillis;
  }
  showDisplay();
  jadwalSholat();
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

void clearDay(){
  for(uint8_t i = 6; i <= 16; i++){ lcd.setCursor(i,0); lcd.print(" "); }
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

// === Tampilkan LCD ===
void updateLCD() {
  RtcDateTime now = Rtc.GetDateTime();
  lcd.setCursor(0, 0);
  lcd.print(formatTime(now.Hour(), now.Minute(), now.Minute()));

  lcd.setCursor(11, 0);
  lcd.print(now.Day());
  lcd.print('/');
  lcd.print(now.Month());

  lcd.setCursor(0, 1);
  if (isnan(suhu) || isnan(lembab)) {
    lcd.print("Sensor Error     ");
  } else {
    lcd.print("S:");
    lcd.print((int)suhu);
    lcd.print((char)223); // derajat
    lcd.print(" H:");
    lcd.print((int)lembab);
    lcd.print("% ");
  }
}

// === Format Jam dengan Nol di Depan ===
String formatTime(int h, int m, int s) {
  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

// === Buzzer Handler ===
void startBuzzing() {
  buzzing = true;
  buzzCount = 0;
  digitalWrite(BUZZER_PIN, HIGH);
  lastBuzzToggle = millis();
}

void toggleBuzzer() {
  bool state = digitalRead(BUZZER_PIN);
  digitalWrite(BUZZER_PIN, !state);

  if (!state) { // Setiap OFF dianggap 1 cycle
    buzzCount++;
    if (buzzCount >= 120) { // 3x ON-OFF = 6 toggle
      buzzing = false;
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
