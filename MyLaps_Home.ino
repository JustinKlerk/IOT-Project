#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define ULTRASONIC A0 //Ultrasonic Pin 

// Indentify the LCD
LiquidCrystal_I2C lcd(0x27,16,2);


// Static variable users can change
const char* SSID_NETWORK = "IOT";
const char* PASSWORD = "";
const String URL = "http://iot-project-justin.glitch.me/laptime";

// Static variables
const int US_RANGE = 600;
const int MINIMAL_LAPTIME = 2000;

// Sensor variables
short USValue;

// Define the used variables for the timekeeping
long previousMillis = 0;
unsigned long startTime;
unsigned long elapsedTime;
unsigned long previous;
unsigned long allSeconds;
int runSeconds;
int runMinutes;
int runFractional;
int diffMillis;
String lapTime;
String diff;
boolean isFastestLap;
int lapCount = 1;
// Set the fastest lap at the really big value (converted 27 hours) so it will be overwritten in the first set lap
unsigned long fastestLapSeconds = 100000000;
// The character array for making the laptime string
char buf[21];


void setup() {
  Serial.begin(115200);

// Set the mode to Station mode and Start the WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NETWORK, PASSWORD);

// Turn on the LCD and the backlight
  lcd.init();
  lcd.backlight();

// Clear the LCD before the writing begins, this is done every time something needs to be written on it
  lcd.clear();
// While the WiFi is being setup write to the lcd that the device is connecting
  while ( WiFi.status() != WL_CONNECTED ) {
    lcd.setCursor(0,0);
    lcd.print("Connecting");
    Serial.print(".");
    delay(500);
  }
  lcd.clear();
  lcd.print("Connected!");
  Serial.print("You're connected to the network!");  

// Set the pinmode of the UltrasSonic (US) to input
  pinMode(ULTRASONIC, INPUT);
}

void loop() {
// Only when wifi is connected the code should run
  if (WiFi.status() == WL_CONNECTED) {

// Initiate the http client
    HTTPClient http;

// Read the value of the Ultrasonic Sensor
    USValue = analogRead(ULTRASONIC);   

// If the value comes above a certain value a car has passed and the laptime should be noted
    if (USValue > US_RANGE) {
      lcd.clear();
      //Serial.println(USValue);

      calculateLaptime();
     
      if (lapTime != "0"){       
        // Send the laptime data to the server
        http.begin(URL + "?lapCount=" + lapCount + "&laptime=" + lapTime + "&isFastest=" + isFastestLap);
        int httpCode = http.GET();
        // Console print the Status code, status 2**: oke. status 4**: Client error. status 5**: Server error. 
        Serial.println(httpCode);
        http.end();
        // Increase the lap counter with 1
        lapCount++;
        
        delay(1500);
      }    
    } 
      // Measure every millisecond, this way the laptimes are as accurate as possible
      delay(1);
  } else {
    Serial.println("Error, no connection!"); 
  }
}

// This funtion calculates the laptimes
void calculateLaptime(){
  isFastestLap = 0;
  // Set the start of the laptime at the current milliseconds that the program is running
  startTime = millis();

// Calculate the elapsed time from the previous laptime
  elapsedTime = (startTime - previous);
// Replace previous with the start time, this is the start of the next lap 
  previous = startTime;

// If the elapsedTime doesn't exceed the minimal laptime the laptime is not validated. this can't be a normal car.
// Something else must has passed the sensor, If this keeps repaiting check the sensor and restart the device.
  if (elapsedTime < MINIMAL_LAPTIME) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Invalid Laptime");
    lapTime = "0";
    delay(500);
    Serial.print("Invalid Laptime");
    
// if the elapsedTime is shorter than the best lap time, set the fastest lap.
  } else if (elapsedTime < fastestLapSeconds) {
    fastestLapSeconds = elapsedTime;
    lapTime = convertTime(elapsedTime);
    diff = "0";

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Fastest Lap!");
    lcd.setCursor(0,1);
    lcd.print("Lap: ");
    lcd.setCursor(5,1);
    lcd.print(lapTime);

    isFastestLap = 1;

//    Serial.print(lapCount);
//    Serial.print(" Fastest Lap: ");
//    Serial.println(lapTime);
    
// If none are true, Calculate the normal laptime and the difference to the fastest lap
  } else {
    lapTime = convertTime(elapsedTime);

    diffMillis = fastestLapSeconds - elapsedTime;
    diffMillis *= -1;
    diff = convertTime(diffMillis);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Dif:-");
    lcd.setCursor(5,0);
    lcd.print(diff);
    lcd.setCursor(0,1);
    lcd.print("Lap: ");
    lcd.setCursor(5,1);
    lcd.print(lapTime);

//    Serial.print(lapCount);
//    Serial.print(" Lap: ");
//    Serial.println(lapTime);
//    Serial.print("Dif:-");
//    Serial.println(diff);
  }
}

// This function converts the milliseconds to a propper time format (minutes:seconds.milliseconds) 
String convertTime(long temp) {
  allSeconds = (int)(temp / 1000);
  runMinutes = allSeconds / 60;
  runSeconds = allSeconds % 60;
  runFractional = (int)(temp % 1000L);

  sprintf(buf, "%d:%02d.%03d", runMinutes, runSeconds, runFractional);
  return String(buf);
}
