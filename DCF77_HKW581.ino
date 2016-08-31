//Decode encrypted Meteotime Data from HKW581:

#include "DCF77.h"
#include "TimeLib.h"

#define led 13
#define DCF_PIN 2           // Connection pin to DCF 77 device
#define DCF_INTERRUPT 0    // Interrupt number associated with pin

time_t time;
DCF77 DCF = DCF77(DCF_PIN, DCF_INTERRUPT);

//60 Region,Cities:
const char *region[]  { "F Bordeaux", "F la Rochelle" , "F Paris", "F Brest", "F Clermont-Ferrand", "F Beziers", "B Bruxelles", "F Dijon", "F Marseille", "F Lyon", "F Grenoble", "CH La Chaux de Fonds"
  , "D Frankfurt am Main", "D Trier", "D Duisburg", "GB Swansea", "GB Manchester", "F le Havre", "GB London", "D Bremerhaven", "DK Herning", "DK Arhus", "D Hannover", "DK Copenhagen"
  , "D Rostock" , "D Ingolstadt", "D Muenchen", "I Bolzano", "D Nuernberg", "D Leipzig", "D Erfurt", "CH Lausanne", "CH Zuerich", "CH Adelboden", "CH Sion", "CH Glarus", "CH Davos"
  , "D Kassel", "CH Locarno", "I Sestriere" , "I Milano", "I Roma", "NL Amsterdam", "I Genova", "I Venezia", "D Strasbourg", "A Klagenfurt", "A Innsbruck", "A Salzburg", "A/SK Wien/Bratislava"
  ,  "CZ Praha", "CZ Decin", "D Berlin", "S Gothenburg" , "S Stockholm", "S Kalmar", "S Joenkoeping", "D Donaueschingen", "N Oslo", "D Stuttgart" , "I Napoli", "I Ancona", "I Bari"
  , "H Budapest", "E Madrid", "E Bilbao", "I Palermo", "E Palma de Mallorca" , "E Valencia", "E Barcelona", "AND Andorra", "E Sevilla", "P Lissabon", "I Sassari", "E Gijon", "IRL Galway"
  , "IRL Dublin", "GB Glasgow", "N Stavanger", "N Trondheim", "S Sundsvall", "PL Gdansk" , "PL Warszawa", "PL Krakow", "S Umea", "S Oestersund", "CH Samedan", "HR Zagreb", "CH Zermatt", "HR Split"
};
const char *weather[]  {  "Reserved", "Sunny", "Partly clouded", "Mostly clouded", "Overcast", "Heat storms", "Heavy rain", "Snow", "Fog", "Sleet", "Rain showers", "Light rain" , "Snow showers",  "Frontal storms", "Stratus cloud", "Sleet storms"};
const char *heavyweather[]  {  "None", "Heavy Weather 24 hrs.", "Heavy weather Day", "Heavy weather Night", "Storm 24hrs.", "Storm Day", "Storm Night",
  "Wind gusts Day", "Wind gusts Night", "Icy rain morning", "Icy rain evening", "Icy rain night", "Fine dust", "Ozon", "Radiation", "High water"
};
const char *probprecip[]  {"0 %", "15 %", "30 %", "45 %", "60 %", "75 %", "90 %", "100 %"};
const char *winddirection[]  { "N", "NO", "O", "SO", "S", "SW", "W", "NW", "Changeable", "Foen", "Biese N/O", "Mistral N", "Scirocco S", "Tramont W", "Reserved", "Reserved"};
const char *windstrength[]  {"0", "0-2", "3-4", "5-6", "7", "8", "9", ">=10"};
const char *anomaly1[]  {"Same Weather ", "Jump 1 ", "Jump 2 ", "Jump 3 "};
const char *anomaly2[]  {"0-2 hrs", "2-4 hrs", "5-6 hrs", "7-8 hrs"};

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
//minute 2: first 14 bits                      & time & hour  & date & month & dow & year
//01011010010110 11100011000111 00001111110011 10011010 01000100 11100 011     00101000
//01000110 00001000 11001110

//HKW581 Pins:     chip needs 40 seconds to warm up
#define Qout 3
#define DataIn 4
#define BFR 5
#define DataOut 6
#define ClockIn 7

