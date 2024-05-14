// c0ded by Amos Siregar 
// ZETA CONVERTER MPPT FIRMWARE v1.0

//========== LOADING LIBRARY ==========//
#include <ESP32Time.h>
#include <WiFi.h>
#include "time.h"
#include <Wire.h> 
#include <SPI.h>
#include <SD.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 4);
/**
 * @fn DFRobot_INA219_IIC
 * @brief pWire I2C controller pointer
 * @param i2caddr  I2C address
 * @n INA219_I2C_ADDRESS1  0x40   A0 = 0  A1 = 0
 * @n INA219_I2C_ADDRESS2  0x41   A0 = 1  A1 = 0
 * @n INA219_I2C_ADDRESS3  0x44   A0 = 0  A1 = 1
 * @n INA219_I2C_ADDRESS4  0x45   A0 = 1  A1 = 1	 
  */
Adafruit_INA219 ina219_in(0x45);
Adafruit_INA219 ina219_out(0x40);
//========== USER PARAMETERS ==========//

#define GDRIVER     33
#define SD_SCK 14
#define SD_MISO 33
#define SD_MOSI 13
#define SD_CS 15
#define true 1
#define false 0
#define ON true
#define OFF false
SPIClass spi(HSPI);
ESP32Time rtc(25200);  // offset in seconds GMT+1

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

//========== WIFI PARAMETERS ==========//
const char* ssid     = "WIFI NAME";
const char* password =  "WIFI PASSWORD";

//========== SYSTEM CONFIG ==========//
bool startw                = true;     //start pwm initiate
int
charger_state         = 3,        //1 = fixedPWM, 2 = CV, 3 = MPPT  
pwmRes                = 10,
pwmFreq               = 30000,    // PWM Freq
PWM_MaxDC             = 99.9900,  // PWM Maximum %
baudRate              = 500000,   // Serial Baud Rate
pwm_Fixedvalue        = 758,      // Change this to set fixed value of PWM
inD                   = 380,      // Initialize PWM value 
loopm                 = 40,       // Loop count for serial monitor
monitorserial         = 1;        // 1 = All shown, 2 = After some loops 
float
desiredVout           = 12.000,    // use to set the fixed Vout
currentCharging       = 3.0000,
voutMax               = 14.8000,
voutMin               = 12.4000;

//========== SENSOR PARAMETERS ==========// 
int
avgVS                       = 8,
avgCS                       = 8;

//========== SYSTEM PARAMETERS ==========//
int
reb                         = 0,
trying                      = 0,
PWM                         = 0,
pwmMax                      = 0,
pwmMaxLimited               = 0,
pwmChannel                  = 0,
avgVsStore                  = 0,
avgCsStore                  = 0;

float
powerInput                  = 0.0000,
powerInputPrev              = 0.0000,
powerOutput                 = 0.0000,
powerOutputPrev             = 0.0000,
zetaEfficiency              = 0.0000,
voltageInput                = 0.0000,      // SYSTEM PARAMETER - Input voltage (solar voltage)
voltageInputPrev            = 0.0000,      // SYSTEM PARAMETER - Previously stored input voltage variable for MPPT algorithm
voltageOutput               = 0.0000,      // SYSTEM PARAMETER - Input voltage (battery voltage)
currentInput                = 0.0000,      // SYSTEM PARAMETER - Output power (battery or charing voltage)
currentOutput               = 0.0000,      // SYSTEM PARAMETER - Output current (battery or charing current in Amperes)
cJin                        = 0.0000,
pWin                        = 0.0000,
lWin                        = 0.0000,
cJout                       = 0.0000,
pWout                       = 0.0000,
lWout                       = 0.0000,
shuntvoltage_in             = 0.0000,
busvoltage_in               = 0.0000,
current_mA_in               = 0.0000,
loadvoltage_in              = 0.0000,
power_mW_in                 = 0.0000,
shuntvoltage_out            = 0.0000,
busvoltage_out              = 0.0000,
current_mA_out              = 0.0000,
loadvoltage_out             = 0.0000,
power_mW_out                = 0.0000,
pret                        = 0.0000;


int
xs        = 8,
loopsm    = 0;

#define SOLAR_ICON 6
byte solar_icon[8] = {  0b11111,  0b10101,  0b11111,  0b10101,  0b11111,  0b10101,  0b11111,  0b00000};
#define PWM_ICON 7
byte _PWM_icon[8]= {  0b11101,  0b10101,  0b10101,  0b10101,  0b10101,  0b10101,  0b10111,  0b00000,};
byte backslash_char[8]= {  0b10000,  0b10000,  0b01000,  0b01000,  0b00100,  0b00100,  0b00010,  0b00000,};

char joker[50];
char datejok[50];
char jamaja[50];

