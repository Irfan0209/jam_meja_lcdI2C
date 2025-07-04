const char *pasar[] = {"WAGE", "KLIWON", "LEGI", "PAHING", "PON"}; 
const char *Hari[] = {"MINGGU","SENIN","SELASA","RABU","KAMIS","JUM'AT","SABTU"};
const char jadwal[][8] PROGMEM = {
    "SUBUH ", "DZUHUR ", "ASHAR ", 
    "MAGRIB ", "ISYA' "
  };

void showDisplay(){
  RtcDateTime now = Rtc.GetDateTime();

  uint8_t jam = now.Hour();
  uint8_t menit = now.Minute();
  uint8_t tgl = now.Day();
  uint8_t bln = now.Month();
  uint16_t thn = now.Year();

  lcd.setCursor(0,0);
  lcd.print((jam < 10 ? "0" : "") + String(jam));
  lcd.print(((now.Second()&1)==0)?":" : " ");
  lcd.print((menit < 10 ? "0" : "") + String(menit));

  uint8_t counter = TIMER(3,2000);

  switch(counter){
    case 0 :
      lcd.setCursor(6,0);
      lcd.print(pasar[jumlahhari() % 5]);
      Serial.println(pasar[jumlahhari() % 5]);
      break;

    case 1 :
      lcd.setCursor(6,0);
      lcd.print(Hari[now.DayOfWeek()]);
      Serial.println(Hari[now.DayOfWeek()]);
      break;

    case 2 :
      lcd.setCursor(6,0);
      lcd.print((tgl <10 ? "0" : "") + String(tgl));
      lcd.print("-");
      lcd.print((bln <10 ? "0" : "") + String(bln));
      lcd.print("-");
      lcd.print(thn);
      break;
  }

  lcd.setCursor(13,1);
  lcd.print(suhu);
  lcd.print("C");

  // === Alarm Logic ===
  if (now.Hour() == alarmHour && now.Minute() == alarmMinute) {
    if (!alarmPlayed) {
      startBuzzing();
      alarmPlayed = true;
    }
  } else {
    alarmPlayed = false;
  }
}


void jadwalSholat(){
 
if (adzan) return;

RtcDateTime now = Rtc.GetDateTime();
    uint8_t jam = now.Hour();
    uint8_t menit = now.Minute();
    uint8_t detik = now.Second();
    uint8_t daynow = now.DayOfWeek();
    uint8_t hours, minutes;
    static uint8_t counter = 0;
    static uint32_t lsTmr,saveTmr;
    //static bool adzanFlag[5] = {false, false, false, false, false};
    float sholatT[]={JWS.floatSubuh,JWS.floatDzuhur,JWS.floatAshar,JWS.floatMaghrib,JWS.floatIsya};
    uint32_t tmr = millis();
    char buff_jam[6]; // Format HH:MM hanya butuh 6 karakter
    char sholat[5];   // Buffer untuk menyimpan nama sholat dari PROGMEM

    
    if (tmr - lsTmr > 2000) {
        lsTmr = tmr;
        
        float stime = sholatT[counter];
        uint8_t hours = floor(stime);
        uint8_t minutes = floor((stime - (float)hours) * 60);
        //uint8_t ssecond = floor((stime - (float)hours - (float)minutes / 60) * 3600);

        // Ambil nama sholat dari Flash
        strcpy_P(sholat, jadwal[counter]);
  
        // Format HH:MM
        sprintf(buff_jam, "%02d:%02d", hours, minutes);
  
        // Tampilkan teks dengan animasi
        lcd.setCursor(0,1);
        lcd.print(sholat);
        lcd.setCursor(7,1);
        lcd.print(buff_jam);
        
        counter = (counter + 1) % 5;
    }
  
}
