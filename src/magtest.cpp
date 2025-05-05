#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_MPU6050.h>
#include <TinyGPSPlus.h>
#include <BluetoothSerial.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_MPU6050 mpu;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

TinyGPSPlus gps;
BluetoothSerial SerialBT;

void updateSensors();
void displaySensorDetails();
void gyroQuaternion();
void intertialAcc();
void updateVelPos();
void sendInfo();
void gyroError();
float MagX, MagY, MagZ;
float mpuDur, mpuCurrent, mpuPrevious;
float GQT = 1;
float GQX, GQY, GQZ;
float GyroX, GyroY, GyroZ;
float GyroErrorX, GyroErrorY, GyroErrorZ;
float AccX, AccY, AccZ;
float GravX, GravY, GravZ;
float PosX, PosY, PosZ, VelX, VelY, VelZ, inAccX, inAccY, inAccZ;

void displaySensorDetails(void)
{
    sensor_t sensor;
    mag.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println(" uT");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println(" uT");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println(" uT");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);

    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    mpu.setMotionDetectionThreshold(1);
    mpu.setMotionDetectionDuration(20);
    mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
    mpu.setInterruptPinPolarity(true);
    mpu.setMotionInterrupt(true);
}

void setup(void)
{

    Serial.begin(115200);
    SerialBT.begin("ESP32_Test");
    Serial.println("Bluetooth Started");
    Serial.println("HMC5883 Magnetometer Test");
    Serial.println("");

    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
        {
            delay(10);
        }
    }

    /* Initialise the sensor */
    if (!mag.begin())
    {
        /* There was a problem detecting the HMC5883 ... check your connections */
        Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
        while (1)
            ;
    }

    /* Display some basic information on this sensor */
    displaySensorDetails();
    gyroError();
    mpuPrevious = micros();
}

void loop(void)
{
    updateSensors();
    gyroQuaternion();
    sendInfo();
    /* Display the results (magnetic vector values are in micro-Tesla (uT))
    Serial.print("X: ");
    Serial.print(event.magnetic.x);
    Serial.print("  ");
    Serial.print("Y: ");
    Serial.print(event.magnetic.y);
    Serial.print("  ");
    Serial.print("Z: ");
    Serial.print(event.magnetic.z);
    Serial.print("  ");
    Serial.println("uT");

    Serial.print("AccelX:");
    Serial.print(a.acceleration.x);
    Serial.print(",");
    Serial.print("AccelY:");
    Serial.print(a.acceleration.y);
    Serial.print(",");
    Serial.print("AccelZ:");
    Serial.print(a.acceleration.z);
    Serial.print(", ");
    Serial.print("GyroX:");
    Serial.print(g.gyro.x);
    Serial.print(",");
    Serial.print("GyroY:");
    Serial.print(g.gyro.y);
    Serial.print(",");
    Serial.print("GyroZ:");
    Serial.print(g.gyro.z);
    Serial.println("");*/
    float a[] = {GQT, GQX, GQY, GQZ, GyroX, GyroY, GyroZ};
    int data_length = sizeof(a) / sizeof(a[0]);
    for (int i = 0; i < data_length - 1; i++)
    {
        Serial.print(a[i]);
        Serial.print(",");
    }
    Serial.println(a[data_length - 1]);

    delay(1);
}

void gyroQuaternion()
{
    float GyroLength = sqrt(pow((GyroX), 2) + pow((GyroY), 2) + pow((GyroZ), 2));
    if (GyroLength > 0.0001f)
    {
        /*Serial.print(GyroX);
        Serial.print("  ");
        Serial.print(GyroY);
        Serial.print("  ");
        Serial.println(GyroZ);*/
        float halfTheta = GyroLength * mpuDur / 2.0f;
        float sinHalf = sin(halfTheta);

        float dGQX = GyroX / GyroLength * sinHalf;
        float dGQY = GyroY / GyroLength * sinHalf;
        float dGQZ = GyroZ / GyroLength * sinHalf;
        float dGQT = cos(halfTheta);

        // apply quaternion to old
        float newGQT = GQT * dGQT - GQX * dGQX - GQY * dGQY - GQZ * dGQZ;
        float newGQX = GQT * dGQX + GQX * dGQT + GQY * dGQZ - GQZ * dGQY;
        float newGQY = GQT * dGQY - GQX * dGQZ + GQY * dGQT + GQZ * dGQX;
        float newGQZ = GQT * dGQZ + GQX * dGQY - GQY * dGQX + GQZ * dGQT;

        float newLength = sqrt(pow((newGQT), 2) + pow((newGQX), 2) + pow((newGQY), 2) + pow((newGQZ), 2));
        GQT = newGQT / newLength;
        GQX = newGQX / newLength;
        GQY = newGQY / newLength;
        GQZ = newGQZ / newLength;
    }
}

