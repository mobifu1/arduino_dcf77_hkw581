//Decode encrypted Meteotime Data from HKW581:

#include "DCF77.h"
#include "TimeLib.h"
#include <TimerOne.h>
#include <EEPROM.h>


#define microseconds_1 3000   //Timer1
#define led 13
#define led_1 12           // time updated indicator
#define DCF_PIN 2          // Connection pin to DCF 77 device
#define DCF_INTERRUPT 0    // Interrupt number associated with pin

time_t time;
DCF77 DCF = DCF77(DCF_PIN, DCF_INTERRUPT);

//60 Region,Cities:
const char *region[]  { "F  Bordeaux", "F  la Rochelle" , "F  Paris", "F  Brest", "F  Clermont", "F  Beziers", "B  Bruxelles", "F  Dijon", "F  Marseille", "F  Lyon", "F  Grenoble", "CH La Chaux"
  , "D  Frankfurt/M", "D  Trier", "D  Duisburg", "GB Swansea", "GB Manchester", "F  le Havre", "GB London", "D  Bremerhaven", "DK Herning", "DK Arhus", "D  Hannover", "DK Copenhagen"
  , "D  Rostock" , "D  Ingolstadt", "D  Muenchen", "I  Bolzano", "D  Nuernberg", "D  Leipzig", "D  Erfurt", "CH Lausanne", "CH Zuerich", "CH Adelboden", "CH Sion", "CH Glarus", "CH Davos"
  , "D  Kassel", "CH Locarno", "I  Sestriere" , "I  Milano", "I  Roma", "NL Amsterdam", "I  Genova", "I  Venezia", "D  Strasbourg", "A  Klagenfurt", "A  Innsbruck", "A  Salzburg", "A  Wien"
  ,  "CZ Praha", "CZ Decin", "D  Berlin", "S  Gothenburg" , "S  Stockholm", "S  Kalmar", "S  Joenkoeping", "D  Donauechingen", "N  Oslo", "D  Stuttgart" , "I  Napoli", "I  Ancona", "I  Bari"
  , "H  Budapest", "E  Madrid", "E  Bilbao", "I  Palermo", "E  Palma" , "E  Valencia", "E  Barcelona", "AN Andorra", "E  Sevilla", "P  Lissabon", "I  Sassari", "E  Gijon", "IR Galway"
  , "IR Dublin", "GB Glasgow", "N  Stavanger", "N  Trondheim", "S  Sundsvall", "PL Gdansk" , "PL Warszawa", "PL Krakow", "S  Umea", "S  Oestersund", "CH Samedan", "HR Zagreb", "CH Zermatt", "HR Split"
};
const char *weather[]  {  "Reserved", "Sunny", "Partly Clouded", "Mostly Clouded", "Overcast", "Heat Storms", "Heavy Rain", "Snow", "Fog", "Sleet", "Rain Showers", "Light Rain" , "Snow Showers",  "Frontal Storms", "Stratus Cloud", "Sleet Storms"};
const char *heavyweather[]  {  "None", "Heavy Weather 24 hrs.", "Heavy Weather Day", "Heavy Weather Night", "Storm 24hrs.", "Storm Day", "Storm Night",
  "Wind Gusts Day", "Wind Gusts Night", "Icy Rain Morning", "Icy Rain Evening", "Icy Rain Night", "Fine Dust", "Ozon", "Radiation", "High Water"
};
const char *rain_prop[]  {"0 %", "15 %", "30 %", "45 %", "60 %", "75 %", "90 %", "100 %"};
const char *winddirection[]  { "N", "NO", "O", "SO", "S", "SW", "W", "NW", "Changeable", "Foen", "Biese N/O", "Mistral N", "Scirocco S", "Tramont W", "Reserved", "Reserved"};
const char *windstrength[]  {"0", "0-2", "3-4", "5-6", "7", "8", "9", ">=10"};
const char *anomaly_1[]  {"No", "1", "2", "3"};
const char *anomaly_2[]  {"0-2 h", "2-4 h", "5-6 h", "7-8 h"};

//Version:
String sw_version = "V0.5";
boolean debuging = true;
boolean vfd_display = true;
boolean time_updated = false;
int vfd_counter = -1;
int day_counter = 0;
String vfd_upper_line = "";
String vfd_lower_line = "";
int y = 0;
boolean timer1_event = false;

//User Region:
int region_code;
byte user_region = 22; //Hannover 22
byte daylight_saving_time = 0;

