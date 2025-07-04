const char * const pasar[] PROGMEM = {"WAGE", "KLIWON", "LEGI", "PAHING", "PON"}; 
const char * const Hari[] PROGMEM = {"MINGGU","SENIN","SELASA","RABU","KAMIS","JUM'AT","SABTU"};
const char jadwal[][8] PROGMEM = {
    "SUBUH ", "TERBIT ", "DZUHUR ", "ASHAR ", 
    "TRBNM ", "MAGRIB ", "ISYA' "
  };


void showDisplay(){
  uint8_t jam = hour();
  uint8_t menit = minute();
  uint8_t tgl = day();
  uint8_t bln = month();
  uint16_t thn = year();
  
  lcd.setCursor(0,0);
  lcd.print((jam < 10 ? "0" : "") + String(jam));
  lcd.print(((second()&1)==0)?":" : " ");
  lcd.print((menit < 10 ? "0" : "") + String(menit));

  uint8_t counter = TIMER(3,2000);
  //Serial.println("counter:" + String(counter));
  switch(counter){
    case 0 :
      lcd.setCursor(6,0);
      lcd.print(Hari[weekday()-1]);
    break;

    case 1 :
      lcd.setCursor(6,0);
      lcd.print(pasar[jumlahhari() % 5]);
    break;

    case 2 :
      lcd.setCursor(6,0);
      lcd.print((tgl <10 ? "0" : "") + String(tgl));
      lcd.print("-");
      lcd.print((bln <10 ? "0" : "") + String(bln));
      lcd.print("-");
      lcd.print(thn);
  };

  lcd.setCursor(13,1);
  lcd.print(suhu);
  lcd.print("C");
  
}


void jadwalSholat(){
 
if (adzan) return;

//  RtcDateTime now = Rtc.GetDateTime();
  static uint32_t   lsRn;
  uint32_t          Tmr = millis(); 
  static uint8_t list,lastList;

  //int hours, minutes;
  char buff_jam[6]; // Format HH:MM hanya butuh 6 karakter
  char sholat[8];   // Buffer untuk menyimpan nama sholat dari PROGMEM

  // Ambil nama sholat dari Flash
  strcpy_P(sholat, jadwal[list]);
  
  int hours, minutes;
  uint16_t tahun = year();
  uint8_t bulan = month();
  uint8_t tanggal = day();

    if((Tmr-lsRn)>2000) 
      { 
        lsRn = Tmr;
        list = (list + 1) % 7;
        if(lastList != list) clearJadwal(); lastList = list;
      }

    if(list == 0){ JadwalSholat(); }
    else if (list == 4) list = 5;  
    else if (list == 7) list = 0;

  // Ambil nama sholat dari Flash
  strcpy_P(sholat, jadwal[list]);

  get_float_time_parts(times[list], hours, minutes);

  minutes = minutes + dataIhty[list];

  if (minutes >= 60) { minutes -= 60; hours++; }

  // Format HH:MM
  sprintf(buff_jam, "%02d:%02d", hours, minutes);

  // Tampilkan teks dengan animasi
  lcd.setCursor(0,1);
  lcd.print(sholat);
  lcd.setCursor(7,1);
  lcd.print(buff_jam);
}

/*=============================================================================================*/

void drawAzzan()
{
    static const char *jadwal[] = {"SUBUH", "TERBIT", "DZUHUR", "ASHAR", "TRBNM", "MAGRIB", "ISYA'"};
    const char *sholat = jadwal[sholatNow]; 
    static uint8_t ct = 0;
    static uint32_t lsRn = 0;
    uint32_t Tmr = millis();
    const uint8_t limit = 40; // config.durasiadzan;

    if (Tmr - lsRn > 500 && ct <= limit)
    {
        lsRn = Tmr;
        if (!(ct & 1))  // Lebih cepat dibandingkan ct % 2 == 0
        {
            dwCtr(0,"ADZAN");//lcd.print("ADZAN");
            dwCtr(1,sholat);//lcd.print(sholat);          
            Buzzer(1);
        }
        else
        {
            Buzzer(0);
            clearAll();
        }
        ct++;
    }
    
    if ((Tmr - lsRn) > 1500 && (ct > limit))
    {
        show = ANIM_HOME;
        ct = 0;
        Buzzer(0);
        adzan = 0;
        lcd.clear();
    }
}