void inertialAcc()
{
    float tx = 2.0f * (GQY * AccZ - GQZ * AccY);
    float ty = 2.0f * (GQZ * AccX - GQX * AccZ);
    float tz = 2.0f * (GQX * AccY - GQY * AccX);

    inAccX = AccX + GQT * tx + (GQY * tz - GQZ * ty);
    inAccY = AccY + GQT * ty + (GQZ * tx - GQX * tz);
    inAccZ = AccZ + GQT * tz + (GQX * ty - GQY * tx);
}

void updateVelPos()
{
    if (abs(inAccX - GravX) > 0.01f)
    {
        VelX = VelX + mpuDur * (inAccX - GravX);
    }
    if (abs(inAccY - GravY) > 0.01f)
    {
        VelY = VelY + mpuDur * (inAccY - GravY);
    }
    if (abs(inAccZ - GravZ) > 0.01f)
    {
        VelZ = VelZ + mpuDur * (inAccZ - GravZ);
    }

    PosX = PosX + VelX * mpuDur;
    PosY = PosY + VelY * mpuDur;
    PosZ = PosZ + VelZ * mpuDur;
}

void sendInfo()
{
    float a[] = {PosX, PosY, PosZ, VelX, VelY, VelZ, inAccX, inAccY, inAccZ, MagX, MagY, MagZ, GQT, GQX, GQY, GQZ, GyroX, GyroY, GyroZ, 0, 0, 0, 0};
    int data_length = sizeof(a) / sizeof(a[0]);
    for (int i = 0; i < data_length - 1; i++)
    {
        SerialBT.print(a[i]);
        SerialBT.print(",");
    }
    SerialBT.println(a[data_length - 1]);
}

void updateSensors()
{
    /* Get a new sensor event */
    sensors_event_t event, a, g, temp;
    mag.getEvent(&event);
    mpu.getEvent(&a, &g, &temp);
    MagX = event.magnetic.x;
    MagY = event.magnetic.y;
    MagZ = event.magnetic.z;

    AccX = a.acceleration.x;
    AccY = a.acceleration.y;
    AccZ = a.acceleration.z;

    GyroX = g.gyro.x - GyroErrorX;
    GyroY = g.gyro.y - GyroErrorY;
    GyroZ = g.gyro.z - GyroErrorZ;
    mpuCurrent = mpuPrevious;
    mpuCurrent = micros();
    mpuDur = (mpuCurrent - mpuPrevious) / 1000000.0f;
}

void gyroError()
{
    int i = 0;
    int iters = 1000;
    float totX = 0;
    float totY = 0;
    float totZ = 0;
    while (i < iters)
    {
        updateSensors();
        totX = totX + GyroX;
        totY = totY + GyroY;
        totZ = totZ + GyroZ;
        i++;
        delay(1);
    }
    GyroErrorX = totX / iters;
    GyroErrorY = totY / iters;
    GyroErrorZ = totZ / iters;
    Serial.print(GyroErrorX);
    Serial.print("  ");
    Serial.print(GyroErrorY);
    Serial.print("  ");
    Serial.print(GyroErrorZ);
    Serial.print("  ");
    Serial.print(GyroX);
    Serial.print("  ");
    Serial.print(GyroY);
    Serial.print("  ");
    Serial.print(GyroZ);
    Serial.print("  ");
}
