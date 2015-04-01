

//RTC (Real Time Clock), Solar Path Calculator and Linear Actuator Driver
//Solar equations from: Solar Calculation Details National Oceanic & Atmospheric Administration (NOAA) and  http://www.jgiesen.de/astro/suncalc/calculations.htm 
//RTC copyrighted (C) 2011 but open source code from:  http://bildr.org/?s=rtc
//Solar Path calcualtion software method written by Andrew Stevens and Professor Jon Stevens for APCS 370, Systems Implmenation Project
//APCS 370 Scrum Team:  Dani Breon, Jerry Groom, Will Smead, Curt Coughlin, Marshawn Lacy, and Brian Pittman
//Professor Mike Doolan, Instructor Andrew Nieuwsma and previous classes of APCS 360 and 370 developed earlier versions, 2012 - 14.
//Spring Term 2015, APCS Department, William Penn University
  
  #include <Wire.h>
  #define PI 3.141592653589793238462643
  #define DS3231_ADDRESS 0x68
  byte zero = 0x00; 
  
  int    monthDay = 0;      int    minute = 0;        int year = 0;        int weekDay;  
  int    hour = 0;          int    month = 0;         int second;   
  double offset = 0;        double doy = 0;           int targetX = 0;     int targetY = 0;
  double lat = 41.3;        double longitude = 92.6; //actually -92.6
  double radDate = 0;       double eqtime = 0;        
  double declin = 0;        double time_offset = 0;
  double tst = 0;           double ha = 0;
  double altitude = 0;      double azimuth = 0;       double zenith = 0;
  double snoon = 0;         double sun_set = 0;       double sun_rise = 0;      
  double hars = 0;          double d2r = PI/180; 

void setup(){
   Wire.begin();
   Serial.begin(9600);\
   Serial1.begin(9600);
}

void loop(){ 
  //setDateTime();
  getDate();
  solar_path();
  printDate();
  alt(hour, minute);
  azm(hour, minute);
  MoveX(targetX);
  MoveY(targetY);
  Serial.print("Decimal Time  \t  "); Serial.println(hour + minute/60.0); Serial.println("");
   delay(10000);
}

void getDate(){ 
  Wire.beginTransmission(DS3231_ADDRESS); Wire.write(zero); Wire.endTransmission(); Wire.requestFrom(DS3231_ADDRESS, 7);
  second = bcdToDec(Wire.read()); minute = bcdToDec(Wire.read()); hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  weekDay = bcdToDec(Wire.read()); /*0-6 -> sunday - Saturday*/ monthDay = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read()); year = bcdToDec(Wire.read()); 
}
  void printDate(){
  //print the date EG   3/1/11 23:59:59
  if (month < 10)    Serial.print("0");       Serial.print(month);      Serial.print("/"); 
  if (monthDay < 10) Serial.print("0");       Serial.print(monthDay);   Serial.print("/");    
                     Serial.print(year);      Serial.print(" ");
  if (hour < 10)     Serial.print("0");       Serial.print(hour);       Serial.print(":");
  if (minute < 10)   Serial.print("0");       Serial.print(minute);     Serial.print(":");
  if (second < 10)   Serial.print("0");       Serial.print(second);     Serial.print("  ");   
                     Serial.print("Day of Week ");                      Serial.print(dayofweek(weekDay));
                     Serial.print("  DOY ");  Serial.println(doy);      Serial.println("");
 }

void solar_path(){
  doy = (dayofyear(month, monthDay));
  if (doy >= 67.0834 || doy <= 305.0834) {offset = -5;} else {offset = -6;} //DST or CST for 2015
  doy = doy - 1 + (hour-12)/24.0 + minute/1440.0 + offset/24.0;
  radDate = (2.0*PI/365.0)*(doy); //Serial.print("radDate " ); Serial.println( radDate );
 
  eqtime = 229.18*(0.000075+0.001868*cos(radDate)-0.032077*sin(radDate)-0.014615*cos(2*radDate)-0.040849*sin(2*radDate));
   //Serial.print("eqtime " ); Serial.println( eqtime );

  declin = 0.006918 - 0.399912*cos(radDate) + 0.070257*sin(radDate) - 0.006758*cos(2*radDate) + 0.000907*sin(2*radDate) - 0.002697*cos(3*radDate) + 0.00148*sin(3*radDate);
  declin = declin/d2r;  //Serial.print("declin " ); Serial.println( declin  );

  time_offset = eqtime - 4*longitude - 60*offset;  //Serial.print("time_offset " ); Serial.println( time_offset );
  tst = hour*60 + minute + time_offset;   //Serial.print("tst/60 " ); Serial.println( tst/60  );
  ha = tst/4 - 180;  //Serial.print("ha " ); Serial.println( ha  );

  zenith = sin(d2r*lat)*sin(d2r*declin)+cos(d2r*lat)*cos(d2r*declin)*cos(d2r*ha);
  zenith = acos(zenith)/d2r;  altitude = 90-zenith;// altitude
   Serial.print("altitude  \t "  ); Serial.println( altitude );

  azimuth = -(sin(d2r*lat)*cos(d2r*zenith) - sin(d2r*declin))/(cos(d2r*lat)*sin(d2r*zenith));
  azimuth = acos(azimuth)/d2r;
  if(ha>=0) azimuth = 360 - azimuth;
   Serial.print("azimuth  \t " ); Serial.println( azimuth  );

  hars = cos(d2r*90.833)/(cos(d2r*lat)*cos(d2r*declin)) - tan(d2r*lat)*tan(d2r*declin);  hars = acos(hars)/d2r;
   //Serial.print("hars " ); Serial.println( hars  );

  sun_rise = 720 + 4*(longitude - hars) - eqtime; sun_rise = sun_rise/60 + offset; 
   Serial.print("sun_rise \t " ); Serial.println( sun_rise );

  sun_set = 720 + 4*(longitude + hars) - eqtime; sun_set = sun_set/60 + offset; 
   Serial.print("sun_set \t " ); Serial.println( sun_set );

  snoon = 720 - 4 * -longitude - eqtime; snoon = snoon / 60 + offset; 
   Serial.print("sun_noon \t " ); Serial.println( snoon ); Serial.println("");
}
 
