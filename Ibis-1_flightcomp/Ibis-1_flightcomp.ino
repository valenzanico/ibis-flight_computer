#include <Wire.h>
#include <SD.h>

const int MPU_adress = 0x68;  // I2C address of the MPU-6050
bool MPU6050_status = false;

bool SDCARD_status = false;
#define SD_CS_PIN 4
File dataFile;

#define WARN_LED LED_BUILTIN

struct gyroscope_data {
  int x;
  int y;
  int z;
};

struct accelerometer_data {
  int x;
  int y;
  int z;
};

struct flight_data { //structure for move measured datas
  int temperature;
  gyroscope_data gyroscope;
  accelerometer_data accelerometer;
};

void writeSDCARD(flight_data data) {
  dataFile.print(
    String(data.accelerometer.x) + "," +
    String(data.accelerometer.y) + "," +
    String(data.accelerometer.z) + "," +
    String(data.gyroscope.x) + "," +
    String(data.gyroscope.y) + "," +
    String(data.gyroscope.z) + "," +
    String(data.temperature) + "\n"
  )

}



flight_data readMPU6050() {  //this function read data from MPU6050 sensor 

  Wire.beginTransmission(MPU_adress);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_adress, 14, true);  // request a total of 14 registers
  

  flight_data data;

  data.accelerometer.x = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  data.accelerometer.y = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  data.accelerometer.z = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  data.temperature = Wire.read() << 8 | Wire.read();      // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  data.gyroscope.x = Wire.read() << 8 | Wire.read();      // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  data.gyroscope.y = Wire.read() << 8 | Wire.read();      // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  data.gyroscope.z = Wire.read() << 8 | Wire.read();      // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  return data;
}

void setup() {
  //setup MPU-6050
  Wire.begin();
  Wire.beginTransmission(MPU_adress);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  MPU6050_status = true;
  Serial.println("MPU6050 works");

  if (SD.begin(SD_CS_PIN)) {//setup sdcard
    int count_file = 0;
    File root = SD.open("/");
    File list_file = root.openNextFile();
    //count already existent file to give name to the new file
    while (list_file)
    {
        if (!list_file.isDirectory())
        {
            count_file++;
        }
        list_file = root.openNextFile();
    }
    String file_name = "DATA" + String(count_file + 1) + ".csv";
    dataFile = SD.open(file_name, FILE_WRITE);//creating and opening file
    dataFile.print("ACX,ACY,ACZ,GYX,GYY,GYZ,TEMP\n");
    SDCARD_status = true;
    Serial.println("SD card works");
  } 
  else {
    SDCARD_status = false;
  }

  Serial.begin(9600);

  pinMode(WARN_LED, OUTPUT);
}

void loop() { 

  if (!MPU6050_status) { //if MPU6050 doesn't setup blink very fast
    while (true) {
      Serial.println("problem with MPU6050");
      digitalWrite(WARN_LED, HIGH);
      delay(100);
      digitalWrite(WARN_LED, LOW);
      delay(100);
    }
  }
  if (!SDCARD_status) //if sd card doesn't setup blink at normal speed
  {
     while (true) {
      Serial.println("problem with SD card");
      digitalWrite(WARN_LED, HIGH);
      delay(500);
      digitalWrite(WARN_LED, LOW);
      delay(500);
    }
  }

   flight_data current_data;
   current_data = readMPU6050();//reading datas from sensor

   Serial.print("Acceleration X: ");
   Serial.println(current_data.accelerometer.x);
   Serial.print("Acceleration Y: ");
   Serial.println(current_data.accelerometer.y);
   Serial.print("Acceleration Z: ");
   Serial.println(current_data.accelerometer.z);
   Serial.print("Gyroscope X: ");
   Serial.println(current_data.gyroscope.x);
   Serial.print("Gyroscope Y: ");
   Serial.println(current_data.gyroscope.y);
   Serial.print("Gyroscope Z: ");
   Serial.println(current_data.gyroscope.z);
   Serial.print("Temperature: ");
   Serial.println(current_data.temperature);

   writeSDCARD(current_data);//save datas in sd card
   delay(1);
}


