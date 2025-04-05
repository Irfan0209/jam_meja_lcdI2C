//////////hijriyah
#define epochHijriah          1948439.5f //math.harvard.edu
#define tambahKurangHijriah   0
uint8_t maxday[]        = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint8_t    trigJam       = 17;
uint8_t    trigMenit     = 30;

//////hijiriyah voidku/////////////////////////////////////////////////
void islam() {
  
  waktuMagrib = {trigJam, trigMenit};
  
  // Serial.println("trigJam  :" + String(trigJam));
  // Serial.println("trigMenit:" + String(trigMenit));

  RtcDateTime now = Rtc.GetDateTime();
  tanggalMasehi.tanggal = now.Day();
  tanggalMasehi.bulan = now.Month();
  tanggalMasehi.tahun = now.Year() - 2000;

  uint32_t jumlahHari;
  double jumlahHariMatahari;

  tanggalMasehi.hari = hariDariTanggal(tanggalMasehi);
  masehiKeHijriah(tanggalMasehi, waktuMagrib, tambahKurangHijriah,
                  tanggalHijriah, tanggalJawa);
  printKonversi(tanggalMasehi, tanggalHijriah, tanggalJawa);

  //yield();
}

double get_julian_date(Tanggal tanggal)
{
  if (tanggal.bulan <= 2) {
    tanggal.tahun--;
    tanggal.bulan += 12;
  }

  int a = tanggal.tahun / 100;
  int b = 2 - a + (a / 4);

  if (tanggal.tahun < 1583) b = 0;
  if (tanggal.tahun == 1582 && tanggal.bulan == 10 && tanggal.tanggal > 4) b = -10;

  return (365.25 * (tanggal.tahun + 4716)) + (30.6001 * (tanggal.bulan + 1)) + tanggal.tanggal + b - 1524.5;
}

double konversiTanggalHijriahKeJulianDate(uint16_t tahun, uint8_t
    bulan, uint8_t tanggal)
{
  return (epochHijriah + tanggal + ceil(29.5 * (bulan - 1)) +
          (354L * (tahun - 1)) + floor((3 + (11 * tahun)) / 30)) - 1;
}

void masehiKeHijriah(TanggalDanWaktu masehi, JamDanMenit
                     waktuSholatMagrib, int8_t koreksiHijriah, Tanggal & hijriah,
                     TanggalJawa & jawa)
{
  uint16_t sisaHari;
  double julianDate = get_julian_date({masehi.tanggal,
                                       masehi.bulan, masehi.tahun + 2000
                                      });

  uint16_t menitMagrib = waktuSholatMagrib.jam * 60  +
                         waktuSholatMagrib.menit;
  uint16_t menitSekarang = masehi.jam * 60 + masehi.menit;

  if (menitSekarang >= menitMagrib)
  {
    julianDate++;//Pergantian hari hijrah pada magrib
  }

  julianDate = floor(julianDate) + 0.5;

  Tanggal tanggalHijriah;

  hijriah.tahun = floor(((30 * (julianDate - epochHijriah)) +
                         10646) / 10631);
  hijriah.bulan = min(12.0, ceil((julianDate - (29 +
                                  konversiTanggalHijriahKeJulianDate(hijriah.tahun, 1, 1))) / 29.5)
                      + 1);
  hijriah.tanggal = (julianDate -
                     konversiTanggalHijriahKeJulianDate(hijriah.tahun, hijriah.bulan,
                         1)) + 1+config.chijir;

  long julianLong = (long)julianDate;
  jawa.pasaran = ((julianLong + 1) % 5); //0 = legi
  jawa.wuku = (((julianLong + 65) % 210) / 7); //0 = Shinta
  jawa.tanggal = hijriah.tanggal;
  jawa.bulan = hijriah.bulan;
  jawa.tahun = hijriah.tahun + 512;

}
uint8_t hariDariTanggal(TanggalDanWaktu tanggalDanWaktu) {
  uint16_t jumlahHari = tanggalDanWaktu.tanggal;
  for (uint8_t i = 1; i < tanggalDanWaktu.bulan; ++i)
    jumlahHari += maxday[i - 1];
  if (tanggalDanWaktu.bulan > 2 && tanggalDanWaktu.tahun % 4 == 0)
    ++jumlahHari;
  jumlahHari += (365 * tanggalDanWaktu.tahun) +
                ((tanggalDanWaktu.tahun + 3) / 4) - 1;

  return ((jumlahHari + 6) % 7) + 1; // 1 Januari 2000 hari sabtu= 7

}
void printKonversi(TanggalDanWaktu tanggalMasehi, Tanggal
                   tanggalHijriah, TanggalJawa tanggalJawa)
{
  ///end hijir voidku///////////////////////////////////////////////////
}

// digunakan untuk menghitung hari pasaran
int jumlahhari() { 
  RtcDateTime now = Rtc.GetDateTime();
  int d = now.Day();
  int m = now.Month();
  int y = now.Year();

  static const int hb[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  int ht = (y - 1970) * 365 - 1;
  int hs = hb[m - 1] + d;

  if (y % 4 == 0 && m > 2) hs++; // Tambahkan 1 hari jika tahun kabisat dan lewat Februari

  int kab = (y - 1969) / 4;  // Hitung langsung jumlah tahun kabisat

  return (ht + hs + kab);
}

// PARAMETER PENGHITUNGAN JADWAL SHOLAT
void JadwalSholat() {
  
  RtcDateTime now = Rtc.GetDateTime();

  int tahun = now.Year();
  int bulan = now.Month();
  int tanggal = now.Day();

  Serial.println("calcualat run");
  set_calc_method(Karachi);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);

  get_prayer_times(tahun, bulan, tanggal, config.latitude, config.longitude, config.zonawaktu, times);
 //yield();
}