void alt(int hour, int minute) {
  if(hour + minute/60.0 <= sun_rise || hour + minute/60.0 >= sun_set) {targetY = 200;}
    else {targetY = altitude/90.0 * 3600.0 + 200;} Serial.print( "Target Y  \t  " ); Serial.println(targetY);
}
void azm(int hour, int minute) {
  if(hour + minute/60.0 <= sun_rise || hour + minute/60.0 >= sun_set) {targetX = 200;}
    else {targetX = azimuth/360.0 * 3600.0 + 200;} Serial.print( "Target X  \t  " ); Serial.println(targetX); Serial.println("");
}

//sets the new target for the azimuth JRK12v12 controller, this uses pololu high resulution protocol
void MoveX(int x) { //azimuth
  word target = x;  //only pass ints
  Serial1.write(0xAA); //tells the controller we're starting to send it commands
  Serial1.write(0xB);   //This is the pololu device # you're connected too that is found in the config utility(converted to hex). I'm using #11 in this example
  Serial1.write(0x40 + (target & 0x1F)); //first half of the target, see the pololu jrk manual for more specifics
  Serial1.write((target >> 5) & 0x7F);   //second half of the target, " " " 
}  

//sets the new target for the altitude JRK12v12 controller, this uses pololu high resulution protocol
void MoveY(int y) { //altiutude
  word target = y;  //only pass ints
  Serial1.write(0xAA); //tells the controller we're starting to send it commands
  Serial1.write(0xC);   //This is the pololu device # you're connected too that is found in the config utility(converted to hex). I'm using #11 in this example
  Serial1.write(0x40 + (target & 0x1F)); //first half of the target, see the pololu jrk manual for more specifics
  Serial1.write((target >> 5) & 0x7F);   //second half of the target, " " " 
}  

double dayofyear(int month, int monthDay){
  switch (month) {
    case 1:  doy = monthDay; break;  
    case 2:  doy = 31 + monthDay; break;  
    case 3:  if (year % 4 == 0) {doy =  60 + monthDay; break;} else { doy =  59 + monthDay; break; } // If Leap year....
    case 4:  if (year % 4 == 0) {doy =  91 + monthDay; break;} else { doy =  90 + monthDay; break; } 
    case 5:  if (year % 4 == 0) {doy = 121 + monthDay; break;} else { doy = 120 + monthDay; break; }
    case 6:  if (year % 4 == 0) {doy = 152 + monthDay; break;} else { doy = 151 + monthDay; break; }
    case 7:  if (year % 4 == 0) {doy = 182 + monthDay; break;} else { doy = 181 + monthDay; break; } 
    case 8:  if (year % 4 == 0) {doy = 213 + monthDay; break;} else { doy = 212 + monthDay; break; }
    case 9:  if (year % 4 == 0) {doy = 244 + monthDay; break;} else { doy = 243 + monthDay; break; }
    case 10: if (year % 4 == 0) {doy = 274 + monthDay; break;} else { doy = 273 + monthDay; break; } 
    case 11: if (year % 4 == 0) {doy = 305 + monthDay; break;} else { doy = 304 + monthDay; break; }
    case 12: if (year % 4 == 0) {doy = 335 + monthDay; break;} else { doy = 334 + monthDay; break; } //default: 
     return  doy;  
}}

String dayofweek(int weekDay){
   String weekday = "";
   switch (weekDay) {
    case 1: weekday = "Sunday";    break;  case 2: weekday = "Monday";   break;   case 3: weekday = "Tuesday";  break; 
    case 4: weekday = "Wednesday"; break;  case 5: weekday = "Thursday"; break;   case 6: weekday = "Friday";   break; 
    case 7: weekday = "Saturday";  break;
      return weekday;
}}

void setDateTime(){
  byte second =      30; //0-59 
  byte minute =      17; //0-59
  byte hour =        19; //0-23
  byte weekDay =      1; //1-7
  byte monthDay =    15; //1-31
  byte month =        3; //1-12
  byte year  =       15; //0-99

  Wire.beginTransmission(DS3231_ADDRESS); Wire.write(zero); //start Oscillator  
  Wire.write(decToBcd(second)); Wire.write(decToBcd(minute)); Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay)); Wire.write(decToBcd(monthDay)); Wire.write(decToBcd(month));
  Wire.write(decToBcd(year)); Wire.write(zero); Wire.endTransmission(); //stop Oscillator
} 

byte decToBcd(byte val){ // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val){ // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}





