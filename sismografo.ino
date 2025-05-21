/*
  This Arduino code is designed for interfacing with the GY-87 IMU module, focusing
  specifically on the MPU6050 sensor, which includes an accelerometer and a gyroscope.
  The code initializes the MPU6050 sensor and prints its accelerometer, gyroscope, and
  temperature readings to the Serial Monitor at regular intervals.
 
  Board: Arduino Uno R4
  Component: GY-87 IMU Module
  Library: https://github.com/adafruit/Adafruit_MPU6050  (Adafruit MPU6050 by Adafruit)
*/

// Include required libraries
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Initialize sensor objects
Adafruit_MPU6050 mpu;
#define x_superior 1.5
#define x_inferior -0.3
#define y_superior 1
#define y_inferior -1.5
#define z_superior 9
#define z_inferior 8.5

float eje_x;       // medición de aceleración eje x
float eje_y;       // medición de aceleración eje y
float eje_z;       // medición de aceleración eje z

float max_x = 0;      
float max_y = 0;      
float max_z = 0;

bool modo_terremoto = 0; // 0 = normal, 1 = terremoto

float eje_xf = 0;       // medición del sensor [0,1023]
float eje_yf = 0;       // medición del sensor [0,1023]
float eje_zf = 0;       // medición del sensor [0,1023]

float alpha = 0.5;

const int buzzer = 13;
const int led1 = 10;
const int led2 = 11;
const int led3 = 12;


void setup() {

  lcd.begin(16, 2);         // Inicializar el LCD
  
  lcd.setCursor(0,0);
  lcd.print("Estado");     // print a simple message

  lcd.setCursor(0,1);
  lcd.print("Magnitud");     // print a simple message

  // Initialize the serial communication with a baud rate of 115200
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  // Initialize the MPU6050 sensor (accelerometer and gyroscope)
  initializeMPU6050();
}

void loop() {

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Estado");     // print a simple message
  lcd.setCursor(0,1);
  lcd.print("Magnitud");     // print a simple message
  lcd.setCursor(11,1);              // Cursor a linea 2, posicion 8
  lcd.print(0);        // Imprime segundos
  lcd.setCursor(9, 0);            // Cursor a linea 1, posicion 13
  lcd.print("NORMAL");              // Imprime el valor leido en la puerta A0
  lcd.setCursor(11,1);              // Cursor a linea 2, posicion 11

  // Print MPU6050 data
  printMPU6050();
  
  

  if (eje_x != 0){
    //digitalWrite(led1, HIGH);
    modo_terremoto = 1;
  } else {
    //digitalWrite(led1, LOW);
  }

    if (eje_y != 0){
    //digitalWrite(led2, HIGH);
    modo_terremoto = 1;
  }  else {
    //digitalWrite(led2, LOW);
  }

    //if (eje_z != 0){
    //digitalWrite(led3, HIGH);
    //modo_terremoto = 1;
  //}  else {
    //digitalWrite(led3, LOW);
  //}

  if (modo_terremoto){
    
    calcular_ritcher();

  }

  delay(1500);
}

void calcular_ritcher(){
  digitalWrite(buzzer, HIGH);

  if (abs(eje_x) >= max_x){
    max_x = abs(eje_x);
  }
  if (abs(eje_y) >= max_y){
    max_y = abs(eje_y);
  }

  //delay(100);

  if (eje_x == 0 && eje_y == 0 && eje_z == 0){
    float vector = sqrt(pow(max_x, 2) + pow(max_y, 2));
    //float vector = (max_x + max_y + max_z)/3;
    float richter = log10(vector*2000)+2;

  

    if (richter < 5.5){
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    }
    if (5.5 < richter <= 6){
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);

    } 
    if (richter > 6){
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    }

    lcd.setCursor(11,1);             // Cursor a linea 2, posicion 8
    lcd.print(richter);              // Imprime ESCALA
    lcd.setCursor(7, 0);             // Cursor a linea 1, posicion 13
    lcd.print("TERREMOTO");          // Imprime el valor leido en la puerta A0
    lcd.setCursor(11,1);             // Cursor a linea 2, posicion 11

    //info_sismo(richter);

    modo_terremoto = 0;
    digitalWrite(buzzer, LOW);
    max_x = 0;
    max_y = 0;
    max_z = 0;

  }

}

void info_sismo(float x){
  Serial.println("__________________________________________________");
  Serial.println("¡HUBO UN SISMO!");
  Serial.print("Magnitud en la escala Richter: ");
  Serial.println(x);

  Serial.print("Aceleración eje x: ");
  Serial.println(max_x);

  Serial.print("Aceleración eje y: ");
  Serial.println(max_y);

  Serial.print("Aceleración eje z: ");
  Serial.println(max_z);

}

void initializeMPU6050() {
  // Check if the MPU6050 sensor is detected
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
      ;  // Halt if sensor not found
  }
  Serial.println("MPU6050 Found!");

  // set accelerometer range to +-8G
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // set gyro range to +- 500 deg/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // set filter bandwidth to 21 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("");
  delay(100);
}

void printMPU6050() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  eje_x = a.acceleration.x;
  eje_y = a.acceleration.y;
  eje_z = a.acceleration.z;

  Serial.print(eje_x); Serial.print("\t");
  Serial.print(eje_y);  Serial.print("\t");
  Serial.println(eje_z);

  if ( eje_x < x_superior && eje_x > x_inferior){
    eje_x = 0;
  }
    if ( eje_y < y_superior && eje_y > y_inferior){
    eje_y = 0;
  }    
    if (eje_z < z_superior && eje_z > z_inferior){
    eje_z = 0;
  }

  

}