//Region Forecast:
byte forecast_high_values[7][4] = { //1.day / 2.day / 3.day / 4.day
  {0, 0, 0, 0}, //wind direction
  {0, 0, 0, 0}, //wind strenth
  {0, 0, 0, 0}, //day
  {0, 0, 0, 0}, //night
  {0, 0, 0, 0}, //weather anomaly
  {0, 0, 0, 0}, //temperatur
  {0, 0, 0, 0}, //decoder status
};
byte forecast_low_values[7][4] = { //1.day / 2.day / 3.day / 4.day
  {0, 0, 0, 0}, //wind direction
  {0, 0, 0, 0}, //wind strenth
  {0, 0, 0, 0}, //day
  {0, 0, 0, 0}, //night
  {0, 0, 0, 0}, //weather anomaly
  {0, 0, 0, 0}, //temperatur
  {0, 0, 0, 0}, //decoder status
};
byte extreme_values[3][4] = {
  {0, 0, 0, 0}, //weather Extreme 1
  {0, 0, 0, 0}, //weather Extreme 2
  {0, 0, 0, 0}, //Rain Prop
};
//EEprom address:
int eeprom_forecast_high_values[7][4] = { //1.day / 2.day / 3.day / 4.day
  {3000, 3007, 3014, 3021}, //wind direction
  {3001, 3008, 3015, 3022}, //wind strenth
  {3002, 3009, 3016, 3023}, //day
  {3003, 3010, 3017, 3024}, //night
  {3004, 3011, 3018, 3025}, //weather anomaly
  {3005, 3012, 3018, 3026}, //temperatur
  {3006, 3013, 3020, 3027}, //decoder status
};
int eeprom_forecast_low_values[7][4] = { //1.day / 2.day / 3.day / 4.day
  {4000, 4007, 4014, 4021}, //wind direction
  {4001, 4008, 4015, 4022}, //wind strenth
  {4002, 4009, 4016, 4023}, //day
  {4003, 4010, 4017, 4024}, //night
  {4004, 4011, 4018, 4025}, //weather anomaly
  {4005, 4012, 4018, 4026}, //temperatur
  {4006, 4013, 4020, 4027}, //decoder status
};
int eeprom_extreme_values[3][4] = {
  {5000, 5003, 5006, 5009}, //weather Extreme 1
  {5001, 5004, 5007, 5010}, //weather Extreme 2
  {5002, 5005, 5008, 5011}, //Rain Prop
};

//UTC-Time schedule / +1h wintertime +2h sommertime:
//22:02-00:59 > 1.day=today Höchstwerte > WeatherExtreme & RainProp
//01:02-03:59 > 1.day=today Tiefststwerte > WindDir & WindStength
//04:02-06:59 > 2.day       Höchstwerte > WeatherExtreme & RainProp
//07:02-09:59 > 2.day       Tiefststwerte > WindDir & WindStength
//10:02-12:59 > 3.day       Höchstwerte > WeatherExtreme & RainProp
//13:02-15:59 > 3.day       Tiefststwerte > WindDir & WindStength
//16:02-18:59 > 4.day       Höchstwerte > WeatherExtreme & RainProp
//19:02-21:59 > 4.day       Wetterprognose & Temperatur > WindDir & WindStength

byte  wind_direction;
byte  weather_extreme_1;
byte  weather_extreme_2;
byte  wind_strength;
byte  rain_propability;
byte  day_value;
byte  night_value;
byte  weather_anomaly;
byte  temperatur;
byte  decoder_status;

//--------------------------------------------------------------------------
boolean lock;
String meteodata;
int decoderbit;
int val;
// Decoder Input Stream:
byte hkw_in[82] {0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}; //82 bits
byte hkw_out[24] {0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0}; //decode from hkw_in > 24 bits > "011001000000011011000110"

//Example:
//minute 0: first 14 bits
//minute 1: first 14 bits
//minute 2: first 14 bits                      & time   & hour   & date & month & dow & year
//01011010010110 11100011000111 00001111110011 10011010 01000100 11100  011     00101000
//01000110 00001000 11001110

//HKW581 Pins:     chip needs 40 seconds to warm up
#define Qout 3
#define DataIn 4
#define BFR 5
#define DataOut 6
#define ClockIn 7

