#include "NimBLEDevice.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

NimBLEScan* pBLEScan;
LiquidCrystal_I2C lcd(0x27, 20, 4);   

String tocheck1 = "fd:2d:52:3f:5f:90";
String tocheck2 = "4c000215ffffffffffffffffffffffffffffffe03f5f";
String tocheck3 = "fa:7b:2b:a3:e0:8d";
String tocheck4 = "4c000215ffffffffffffffffffffffffffffffe0a3e08d";
String add, dat;

float PSI1 = 0.00;
float PSI2 = 0.00;

const int FuelSensor_ADC = 34;

int  sensorValue, fuelLevel, fuelLast; 
int viscocity;
String outputString;

const int tank_volume = 5;   // liters
const int milage      = 50;  // distance/liter
float distance = 0.00;

void PSIFinder(String data) 
{
  add = ""; dat = "";
  //Serial.print("REC>>> "); Serial.print(data); Serial.println();
  //Serial.print("Address = "); Serial.println(add); Serial.print("data = "); Serial.println(dat);
  add = data.substring(17,34); //Serial.print("Address = "); Serial.println(add);     
  
  if(tocheck1.equals(add)) 
  {
    //Serial.print("Address = "); Serial.println(add); Serial.println("Address Found");
    dat = data.substring(55,99); //Serial.print("data = "); Serial.println(dat);
    if(tocheck2.equals(dat))
    {
      //Serial.print("Hex Data = "); Serial.println(dat); Serial.println("Data Found");
      String HexPSI1 = data.substring(101,103);
      char temp[20];
      HexPSI1.toCharArray(temp, HexPSI1.length()+1);
      PSI1 = convert(temp);
      //Serial.print("PSI - L = "); Serial.println(PSI1);
    }
  }
  if(tocheck3.equals(add)) 
  {
    //Serial.print("Address = "); Serial.println(add); Serial.println("Address Found");
    dat = data.substring(55,101); // Serial.print("data = "); Serial.println(dat);   
    if(tocheck4.equals(dat))
    {
      //Serial.print("Hex Data = "); Serial.println(dat); Serial.println("Data Found");
      String HexPSI2 = data.substring(101,103);
      char temp[20];
      HexPSI2.toCharArray(temp, HexPSI2.length()+1);
      PSI2 = convert(temp);
      //Serial.print("PSI - R = "); Serial.println(PSI2);
    }
  }
}

int convert(char *s) 
{
  int x = 0;
  for(;;) {
    char c = *s;
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0'; 
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10; 
    }
    else if (c >= 'a' && c <= 'f') {
      x *= 16;
      x += (c - 'a') + 10;
    }
    else break;
    s++;
  }
  return x;
}

class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) 
    {
      String data = advertisedDevice->toString().c_str();
      PSIFinder(data);
     }
};

void setup() 
{
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(2,1);  lcd.print("SMART INFO SYSTEM");
  lcd.setCursor(6,2);  lcd.print("FOR BIKE");
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  delay(5500);

  Serial.println("SMART INFO SYSTEM FOR BIKE");
  NimBLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DATA);
  NimBLEDevice::setScanDuplicateCacheSize(1000);
  NimBLEDevice::init("");

  pBLEScan = NimBLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
  pBLEScan->setActiveScan(true); // Set active scanning, this will get more data from the advertiser.
  pBLEScan->setInterval(50); // How often the scan occurs / switches channels; in milliseconds, 97
  pBLEScan->setWindow(40); // How long to scan during the interval; in milliseconds. 37
  pBLEScan->setMaxResults(0); // do not store the scan results, use callback only.
  lcd.clear();
}

void loop() 
{
  if(pBLEScan->isScanning() == false) { pBLEScan->start(0, nullptr, false); }
  
  get_fuel_level();
  calculate_distance();

  display_lcd();
  combine_data();
  delay(1500);  
}

void get_fuel_level()
{
  sensorValue = analogRead(FuelSensor_ADC);
  fuelLevel = map(sensorValue, 3000, 1100, 100, 0);
  if(fuelLevel <= 0) { fuelLevel = 0; }
  if (fuelLevel <= 0)        { fuelLast = 0;  }
  else if (fuelLevel <= 10)  { fuelLast = 1;  }
  else if (fuelLevel <= 20)  { fuelLast = 2;  }
  else if (fuelLevel <= 30)  { fuelLast = 3;  }
  else if (fuelLevel <= 40)  { fuelLast = 4;  }
  else if (fuelLevel <= 50)  { fuelLast = 5;  }
  else if (fuelLevel <= 60)  { fuelLast = 6;  }
  else if (fuelLevel <= 70)  { fuelLast = 7;  }
  else if (fuelLevel <= 80)  { fuelLast = 8;  }
  else if (fuelLevel <= 90)  { fuelLast = 9;  }
  else if (fuelLevel >= 100) { fuelLast = 10; }
  //Serial.print(sensorValue);  Serial.print("   Fuel Level: "); Serial.println(fuelLast);
}

void  calculate_distance()
{
  distance = tank_volume * fuelLevel *  milage * 0.01;
}

void combine_data()
{
  viscocity = 1;
  outputString = String(fuelLast) + "**" + String(viscocity)  + "**" +  String(PSI1) + "**" + String(PSI2) + "**" + String(distance);
  Serial.println(outputString);
  Serial2.println(outputString);
}

void display_lcd()
{
  lcd.setCursor(0,0);  lcd.print("F:");  lcd.print(PSI1,0);  lcd.print(" psi"); lcd.print("    R:");  lcd.print(PSI2,0);  lcd.print(" psi"); 
  //lcd.setCursor(0,1);  lcd.print("P2:");  lcd.print(PSI2,0);  lcd.print(" psi");
  lcd.setCursor(0,1);  lcd.print("Fuellavel:");  lcd.print(fuelLast,1); lcd.print(" % ");
  lcd.setCursor(0,2);  lcd.print("Distance:");  lcd.print(distance,1); lcd.print(" Km/lit "); 
  lcd.setCursor(0,3);  lcd.print("Viscocity:");  lcd.print(viscocity,3); lcd.print(" cSt "); 

}
