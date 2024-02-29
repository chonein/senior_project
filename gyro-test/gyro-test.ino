// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // // Try to initialize!
  // if (!mpu.begin()) {
  //   Serial.println("Failed to find MPU6050 chip");
  //   while (1) {
  //     delay(10);
  //   }
  // }
  // Serial.println("MPU6050 Found!");

  // mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  // mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  init_accel_gyro();
  delay(100);
}

void loop() {
  // sensors_event_t a, g, temp;
  // mpu.getEvent(&a, &g, &temp);

  // /* Print out the values */
  // Serial.print("Acceleration X: ");
  // Serial.print(a.acceleration.x);
  // Serial.print(", Y: ");
  // Serial.print(a.acceleration.y);
  // Serial.print(", Z: ");
  // Serial.print(a.acceleration.z);
  // Serial.println(" m/s^2");

  // Serial.print("Acceleration X %: ");
  // Serial.print(convert_accel_range(a.acceleration.x));
  // Serial.print(", Y %: ");
  // Serial.print(convert_accel_range(a.acceleration.y));
  // Serial.print(", Z %: ");
  // Serial.print(convert_accel_range(a.acceleration.z));
  // Serial.println();

  // Serial.print("Rotation X: ");
  // Serial.print(g.gyro.x);
  // Serial.print(", Y: ");
  // Serial.print(g.gyro.y);
  // Serial.print(", Z: ");
  // Serial.print(g.gyro.z);
  // Serial.println(" rad/s");

  // Serial.print("Temperature: ");
  // Serial.print(temp.temperature);
  // Serial.println(" degC");

  // Serial.println("");
  // delay(500);

  send_accel_gyro_periodically();
  // delay(500);
}

void init_accel_gyro() {
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

int8_t convert_accel_range(float accel) {
  int val = accel * 127.0 / (4.0 * SENSORS_GRAVITY_STANDARD);
  if (val > 127) {
    val = 127;
  } else if (val < -128) {
    val = -128;
  }
  return val;
}

void send_accel_gyro() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  int8_t conv_x = convert_accel_range(a.acceleration.x);
  int8_t conv_y = convert_accel_range(a.acceleration.y);
  int8_t conv_z = convert_accel_range(a.acceleration.z);

  Serial.print("Acceleration X: ");
  Serial.print(conv_x);
  Serial.print(", Y: ");
  Serial.print(conv_y);
  Serial.print(", Z: ");
  Serial.print(conv_z);
  Serial.println();
}

void send_accel_gyro_periodically() {
  const uint32_t send_period_ms = 200; // 10Hz => 100ms
  static unsigned long last_sent_time = 0;

  if (millis() - last_sent_time > send_period_ms) {
    send_accel_gyro();
    last_sent_time = millis();
  }
}