//----------------------------------------------------------------------------------------------
void setup() {

  Timer1.initialize(microseconds_1);
  Timer1.attachInterrupt(timer1_subroutine);
  pinMode(led, OUTPUT);
  pinMode(led_1, OUTPUT);

  pinMode(DataIn, OUTPUT);
  pinMode(DataOut, INPUT);
  pinMode(ClockIn, OUTPUT);
  pinMode(Qout, INPUT);
  pinMode(BFR, INPUT);
  digitalWrite(DataIn, LOW);
  digitalWrite(ClockIn, LOW);

  Serial.begin(9600);
  Serial1.begin(9600);
  Serial1.write(0x1B);//init vfd
  Serial1.write(0x40);//init vfd
  DCF.Start();
  Serial.println(sw_version);
  Serial.println("Waiting for DFC77 Signal.................");
  Serial.print("My Region: " + String(user_region) + " ");
  Serial.println(region[user_region]);

  Serial1.write(0x0C);//Clear display
  Serial1.write(0x0B);//home position
  Serial1.print("DCF77 " + sw_version);
  Serial1.write(0x0A);//move cursor down
  Serial1.write(0x0D);//move cursor left end
  Serial1.print("Waiting for Signal..");
  //-----------------------
  //init_eeprom_table();//reset all values
  load_eeprom_table(); // load all stored data into ram array

  //setTime(13, 02, 00, 31, 12, 2016);
  //   0-3,   4-7,    8-11,   12-14,              15, 16-21,          22-23,
  //   day, night, winddir, windstr,  weatheranomaly,  temp,  decoder state,

  //meteodata = "111001101100011010110110"; //Test: 24bits: Snow, Heavy rain, SO, 9-Bf, No, 23⁰c

}
//----------------------------------------------------------------------------------------------
void loop() {

  if (timer1_event == true) {
    timer1_event = false;
    show_forecast_vfd();
  }

  boolean val = digitalRead(DCF_PIN);
  if (val == LOW) digitalWrite(led, LOW);
  if (val == HIGH) digitalWrite(led, HIGH);
  if (time_updated == true) digitalWrite(led_1, HIGH);


  time_t DCFtime = DCF.getTime(); // Check if new DCF77 time is available
  if (DCFtime != 0)  {
    setTime(DCFtime);
    if (debuging == true) {
      Serial.print("DCF Time is updated: ");
      Serial.println(String(hour()) + ":" + String(minute()) + ":" + String(second()));
    }
    time_updated = true;
  }

  if (time_updated == true) {

    if (second() == 59 && lock == false) {
      digitalWrite(led_1, LOW);//reset update led
      lock = true;
      int result = (minute() + 1) % 3;
      String dcf_bitstream = DCF.getEncWeatherData();
      if (debuging == true) {
        Serial.print(String(hour()) + ":" + String(minute()) + ":" + String(second()) + " ");
        Serial.print("Bitstream:" + String(dcf_bitstream));
      }
      collect_data(dcf_bitstream, result); //result can 0=final,2=2.,1=1.packet

      if (result == 0 ) {//minute:2,5,8,11........
        write_data_to_hkw(); // input > hkw_in[]
        read_data_from_hkw(); // result > meteodata
        show_region();
        calc_data();
        fill_forecast_table();
        show_forcast_table();
      }
    }

    if (second() == 58) {
      lock = false;
    }
  }
}
//----------------------------------------------------------------------------------------------
void init_eeprom_table() { // load all stored data into ram array

  for (int i = 0; i < 7; i++) {
    for (int l = 0; l < 4; l++) {
      int address = eeprom_forecast_high_values[i][l];
      write_eeprom_byte(address, 0);
    }
  }

  for (int i = 0; i < 7; i++) {
    for (int l = 0; l < 4; l++) {
      int address = eeprom_forecast_low_values[i][l];
      write_eeprom_byte(address, 0);
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int l = 0; l < 4; l++) {
      int address = eeprom_extreme_values[i][l];
      write_eeprom_byte(address, 0);
    }
  }
}
//----------------------------------------------------------------------------------------------
void load_eeprom_table() { // load all stored data into ram array

  for (int i = 0; i < 7; i++) {
    for (int l = 0; l < 4; l++) {
      int address = eeprom_forecast_high_values[i][l];
      byte val = read_eeprom_byte(address);
      forecast_high_values[i][l] = val; //1.day / 2.day / 3.day / 4.day
    }
  }

  for (int i = 0; i < 7; i++) {
    for (int l = 0; l < 4; l++) {
      int address = eeprom_forecast_low_values[i][l];
      byte val = read_eeprom_byte(address);
      forecast_low_values[i][l] = val; //1.day / 2.day / 3.day / 4.day
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int l = 0; l < 4; l++) {
      int address = eeprom_extreme_values[i][l];
      byte val = read_eeprom_byte(address);
      extreme_values[i][l] = val; //1.day / 2.day / 3.day / 4.day
    }
  }
}
//----------------------------------------------------------------------------------------------
void write_eeprom_byte(int address, byte value) {
  EEPROM.update(address, value);
}
//----------------------------------------------------------------------------------------------
byte read_eeprom_byte(int address) {
  byte value;
  value = EEPROM.read(address);
  return value;
}
//----------------------------------------------------------------------------------------------
void collect_data(String input, int packet) { //packet can 0=final,1=2.,2=1. packet

  if (debuging == true) Serial.println(" Packet:" + String(packet));
  int bit_count;

  if (input.substring(16, 18) == "01") daylight_saving_time = 1;//MEZ
  if (input.substring(16, 18) == "10")daylight_saving_time = 2;//MESZ

  if (packet == 1) { //1.packet
    bit_count = 0;
    for (int k = 0; k < 14; k++) {
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
  }

  if (packet == 2) { //2.packet
    bit_count = 14;
    for (int k = 0; k < 14; k++) {
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    //time > Key
    bit_count = 42;
    for (int k = 20; k < 27; k++) {//minute bit 21-27 > gesamt:8bits
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    hkw_in[bit_count] = 0; //leading zero for minute
    bit_count++;
    //--------------------------------
    for (int k = 28; k < 34; k++) {//hour bit 29-34  > gesamt:8bits
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    hkw_in[bit_count] = 0;//leading zeros for hour
    bit_count++;
    hkw_in[bit_count] = 0;
    bit_count++;
    //--------------------------------
    for (int k = 35; k < 41; k++) {//day bit 36-41  > gesamt:8bits
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    hkw_in[bit_count] = 0;//leading zeros for day
    bit_count++;
    hkw_in[bit_count] = 0;
    bit_count++;
    //--------------------------------
    for (int k = 44; k < 49; k++) {//month bit 45-49  > gesamt:5bits
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    //--------------------------------
    for (int k = 41; k < 44; k++) {//dow 42-44  > gesamt:3bits
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    //--------------------------------
    for (int k = 49; k < 57; k++) {//year 50-57  > gesamt:8bits
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    //--------------------------------
  }

  if (packet == 0) { //3.packet > final

    bit_count = 28;
    for (int k = 0; k < 14; k++) {
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    //---------------------------------------
    if (debuging == true) {
      Serial.println();
      for (int k = 0; k < 82; k++) {
        if (k == 14 || k == 28 || k == 42 || k == 50 || k == 58 || k == 66 || k == 71 || k == 74) Serial.print("-");//markers
        Serial.print(String(hkw_in[k]));
      }
      Serial.println();
    }
  }
}
//----------------------------------------------------------------------------------------------
void write_data_to_hkw() {

  /*
    Starting condition all lines low.
    Sending data to the chip:
    –> Set the bit on the DataIn line
    –> Set ClockIn to high
    <– Chip sets Qout to high
    –> Set ClockIn to low
    <– Chip sets QOut put to low

    Then you have to ClockIn the data.
    You set the first bit on DataIn, and then pulse the ClockIn line.
    This you do 82 times for all 82 bits. Once all the data is clocked in the decoding chip calculates the result.
  */

  if (debuging == true) Serial.println("Write 82  bits to Chip");

  digitalWrite(DataIn, LOW);
  digitalWrite(ClockIn, LOW);

  for (int k = 0; k < 82; k++) {

    if (hkw_in[k] == 1)digitalWrite(DataIn, HIGH);
    if (hkw_in[k] == 0)digitalWrite(DataIn, LOW);
    digitalWrite(ClockIn, HIGH);

    do {}
    while (digitalRead (Qout) == LOW);//wait for Qout == HIGH

    digitalWrite(ClockIn, LOW);

    do {}
    while (digitalRead (Qout) == HIGH);//wait for Qout == LOW

  }
  digitalWrite(DataIn, LOW);
}
//----------------------------------------------------------------------------------------------
void read_data_from_hkw() {

  /*
    Starting condition all lines low.
    Receiving data from the chip:
    <– Chip sets QOut to high
    –> Read bit from DOut
    –> Set ClockIn to high
    <– Chip sets QOut put to low
    –> Set ClockIn to low

    Then you have to ClockIn the data.
    You set the first bit on DataIn, and then pulse the ClockIn line.
    This you do 82 times for all 82 bits. Once all the data is clocked in the decoding chip calculates the result.
  */
  meteodata = "";

  for (int k = 23; k > -1; k--) { //read and flip bit stream

    do {}
    while (digitalRead (Qout) == LOW);//wait for Qout == HIGH

    boolean value = digitalRead (DataOut);
    if (value == true) hkw_out[k] = 1;
    if (value == false) hkw_out[k] = 0;
    meteodata += String(hkw_out[k]);
    digitalWrite(ClockIn, HIGH);

    do {}
    while (digitalRead (Qout) == HIGH);//wait for state == LOW

    digitalWrite(ClockIn, LOW);
  }
  if (debuging == true) Serial.println("Read Meteodata:" + meteodata);
}
//----------------------------------------------------------------------------------------------
void show_region() {

  int utc_hour_int = hour();
  utc_hour_int -= daylight_saving_time;
  if (utc_hour_int < 0)utc_hour_int += 24;

  region_code = ((((utc_hour_int + 2) % 3) * 60) + minute()); //every 3 hours full round
  //22:00 = 0,  00:59 = 2*60 + 59 = 179
  region_code = region_code / 3;

  Serial.println();
  if (hour() < 10) {
    Serial.print("0" + String(hour()) + ":");
  }
  else {
    Serial.print(String(hour()) + ":");
  }

  if (minute() < 10) {
    Serial.print("0" + String(minute()));
  }
  else {
    Serial.print(String(minute()));
  }
  Serial.print(" = UTC+" + String(daylight_saving_time) + "h");
  Serial.println();
  Serial.print("Life Region: " + String(region_code) + " ");
  Serial.println(region[region_code]);

}
//----------------------------------------------------------------------------------------------
void calc_data() {

  int utc_hour_int = hour();
  utc_hour_int -= daylight_saving_time;
  if (utc_hour_int < 0)utc_hour_int += 24;

  //   0-3,   4-7,    8-11,   12-14,              15, 16-21,
  //   day, night, winddir, windstr,  weatheranomaly,  temp,  decoder state,

  if (utc_hour_int > 18 && utc_hour_int < 23) { //not between 19 & 22 hour
    // do something
  }
  else {
    if (((utc_hour_int + 2) % 6) < 3) { //22-00 / 04-06 / 10-12 / 16-18
      //here receiving the WeatherExtreme & RainProps:
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      if (debuging == true) {
        Serial.print("Rain Prop         =    ");
        Serial.print(meteodata.substring(12, 15));
        Serial.print(" ");
      }

      val = string_to_int(12, 15);
      val = reverse_bits (val, 3);
      wind_strength = val;
      rain_propability = val;

      if (debuging == true) {
        Serial.print(val, HEX);
        Serial.print(" ");
        Serial.print(rain_prop[rain_propability]);
        Serial.println();
      }
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      if (debuging == true) {
        Serial.print("Weather Anomaly =      ");
        Serial.print(meteodata.substring(15, 16));
        Serial.print(" ");
      }

      val = string_to_int(15, 16);
      weather_anomaly = val;

      if (debuging == true) {
        Serial.print(val, HEX);
        if (val == 1) {
          Serial.println(" Yes");
        }
        else {
          Serial.println(" No");
        }
      }
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      if (weather_anomaly == 1) {
        if (debuging == true) {
          Serial.print("Anomaly Weather 1 =   ");
          Serial.print(meteodata.substring(8, 10));
        }

        val = string_to_int(8, 10);
        weather_extreme_1 = val;

        if (debuging == true) {
          Serial.print(val, HEX);
          Serial.println(anomaly_1[weather_extreme_1]);
        }

        if (debuging == true) {
          Serial.print("Anomaly Weather 2 =   ");
          Serial.print(meteodata.substring(10, 12));
        }

        val = string_to_int(10, 12);
        weather_extreme_2 = val;

        if (debuging == true) {
          Serial.print(val, HEX);
          Serial.println(anomaly_2[weather_extreme_2]);
        }
      }
    }
    else {
      //here receiving the Wind Dir & Wind Strength:
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      if (debuging == true) {
        Serial.print("Weather Anomaly   =   ");
        Serial.print(meteodata.substring(15, 16));
        Serial.print(" ");
      }

      val = string_to_int(15, 16);
      weather_anomaly = val;

      if (debuging == true) {
        Serial.print(val, HEX);
        if (val == 1) {
          Serial.println(" Yes");
        }
        else {
          Serial.println(" No");
        }
      }
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      if (debuging == true) {
        Serial.print("Winddirection     =   ");
        Serial.print(meteodata.substring(8, 12));      //   0-3,   4-7,    8-11,   12-14,              15, 16-21,          22-23,
        Serial.print(" ");                             //   day, night, winddir, windstr,  weatheranomaly,  temp,  decoder state,
      }

      val = string_to_int(8, 12);
      val = reverse_bits (val, 4);
      wind_direction = val;

      if (debuging == true) {
        Serial.print(val, HEX);
        Serial.print(" ");
        Serial.print(winddirection[val]);
        Serial.println();
      }
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      if (debuging == true) {
        Serial.print("Windstrength      =    ");
        Serial.print(meteodata.substring(12, 15));
        Serial.print(" ");
      }

      val = string_to_int(12, 15);
      val = reverse_bits (val, 3);
      wind_strength = val;

      if (debuging == true) {
        Serial.print(val, HEX);
        Serial.print(" ");
        Serial.print(windstrength[val]);
        Serial.println(" Bft");
      }
    }
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  if (debuging == true) {
    Serial.print("Day               =   ");
    Serial.print(meteodata.substring(0, 4));
    Serial.print(" ");
  }

  val = string_to_int(0, 4);
  val = reverse_bits (val, 4);
  day_value = val;

  if (debuging == true) {
    Serial.print(val, HEX);
    Serial.print(" ");
    Serial.print(weather[val]);
    Serial.println();
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  if (debuging == true) {
    Serial.print("Night             =   ");
    Serial.print(meteodata.substring(4, 8));
    Serial.print(" ");
  }

  val = string_to_int(4, 8);
  val = reverse_bits (val, 4);
  night_value = val;

  if (debuging == true) {
    Serial.print(val, HEX);
    Serial.print(" ");
    if (val == 1) {
      Serial.print("Clear");
      Serial.println();
    }
    else {
      Serial.print(weather[val]);
      Serial.println();
    }
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  if (debuging == true) {
    Serial.print("Temperature       = ");
    Serial.print(meteodata.substring(16, 22));
    Serial.print(" ");
  }

  val = string_to_int(16, 22);
  val = reverse_bits (val, 6);
  temperatur = val - 22;

  if (debuging == true) {
    Serial.print("0x");
    Serial.print(val, HEX);
    Serial.print(" ");
    if (val == 0) {
      Serial.print("<-21 ");
      Serial.print("C");
    }
    if (val == 63) {
      Serial.print (">40 ");
      Serial.print("C");
    }
    if ((val != 0) & (val != 63)) {
      Serial.print(val - 22, DEC);
      Serial.print("C");
    }
    Serial.println();
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++

  if (debuging == true)   Serial.print("Decoder status    =     ");
  if (debuging == true)   Serial.print(meteodata.substring(22, 24));

  if (meteodata.substring(22, 24) == "10") {
    decoder_status = 10;
    if (debuging == true) Serial.println (" OK");
  } else {
    decoder_status = 0;
    if (debuging == true) Serial.println (" FAIL");
  }
}
//---------------------------------------------------------------------
void fill_forecast_table() {

  byte day_x = 0;
  boolean high_value = false;
  boolean propagation = false;
  int utc_hour_int = hour();
  utc_hour_int -= daylight_saving_time;
  if (utc_hour_int < 0)utc_hour_int += 24;

  if (utc_hour_int  >= 22  && utc_hour_int  <= 0 ) { //UTC 22:02-00:59 > 1.day=today Höchstwerte
    day_x = 0;
    high_value = true;
  }
  if (utc_hour_int   >= 1  && utc_hour_int   <= 3 ) { //01:02-03:59 > 1.day=today Tiefststwerte
    day_x = 0;
    high_value = false;
  }
  if (utc_hour_int   >= 4  && utc_hour_int   <= 6 ) { //04:02-06:59 > 2.day Höchstwerte
    day_x = 1;
    high_value = true;
  }
  if (utc_hour_int   >= 7  && utc_hour_int  <= 9 ) { //07:02-09:59 > 2.day Tiefststwerte
    day_x = 1;
    high_value = false;
  }
  if (utc_hour_int   >= 10  && utc_hour_int   <= 12 ) { //10:02-12:59 > 3.day Höchstwerte
    day_x = 2;
    high_value = true;
  }
  if (utc_hour_int   >= 13  && utc_hour_int   <= 15 ) { //13:02-15:59 > 3.day Tiefststwerte
    day_x = 2;
    high_value = false;
  }
  if (utc_hour_int   >= 16  && utc_hour_int   <= 18 ) { //16:02-18:59 > 4.day Höchstwerte
    day_x = 3;
    high_value = true;
  }
  if (utc_hour_int   >= 19  && utc_hour_int   <= 21 ) { //19:02-22:59 > 4.day Wetterprognose & Temperatur
    day_x = 3;
    high_value = false;
    propagation = true;
  }
  if (debuging == true) {
    Serial.print("Schedule          =        ");
    Serial.print(String(day_x + 1) + ".Day ");
    if (propagation == false) {
      if (high_value == true) Serial.print("Hoechstwerte");
      if (high_value == false) Serial.print("Tiefstwerte");
      Serial.println();
    }
    else {
      Serial.print("Wetterprognose & Temperatur");
    }
  }

  if (region_code == user_region) {
    if (decoder_status == 10) {
      if (utc_hour_int >= 19 && utc_hour_int <= 21) { //not between 19 & 22 hour
        // do something
      }
      else {
        if (high_value == true) {
          int address;
          address = eeprom_extreme_values[0][day_x];
          write_eeprom_byte(address, weather_extreme_1);
          address = eeprom_extreme_values[1][day_x];
          write_eeprom_byte(address, weather_extreme_2);
          address = eeprom_extreme_values[2][day_x];
          write_eeprom_byte(address, rain_propability);

          address = eeprom_forecast_high_values[0][day_x];
          write_eeprom_byte(address, wind_direction);
          address = eeprom_forecast_high_values[1][day_x];
          write_eeprom_byte(address, wind_strength);
          address = eeprom_forecast_high_values[2][day_x];
          write_eeprom_byte(address, day_value);
          address = eeprom_forecast_high_values[3][day_x];
          write_eeprom_byte(address, night_value);
          address = eeprom_forecast_high_values[4][day_x];
          write_eeprom_byte(address, weather_anomaly);
          address = eeprom_forecast_high_values[5][day_x];
          write_eeprom_byte(address, temperatur);
          address = eeprom_forecast_high_values[6][day_x];
          write_eeprom_byte(address, decoder_status);
        }
        if (high_value == false) {
          int address;
          address = eeprom_forecast_low_values[0][day_x];
          write_eeprom_byte(address, wind_direction);
          address = eeprom_forecast_low_values[1][day_x];
          write_eeprom_byte(address, wind_strength);
          address = eeprom_forecast_low_values[2][day_x];
          write_eeprom_byte(address, day_value);
          address = eeprom_forecast_low_values[3][day_x];
          write_eeprom_byte(address, night_value);
          address = eeprom_forecast_low_values[4][day_x];
          write_eeprom_byte(address, weather_anomaly);
          address = eeprom_forecast_low_values[5][day_x];
          write_eeprom_byte(address, temperatur);
          address = eeprom_forecast_low_values[6][day_x];
          write_eeprom_byte(address, decoder_status);
        }
      }
    }
    load_eeprom_table(); // load all stored data into ram array
  }
}
//---------------------------------------------------------------------
void show_forcast_table() {

  if (debuging == true) {
    Serial.println();
    Serial.print("My Region:" + String(user_region) + " ");
    Serial.println(region[user_region]);
    Serial.println("High:");
    for (int k = 0; k < 4; k++) {
      if (forecast_high_values[6][k] == 10)Serial.println("Day:" + String(k + 1) + " Wind Dir:" + winddirection[forecast_high_values[0][k]] + " Wind Strength:" + windstrength[forecast_high_values[1][k]] + "Bft Day:" + weather[forecast_high_values[2][k]] + " Night:" + weather[forecast_high_values[3][k]] + " Temp:" + forecast_high_values[5][k]  + "C Rain:" + rain_prop[extreme_values[2][k]] + " Decoder Status:" + forecast_high_values[6][k]);
    }
    Serial.println("Low:");
    for (int k = 0; k < 4; k++) {
      if (forecast_low_values[6][k] == 10)Serial.println("Day:" + String(k + 1) + " Wind Dir:" + winddirection[forecast_low_values[0][k]] + " Wind Strength:" + windstrength[forecast_low_values[1][k]] + "Bft Day:" + weather[forecast_low_values[2][k]] + " Night:" + weather[forecast_low_values[3][k]] + " Temp:" + forecast_low_values[5][k]  + "C Rain:" + rain_prop[extreme_values[2][k]] + " Decoder Status:" + forecast_low_values[6][k]);
    }
    Serial.println("Anomaly:");
    for (int k = 0; k < 4; k++) {
      if (forecast_high_values[4][k] == 1) {//day x:weather Anomaly
        if (forecast_high_values[6][k] == 10)Serial.println("Day:" + String(k + 1) + " Anomaly:" + anomaly_1[forecast_high_values[4][k]] + " Risk:" + anomaly_1[extreme_values[0][k]] + " Next:" + anomaly_2[extreme_values[1][k]]);
      }
      if (forecast_high_values[6][k] == 10)Serial.println("Day:" + String(k + 1) + " No Anomaly");
    }
    Serial.println();
    //------------------------------------
  }
}
//---------------------------------------------------------------------
void send_text_to_vfd(String input_text) {//Horizontal Scrollmode

  int len = input_text.length();
  delay(10);
  Serial1.write(0x0C);//Clear display
  delay(10);
  Serial1.write(0x0B);//home position
  vfd_upper_line = vfd_lower_line;
  delay(10);
  Serial1.print(vfd_upper_line);

  delay(10);
  Serial1.write(0x0B);//home position
  delay(10);
  Serial1.write(0x0A); //move cursor down
  delay(10);
  Serial1.write(0x0D);//move cursor left end
  vfd_lower_line = input_text;
  delay(10);
  Serial1.print(vfd_lower_line);

}
//---------------------------------------------------------------------
void show_forecast_vfd() {

  if (vfd_display == true) {
    int len;
    vfd_counter++;

    if (vfd_counter > 31 ) vfd_counter = 0;
    //-------------------------------------------
    if (vfd_counter >= 0 && vfd_counter <= 11) {//Hoechstwerte

      if (vfd_counter == 0 || vfd_counter == 3 || vfd_counter == 6 || vfd_counter == 9) {
        if (vfd_counter == 0 )day_counter = 0;
        String area = String(region[user_region]);
        len = area.length();
        area = area.substring(3, len);
        String vfd_text = area + " Tag " + String(day_counter + 1) + " Hoch";
        len = vfd_text.length();
        if (len > 20) vfd_text = vfd_text.substring(0, 20);
        send_text_to_vfd(vfd_text);
      }

      if (vfd_counter == 1 || vfd_counter == 4 || vfd_counter == 7 || vfd_counter == 10) {
        if (forecast_high_values[6][day_counter] == 10) { //decoder status
          String vfd_text = String(forecast_high_values[5][day_counter]) + "C " + String(windstrength[forecast_high_values[1][day_counter]]) + "Bft " + String(winddirection[forecast_high_values[0][day_counter]]) + "+";
          len = vfd_text.length();
          if (len > 20) vfd_text = vfd_text.substring(0, 20);
          send_text_to_vfd(vfd_text);
        }
        else {
          send_text_to_vfd("n/a");
        }
      }

      if (vfd_counter == 2 || vfd_counter == 5 || vfd_counter == 8 || vfd_counter == 11) {
        if (forecast_high_values[6][day_counter] == 10) { //decoder status
          String vfd_text = "T:" + String(weather[forecast_high_values[2][day_counter]]) + " N:" + String(weather[forecast_high_values[3][day_counter]]);
          len = vfd_text.length();
          if (len > 20) vfd_text = vfd_text.substring(0, 20);
          send_text_to_vfd(vfd_text);
          day_counter++;
        }
        else {
          send_text_to_vfd("n/a");
          day_counter++;
        }
      }
    }
    //---------------------------------------------
    if (vfd_counter >= 12 && vfd_counter <= 23) {//Tiefstwerte

      if (vfd_counter == 12 || vfd_counter == 15 || vfd_counter == 18 || vfd_counter == 21) {
        if (vfd_counter == 12 ) day_counter = 0;
        String area = String(region[user_region]);
        len = area.length();
        area = area.substring(3, len);
        String vfd_text = area + " Tag " + String(day_counter + 1) + " Tief";
        len = vfd_text.length();
        if (len > 20) vfd_text = vfd_text.substring(0, 20);
        send_text_to_vfd(vfd_text);
      }

      if (vfd_counter == 13 || vfd_counter == 16 || vfd_counter == 19 || vfd_counter == 22) {
        if (forecast_low_values[6][day_counter] == 10) { //decoder status
          String vfd_text = String(forecast_low_values[5][day_counter]) + "C " + String(windstrength[forecast_low_values[1][day_counter]]) + "Bft " + String(winddirection[forecast_low_values[0][day_counter]]) + "+";
          len = vfd_text.length();
          if (len > 20) vfd_text = vfd_text.substring(0, 20);
          send_text_to_vfd(vfd_text);
        }
        else {
          send_text_to_vfd("n/a");
        }
      }
      if (vfd_counter == 14 || vfd_counter == 17 || vfd_counter == 20 || vfd_counter == 23) {
        if (forecast_low_values[6][day_counter] == 10) { //decoder status
          String vfd_text = "T:" + String(weather[forecast_low_values[2][day_counter]]) + " N:" + String(weather[forecast_low_values[3][day_counter]]);
          len = vfd_text.length();
          if (len > 20) vfd_text = vfd_text.substring(0, 20);
          send_text_to_vfd(vfd_text);
          day_counter++;
        }
        else {
          send_text_to_vfd("n/a");
          day_counter++;
        }
      }
    }
    //---------------------------------------------
    if (vfd_counter >= 24 && vfd_counter <= 31 ) { //Extreme
      if (vfd_counter == 24 || vfd_counter == 26 || vfd_counter == 28 || vfd_counter == 30) {
        if (vfd_counter == 24) day_counter = 0;
        String area = String(region[user_region]);
        len = area.length();
        area = area.substring(3, len);
        String vfd_text = area + " Tag " + String(day_counter + 1) + " Anom";
        len = vfd_text.length();
        if (len > 20) vfd_text = vfd_text.substring(0, 20);
        send_text_to_vfd(vfd_text);
      }

      if (vfd_counter == 25 || vfd_counter == 27 || vfd_counter == 29 || vfd_counter == 31) {
        if (forecast_low_values[6][day_counter] == 10) { //decoder status
          String vfd_text;
          if (extreme_values[0][day_counter] > 0) {
            vfd_text  = "K " + String(anomaly_1[extreme_values[0][day_counter]]) + " " + String(anomaly_2[extreme_values[1][day_counter]]) + " R " + String(rain_prop[extreme_values[2][day_counter]]);
          }
          else {
            vfd_text = String(anomaly_1[extreme_values[0][day_counter]]) + " R " + String(rain_prop[extreme_values[2][day_counter]]);
          }
          len = vfd_text.length();
          if (len > 20) vfd_text = vfd_text.substring(0, 20);
          send_text_to_vfd(vfd_text);
          day_counter++;
        }
        else {
          send_text_to_vfd("n/a");
          day_counter++;
        }
      }
    }
  }
}
//---------------------------------------------------------------------
int string_to_int(int a, int b) {           //   substring.meteodata > int val

  int value = 0;
  for (int i = 0; i < 16; i++) {
    if (! meteodata.substring(a, b) [i]) {
      break;
    }
    value <<= 1;
    value |= (meteodata.substring(a, b) [i] == '1') ? 1 : 0;
  }
  return value;
}
//---------------------------------------------------------------------
int reverse_bits(int x, int z)     // bit reverse int val length z   (last to first bit)
{
  int y = 0;
  for (int i = 0; i < z; ++i)
  {
    y <<= 1;
    y |= (x & 1);
    x >>= 1;
  }
  return y;
}
//---------------------------------------------------------------------
void timer1_subroutine(void) {
  y++;
  if (y > 2000) { // 9sec
    timer1_event = true;
    y = 0;
  }
}