//----------------------------------------------------------------------------------------------
void setup() {

  pinMode(led, OUTPUT);

  pinMode(DataIn, OUTPUT);
  pinMode(DataOut, INPUT);
  pinMode(ClockIn, OUTPUT);
  pinMode(Qout, INPUT);
  pinMode(BFR, INPUT);
  digitalWrite(DataIn, LOW);
  digitalWrite(ClockIn, LOW);

  Serial.begin(9600);
  DCF.Start();
  Serial.println("Waiting for DCF77 time ... ");
  Serial.println("It will take at least 2 minutes until a first update can be processed.");
  //setTime(13, 02, 00, 31, 12, 2016);
  //   0-3,   4-7,    8-11,   12-14,              15, 16-21,          22-23,
  //   day, night, winddir, windstr,  weatheranomaly,  temp,  decoder state,

  meteodata = "111001101100011010110110"; //Test: 24bits: Snow, Heavy rain, SO, 9-Bf, No, 23⁰c


}
//----------------------------------------------------------------------------------------------
void loop() {

  boolean val = digitalRead(DCF_PIN);
  if (val == LOW) digitalWrite(led, LOW);
  if (val == HIGH) digitalWrite(led, HIGH);

  time_t DCFtime = DCF.getTime(); // Check if new DCF77 time is available
  if (DCFtime != 0)  {
    Serial.println("Time is updated");
    setTime(DCFtime);
  }

  if (second() == 59 && lock == false) {

    lock = true;
    int result = (minute() + 1) % 3;
    String dcf_bitstream = DCF.getEncWeatherData();
    Serial.println("Bitstream:" + String(dcf_bitstream));
    collect_data(dcf_bitstream, result); //result can 0=final,2=2.,1=1.packet

    if (result == 0 ) {
      //write_data_to_hkw(); // input > hkw_in[]
      //read_data_from_hkw(); // result > meteodata
      show_region();
    }
  }

  if (second() == 58) {
    lock = false;
  }
}
//----------------------------------------------------------------------------------------------
void collect_data(String input, int packet) { //packet can 0=final,2=2.,1=1. packet

  int bit_count;
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
  }

  if (packet == 0) { //final
    bit_count = 28;
    for (int k = 0; k < 14; k++) {
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }

    for (int k = 21; k < 29; k++) {//minute
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    for (int k = 29; k < 36; k++) {//hour
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    for (int k = 36; k < 42; k++) {//date
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    for (int k = 45; k < 50; k++) {//month
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    for (int k = 42; k < 45; k++) {//dow
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    for (int k = 50; k < 58; k++) {//year
      if (input.substring(k, k + 1) == "0")hkw_in[bit_count] = 0;
      if (input.substring(k, k + 1) == "1")hkw_in[bit_count] = 1;
      bit_count++;
    }
    for (int k = 0; k < 82; k++) {
      Serial.print(String(hkw_in[k]));
    }
    Serial.println();
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

  Serial.println("Write 82  bits to Chip:");
  Serial.println("Datapackage 1  Datapackage 2  Datapackage 3  Time  Hour  Date  Month  Dow  Year");

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
}
//----------------------------------------------------------------------------------------------
void show_region() {

  int region_code = (((hour() % 3) * 60) + minute()); //every 3 hours full round
  //00:00 = 0,  02:59 = 2*60 + 59 = 179
  region_code = region_code / 3;

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
  Serial.println();
  Serial.print("Location: ");
  Serial.println(region[region_code]);
}
//----------------------------------------------------------------------------------------------
void show_data() {

  Serial.println("Start:");
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Winddirection     =   ");
  Serial.print(meteodata.substring(8, 12));      //   0-3,   4-7,    8-11,   12-14,              15, 16-21,          22-23,
  Serial.print(" ");                             //   day, night, winddir, windstr,  weatheranomaly,  temp,  decoder state,
  val = string_to_int(8, 12);
  val = reverse_bits (val, 4);
  Serial.print(val, HEX);
  Serial.print(" ");
  Serial.print(winddirection[val]);
  Serial.println();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Windstrength      =    ");
  Serial.print(meteodata.substring(12, 15));
  Serial.print(" ");
  val = string_to_int(12, 15);
  val = reverse_bits (val, 3);
  Serial.print(val, HEX);
  Serial.print(" ");
  Serial.print(windstrength[val]);
  Serial.println(" Bft");
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Day               =   ");
  Serial.print(meteodata.substring(0, 4));
  Serial.print(" ");
  val = string_to_int(0, 4);
  val = reverse_bits (val, 4);
  Serial.print(val, HEX);
  Serial.print(" ");
  Serial.print(weather[val]);
  Serial.println();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Night             =   ");
  Serial.print(meteodata.substring(4, 8));
  Serial.print(" ");
  val = string_to_int(4, 8);
  val = reverse_bits (val, 4);
  Serial.print(val, HEX);
  Serial.print(" ");
  if (val == 1) {
    Serial.print("Clear");
    Serial.println();
  } else {
    Serial.print(weather[val]);
    Serial.println();
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Weather Anomality =      ");
  Serial.print(meteodata.substring(15, 16));
  Serial.print(" ");
  val = string_to_int(15, 16);
  Serial.print(val, HEX);
  if (val == 1) {
    Serial.println(" Yes");
  } else {
    Serial.println(" No");
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Temperature       = ");
  Serial.print(meteodata.substring(16, 22));
  Serial.print(" ");
  val = string_to_int(16, 22);
  val = reverse_bits (val, 6);
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
  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Decoder status    =     ");
  Serial.print(meteodata.substring(22, 24));
  if (meteodata.substring(22, 24) == "10") {
    Serial.println (" OK");
  } else {
    Serial.println (" NOT OK");
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
