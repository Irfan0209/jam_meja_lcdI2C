/*//================= cek waktu sholat ===================//
void check() {
    RtcDateTime now = Rtc.GetDateTime();
    uint8_t jam = now.Hour();
    uint8_t menit = now.Minute();
    uint8_t detik = now.Second();
    uint8_t daynow = now.DayOfWeek();
    uint8_t hours, minutes;
    static uint8_t counter = 0,cekList = 0;
    static uint32_t lsTmr,saveTmr;
    static bool adzanFlag[5] = {false, false, false, false, false};
    float sholatT[]={JWS.floatSubuh,JWS.floatDzuhur,JWS.floatAshar,JWS.floatMaghrib,JWS.floatIsya};
    uint32_t tmr = millis();
    
    
    if (tmr - lsTmr > 100) {
        lsTmr = tmr;
        
        float stime = sholatT[counter];
        uint8_t hours = floor(stime);
        uint8_t minutes = floor((stime - (float)hours) * 60);
        //uint8_t ssecond = floor((stime - (float)hours - (float)minutes / 60) * 3600);
    
        if (!adzanFlag[counter]) {
            if (jam == hours && menit == minutes && detik == 0) {
                
              if(daynow == 5 && counter == 1){
                return ;
              }else{
                lcd.clear();
                sholatNow = counter;
                adzan = 1;
                reset_x = 1;
                list = 0;
                lastList = 0;
                show = ANIM_ADZAN;
                adzanFlag[counter] = true;
              }
                
            }
        }

        if (jam != hours || menit != minutes) {
            adzanFlag[counter] = false;
        }
        
        
        counter = (counter + 1) % 5;
    }
}*/