unsigned long previousMillis = 0;
const unsigned long interval = 3600000; // 1 hour in milliseconds
const int maxLoopsPerHour = 5000;
int loopCount = 0;
void setup() {
  
  Serial.begin(baudRate);                                   //Set serial baud rate
  Serial.println("> Serial Initialized");                   //Startup message
  lcd.begin();
  lcd.backlight();
  
  spi.begin(SD_SCK, SD_MISO, SD_MOSI);  // SCK, MISO, MOSI, SS
  while(!SD.begin(SD_CS, spi)) {
        Serial.println("Card Mount Failed");
        delay(2000);
    }

  uint8_t cardType = SD.cardType();
  while(cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        delay(2000);
    }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){ Serial.println("MMC");} 
  else if(cardType == CARD_SD){ Serial.println("SDSC"); } 
  else if(cardType == CARD_SDHC){ Serial.println("SDHC"); } 
  else { Serial.println("UNKNOWN"); }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("> WiFi connected.");
  Serial.println(WiFi.localIP());

 configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 struct tm timeinfo;
 while(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get Time Information");
        delay(100);
    }
 
 //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  rtc.setTimeStruct(timeinfo); 
  strftime(joker, sizeof(joker), "%d %B %Y", &timeinfo);
  strftime(datejok, sizeof(datejok), "%H:%M:%S", &timeinfo);
  Serial.println(joker);
  Serial.println(datejok);
  lcd.setCursor(0,0); lcd.print(joker);
  lcd.setCursor(0,1); lcd.print(datejok);
  delay(5000);
  lcd.clear();
  //PWM INITIALIZATION
  ledcSetup(pwmChannel,pwmFreq,pwmRes);          //Set PWM Parameters
  ledcAttachPin(GDRIVER, pwmChannel);                        //Set pin as PWM
  ledcWrite(pwmChannel,PWM);                                 //Write PWM value at startup (duty = 0)
  pwmMax = pow(2,pwmRes)-1;                           //Get PWM Max Bit Ceiling
  pwmMaxLimited = (PWM_MaxDC*pwmMax)/100.000;                //Get maximum PWM Duty Cycle (pwm limiting protection)

  while(ina219_in.begin() != true) {
        Serial.println("INA219 in begin failed");
        delay(2000);
    }
  while(ina219_out.begin() != true) {
        Serial.println("INA219 out begin failed");
        delay(2000);
    } 

  lcd.createChar(PWM_ICON,_PWM_icon);
  lcd.createChar(SOLAR_ICON,solar_icon);
  lcd.createChar('\\', backslash_char);
  lcd.setCursor(0, 0); lcd.print("SOL");
  lcd.setCursor(4, 0); lcd.write(SOLAR_ICON);
  lcd.setCursor(13, 0); lcd.print("PWM");
  lcd.setCursor(18, 0); lcd.write(PWM_ICON);
  showLCD();
  Serial.println("> MPPT HAS INITIALIZED");                //Startup message
  if(startw == 1){ 
    PWM = inD; 
    }
}

void showPrevlue(){
  Serial.print(" PIPrev:");    Serial.print(powerInputPrev,6);
  Serial.print(" POPrev:");    Serial.print(powerOutputPrev,6); 
}

void showinserial(){

  pret = PWM/(float)pwmMax*100.00;
  Serial.print(" PI:");    Serial.print(powerInput,6); 
  Serial.print(" PO:");    Serial.print(powerOutput,6); 
  Serial.print(" ZetaEff:");   Serial.print(zetaEfficiency); Serial.print("%"); 
  Serial.print(" PWM:");   Serial.print(PWM);
  Serial.print(" PWM%:");   Serial.print(pret); Serial.print("%"); 
  Serial.print(" VI:");    Serial.print(voltageInput,3); 
  Serial.print(" VO:");    Serial.print(voltageOutput,3); 
  Serial.print(" CI:");    Serial.print(currentInput,8); 
  Serial.print(" CO:");    Serial.print(currentOutput,8);
  Serial.print(" Movement:");  Serial.print(reb); 
  Serial.println("");
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if the interval (1 hour) has elapsed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    loopCount = 0; // Reset the loop counter when a new hour starts
  }
    struct tm timeinfo = rtc.getTimeStruct();
    strftime(joker, sizeof(joker), "%d %B %Y, %A", &timeinfo);
    strftime(datejok, sizeof(datejok), "%H:%M:%S", &timeinfo);
    strftime(jamaja, sizeof(jamaja), "%H", &timeinfo);

    int hour = atoi(jamaja);
    static int previousHour = -1;
    if (hour != previousHour) {
      previousHour = hour;
      loopCount = 0;
    }

    if (loopCount == 0){ PWM = inD; }

    if (loopCount < maxLoopsPerHour) {
        Serial.print("Loop Count: "); Serial.print(loopCount); Serial.print(" ");
        Serial.print(datejok); Serial.print(" ");
        read_Sensors();
        showPrevlue();
        Charging_Algorithm();
        String dataString = String(datejok)  + "," + String(powerInput,4) + "," + String(powerOutput,4) + "," + String(PWM)+ "," + String(pret)+ "," + String(voltageInput,4)+ "," + String(voltageOutput,4)+ "," + String(currentInput,8)+ "," + String(currentOutput,8);
        File dataFile = SD.open("/"+String(joker)+".csv", FILE_APPEND);
        if(dataFile) {
          dataFile.println(dataString);
          dataFile.close();
        } else {
          Serial.println("Failed to open data file for writing");
        }
        if (monitorserial == 1){ showinserial(); }
        else{
          if (loopsm == loopm){ showinserial(); loopsm = 0; }
          else{ loopsm++; }
        }
        if(xs == 10){ showLCD(); xs = 1; } 
        else { xs++; }

        

        loopCount++;
    } else {
      Serial.print("Loop Count: "); Serial.print(loopCount); Serial.print(" ");
      PWM = 0; 
      PWM_Modulation();
      read_Sensors();
      showPrevlue();
      showinserial();
      showLCD();
      unsigned long timeToWait = interval - (currentMillis - previousMillis);
      Serial.print("Waiting for ");
      Serial.print(timeToWait / 1000);
      Serial.println(" seconds until the next hour.");
      delay(timeToWait);
    }


}
