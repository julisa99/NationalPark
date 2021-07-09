#include <DHT.h>  //Sensor library
#include <TFT.h>  //LCD library
#include <SPI.h>

//Constants of ultrasonic sensor
#define TRIG_PIN 2
#define ECHO_PIN 3

//Constants of weather sensor
#define DHTPIN 50  // DATA Pin
#define DHTTYPE DHT22 // DHT 22

//Constants for displays
#define CS_WEATHER   30
#define DC_WEATHER   32
#define RST_WEATHER  28

#define CS_COUNTER     24
#define DC_COUNTER     26
#define RESET_COUNTER  22

//Pressure sensor
#define FSR_PIN A0

//Serial for bluetooth module
#define HC05 Serial3

#define HEIGHT_CHILD_THRESHOLD 100

#define RESET_PIN 4

//Instances
DHT dht(DHTPIN, DHTTYPE);
TFT WeatherScreen = TFT(CS_WEATHER, DC_WEATHER, RST_WEATHER);
TFT CounterScreen = TFT(CS_COUNTER, DC_COUNTER, RESET_COUNTER);

//Global variables
int heightThreshold = 0; // distance between sensor and bottom in cm
int counterChild = 0;
int counterAdult = 0;
char printoutChild[5];
char printoutAdult[5];
String counterChildString;
String counterAdultString;
String hum;  //Stores humidity value
String temp; //Stores temperature value

char humArray[6]; // humidity char array to print to the screen
char tempArray[5]; // temperature char array to print to the screen

// BT module variables
char BT_receive;
String BT_state;

// measurement of visitors with ultrasonic module
int calculateDistance() {
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW); 
  
  return pulseIn(ECHO_PIN, HIGH)/58;
}

// visitors counter
void counter() {
  int distance = calculateDistance();
  // write distance output
  if (distance < 5 || distance > heightThreshold){
    //Serial.println("No distance measured");
  } 
  else {
    //Serial.print("Distance: ");
    //Serial.print(distance);
    //Serial.println("cm");
    
    // object detected 
    if(distance > HEIGHT_CHILD_THRESHOLD){
     counterChild++;
    } else {
     counterAdult++;
    }  
  }
}

void setup() {
  Serial.begin(9600);
  HC05.begin(9600);
  
  pinMode(FSR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RESET_PIN, OUTPUT);  
  digitalWrite(RESET_PIN, HIGH);
  
  dht.begin();
  WeatherScreen.begin();
  CounterScreen.begin();

  WeatherScreen.background(0, 0, 0); // clear the screen with a black background
  CounterScreen.background(0,0,0);

  WeatherScreen.stroke(255, 255, 255); // set the font color to white
  CounterScreen.stroke(255, 255, 255);
  
  WeatherScreen.setTextSize(2); // set the font size
  CounterScreen.setTextSize(2);

  //setting permanent text on the screen
  WeatherScreen.text("Temperature:\n ", 0, 0);
  WeatherScreen.text("Humidity:\n ", 0, 64);

  CounterScreen.text("CHILD", 0, 0);
  CounterScreen.text("ADULT", 0, 64);

  WeatherScreen.setTextSize(4); //set the font size very large for the loop
  CounterScreen.setTextSize(4); //set the font size very large for the loop

  //setting permanent text on the screen
  WeatherScreen.text("C\n", 130, 25);
  WeatherScreen.text("%\n", 130, 85);

  CounterScreen.text("0", 0, 25);
  CounterScreen.text("0", 0, 85);

  heightThreshold = calculateDistance();
  Serial.println(heightThreshold);
}

void loop() {
  HC05.flush();
  if(HC05.available()){    // check if BT is available and send command
    String serialOutput = HC05.readString();
    if(serialOutput == "RESET"){
      counterChild = 0;
      counterAdult = 0;
      HC05.println("");
      HC05.println("Counters were cleaned. Current values... A: " + String(counterAdult) + "  CH: " + String(counterChild));
    }
    if(serialOutput == "STATUS"){
      HC05.println("");
      HC05.println("Visitors status:");
      HC05.println("ADULTS: " + String(counterAdult));
      HC05.println("CHILDS: " + String(counterChild));
      HC05.println("");
      HC05.println("SUMMARY: " + String(counterAdult + counterChild));
    }
    if(serialOutput == "RESTART"){
      HC05.println("");
      HC05.println("Restarting Arduino");
      digitalWrite(RESET_PIN, LOW);
    }
  }
  
  if(analogRead(FSR_PIN) > 100)
  {
    Serial.println("Person detected.");
    counter();
  }
  else {
     Serial.println("No person detected.");
  }
  
  WeatherScreen.fillRect(0,25,120,30,0x0000); // clear temperature value box
  WeatherScreen.fillRect(0,85,120,30,0x0000); // clear humidity value box

  CounterScreen.fillRect(0,25,120,30,0x0000); // clear children value box
  CounterScreen.fillRect(0,85,120,30,0x0000); // clear adults value box

  temp = String(dht.readTemperature()); //read temperature
  hum = String(dht.readHumidity()); //read humidity

  counterChildString = static_cast<String>(counterChild); //count childs
  counterAdultString = static_cast<String>(counterAdult); //count adults
  
  counterChildString.toCharArray(printoutChild,5); // pretype varialbe
  counterAdultString.toCharArray(printoutAdult,5); //pretype variable
  
  temp.toCharArray(tempArray, 5); // pretype variable
  hum.toCharArray(humArray, 6); //pretype variable

  WeatherScreen.stroke(200, 200, 1); // set the font color to yellow
  CounterScreen.stroke(200, 200, 1);

  WeatherScreen.text(tempArray, 0, 25); 
  WeatherScreen.text(humArray, 0, 85);
  
  CounterScreen.text(printoutChild, 0, 25);
  CounterScreen.text(printoutAdult, 0, 85);

  delay(800);
}
