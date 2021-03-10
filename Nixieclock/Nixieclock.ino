#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define DATA D0
#define CLK D1
#define OE D2
#define CSS D3
#define CSM D4
#define CSH D5

const char *ssid     = "SSID";
const char *password = "password";

const long utcOffsetInSeconds = 3600;
int monthDay = 0;
int currentMonth;
String currentMonthName = "";
int currentYear = 0;
int oldhour = 0;

int testhour = 0;
int testminute = 0;
int testsecond = 0;
int testhourzehner = 0;
byte testhoureiner = 0;
byte testminutezehner = 0;
byte testminuteeiner = 0;
byte testsecondzehner = 0;
byte testsecondeiner = 0;
byte shifthour = 0;
byte shiftminute = 0;
byte shiftsecond = 0;

boolean dst = 0;

char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
String months[12]={"Januar", "Februar", "Maerz", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup(){
  //set pins to output because they are addressed in the main loop
  pinMode(CSS, OUTPUT);
  pinMode(CSM, OUTPUT);
  pinMode(CSH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(OE, OUTPUT);
  digitalWrite(OE, LOW);
  digitalWrite(CSS, LOW);
  digitalWrite(CSM, LOW);
  digitalWrite(CSH, LOW);
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  monthDay = ptm->tm_mday; 
  currentMonth = ptm->tm_mon+1;
  currentMonthName = months[currentMonth-1];
  currentYear = ptm->tm_year+1900;
  dst = summertime_EU(currentYear, currentMonth, monthDay, timeClient.getHours(), 1);
  timeClient.setTimeOffset(3600 + dst * 3600);
  writeTime(0,0,0);
  digitalWrite(OE, HIGH);
}

void loop() {
  timeClient.update();

  // check dst every hour
  if (oldhour != timeClient.getHours()){
    dst = summertime_EU(currentYear, currentMonth, monthDay, timeClient.getHours(), 1);
    timeClient.setTimeOffset(3600 + dst * 3600);
  }

  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  if (timeClient.getHours()<10){
    Serial.print("0");
  }
  Serial.print(timeClient.getHours());
  oldhour = timeClient.getHours();
  Serial.print(":");
    if (timeClient.getMinutes()<10){
    Serial.print("0");
  }
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
    if (timeClient.getSeconds()<10){
    Serial.print("0");
  }
  Serial.println(timeClient.getSeconds());
  //Serial.println(timeClient.getFormattedTime());

  // time in byte
  testhour = timeClient.getHours();
  testminute = timeClient.getMinutes();
  testsecond = timeClient.getSeconds();
  testhourzehner = (testhour/10)%10;
  testhoureiner = testhour%10;
  testminutezehner = (testminute/10)%10;
  testminuteeiner = testminute%10;
  testsecondzehner = (testsecond/10)%10;
  testsecondeiner = testsecond%10;
  shifthour = (testhourzehner << 4) | testhoureiner;
  shiftminute = (testminutezehner << 4) | testminuteeiner;
  shiftsecond = (testsecondzehner << 4) | testsecondeiner;
  Serial.println(shifthour,BIN);
  Serial.println(shiftminute,BIN);
  Serial.println(shiftsecond,BIN);
  writeTime(shifthour, shiftminute, shiftsecond);
  delay(1000);
  if ((shifthour == 3) && (shiftminute == 0) && (shiftsecond == 0))
  {
    cyclenumbers();
  }
}

void writeTime(byte hour, byte minute, byte second)
{
  digitalWrite(CSS, HIGH);
  //delay(100);
  shiftOut(DATA, CLK, MSBFIRST, second);
  //delay(100);
  digitalWrite(CSS, LOW);
  //delay(100);
  digitalWrite(CSM, HIGH);
  //delay(100);
  shiftOut(DATA, CLK, MSBFIRST, minute);
  //delay(100);
  digitalWrite(CSM, LOW);
  //delay(100);
  digitalWrite(CSH, HIGH);
  //delay(100);
  shiftOut(DATA, CLK, MSBFIRST, hour);
  //delay(100);
  digitalWrite(CSH, LOW);
  //delay(100);
  
}

void cyclenumbers()
{
  byte cycle = 0;
  unsigned long previousMillis = 0;
  const long interval = 60000;
  while(cycle <= 100 )
  {
    writeTime(cycle, cycle, cycle);
    
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) 
    {
      previousMillis = currentMillis;
      cycle+=11;
    }
  }
}

boolean summertime_EU(int _year, byte _month, byte _day, byte _hour, byte _tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
  if (_month < 3 || _month > 10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (_month > 3 && _month < 10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (_month == 3 && (_hour + 24 * _day) >= (1 + _tzHours + 24 * (31 - (5 * _year / 4 + 4) % 7)) || _month == 10 && (_hour + 24 * _day) < (1 + _tzHours + 24 * (31 - (5 * _year / 4 + 1) % 7)))
    return true;
  else
    return false;
}                                                  // End of function summertime_EU
