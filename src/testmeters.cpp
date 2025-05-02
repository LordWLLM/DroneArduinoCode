	#include <Arduino.h>
	//#include <SingleWireSerial.h>
	//#include <SoftwareSerial.h>
	#include <Wire.h>


	//mpu6050
	const int MPU = 0x68;

	float GQT = 1;
	float GQX = 0;
	float GQY = 0;
	float GQZ = 0;

	float AccX, AccY, AccZ;

	float GyroX = 0;
	float GyroY = 0;
	float GyroZ = 0;

	float mpuX = 0;
	float mpuY = 0;
	float mpuZ = 0;
	float VelX = 0, VelY = 0, VelZ = 0; 

	float inAccX, inAccY, inAccZ; //inertialsystem

	float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
	float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
	float mpuDur, mpuCurrent, mpuPrevious;

	float GravX, GravY, GravZ;





	int c = 0;
	void mpuData();
	void gyroError();
	void gravCal();
	void updateGyro();
	void updateAcc();
	void gyroQuaternion();
	void inertialAcc();
	void updateVelPos();
	const int iters = 5000;

	void setup() { 
		Wire.begin(21, 22);
		Wire.setClock(100000);
		Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
		Wire.write(0x6B);                  // Talk to the register 6B
		Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
		Wire.endTransmission(true);        //end the transmission
		Serial.begin(115200);
		Wire.beginTransmission(MPU);
		Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
		Wire.write(0x00);                  //Set the register bits as 00010000 (+/- 8g full scale range)
		Wire.endTransmission(true);
		// Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
		Wire.beginTransmission(MPU);
		Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
		Wire.write(0x00);                   // Set the register bits as 00010000 (1000deg/s full scale)
		Wire.endTransmission(true);
		delay(20);
		//sound
		gyroError();
		gravCal();
		
		GQT = 1;
		GQX = 0;
		GQY = 0;
		GQZ = 0;
		
	 	mpuCurrent = micros();
		delay(10);
	}


	void loop() {  
		/*digitalWrite(trigPin, LOW);  
		delayMicroseconds(2);  
		digitalWrite(trigPin, HIGH);  
		delayMicroseconds(10);  
		digitalWrite(trigPin, LOW);  
		sdur = pulseIn(echoPin, HIGH); 
		sdist = sdur * 0.034 / 2;
		Serial.print("Distance: ");
		Serial.println(sdist);*/
		mpuData();
		//float a[] = {mpuX, mpuY, mpuZ,GQT, GQX, GQY, GQZ};
		float a[] = {inAccX, inAccY, inAccZ,GQT, GQX, GQY, GQZ};
		Serial.print("[");
		for(int i=0; i<6;i++){
			Serial.print(a[i]);
			Serial.print(",");
		}
		Serial.print(GQZ);
		Serial.println("]");
		/*
		Serial.print("tot:");
		Serial.print(sqrt(pow((AccX),2)+pow((AccY),2)+pow((AccZ), 2)));
		Serial.print("  ");
		Serial.print(sqrt(pow((GyroX),2)+pow((GyroY),2)+pow((GyroZ), 2)));
		Serial.println("");*/
		delay(10);
		
	}

	void mpuData()
	{
		mpuPrevious = mpuCurrent;
		mpuCurrent = micros();   
		mpuDur = (mpuCurrent - mpuPrevious) / 1000000;
		updateAcc();
		updateGyro();
		gyroQuaternion();
		inertialAcc();
		updateVelPos();

		/*
		Serial.print(GQT);
		Serial.print("   ");
		Serial.print(GQX);
		Serial.print("   ");
		Serial.print(GQY);
		Serial.print("   ");
		Serial.println(GQZ);*/
		

		/*
		Serial.print(sqrt(pow((AccX),2)+pow((AccY),2)+pow((AccZ), 2)));
		Serial.print("   ");
		Serial.print(sqrt(pow((inAccX),2)+pow((inAccY),2)+pow((inAccZ), 2)));
		Serial.print("   ");
		Serial.print(sqrt(pow((inAccX-GravX),2)+pow((inAccY-GravY),2)+pow((inAccZ-GravZ), 2)));
		Serial.println("   ");*/
		
	/*
		Serial.print(inAccX);
		Serial.print("   ");
		Serial.print(inAccY);
		Serial.print("   ");
		Serial.print(inAccZ);
		Serial.print("   ");
		Serial.print(GravX- AccX);
		Serial.print("   ");
		Serial.print(GravY-AccY);
		Serial.print("   ");
		Serial.print(GravZ-AccZ);
		Serial.print("   ");
		*//*
		Serial.print(GravX- inAccX);
		Serial.print("   ");
		Serial.print(GravY-inAccY);
		Serial.print("   ");
		Serial.println(GravZ-inAccZ);*/
		/*
		Serial.print(VelX);
		Serial.print("   ");
		Serial.print(VelY);
		Serial.print("   ");
		Serial.println(VelZ);*/
	}



	void gyroError() {
		// We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
		// Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
		// Read accelerometer values 200 times
		c = 0;
		// Read gyro values 200 times
		float totX = 0;
		float totY = 0;
		float totZ = 0;
		while (c < iters) {
			updateGyro();
			totX = totX+GyroX;
			totY = totY+GyroY;
			totZ = totZ+GyroZ;
			c++;
			delay(1);
		}
		//Divide the sum by 200 to get the error value
		GyroErrorX = totX / iters;
		GyroErrorY = totY / iters;
		GyroErrorZ = totZ / iters;
		// Print the error values on the Serial Monitor
		Serial.println(GyroErrorX);
		Serial.println(GyroErrorY);
		Serial.println(GyroErrorZ);
	}

	void gravCal(){
		c=0;
		while (c < iters) {
			updateAcc();
			GravX=GravX+AccX;
			GravY=GravY+AccY;
			GravZ=GravZ+AccZ;
			c++;
			/*
			Serial.print(AccX);
			Serial.print("   ");
			Serial.print(AccY);
			Serial.print("   ");
			Serial.print(AccZ);*/
			delay(1);
		}
		GravX=GravX/iters;
		GravY=GravY/iters;
		GravZ=GravZ/iters;
		Serial.println( GravX);
		Serial.println( GravY);
		Serial.println( GravZ);

	}

	void updateAcc(){
		Wire.beginTransmission(MPU);
		Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
		Wire.endTransmission(false);
		Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
		//For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
		//AccX = ((Wire.read() << 8 | Wire.read()) / 16384.0 * 9.81)*1.00051291-0.1661476; // X-axis value
		//AccY = ((Wire.read() << 8 | Wire.read()) / 16384.0 * 9.81)*1.25819918 -0.82226703; // Y-axis value
		//AccZ = ((Wire.read() << 8 | Wire.read()) / 16384.0 * 9.81)*0.96164858+0.18752147; // Z-axis value  
		AccX = ((Wire.read() << 8 | Wire.read()) / 16384.0 * 9.81); // X-axis value
		AccY = ((Wire.read() << 8 | Wire.read()) / 16384.0 * 9.81); // Y-axis value
		AccZ = ((Wire.read() << 8 | Wire.read()) / 16384.0 * 9.81); // Z-axis value  
		
	}

	void updateGyro(){
		Wire.beginTransmission(MPU);
		Wire.write(0x43); // Gyro data first register address 0x43
		Wire.endTransmission(false);
		Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
		GyroX = (Wire.read() << 8 | Wire.read()) / 131.0 * PI/180; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
		GyroY = (Wire.read() << 8 | Wire.read()) / 131.0 * PI/180;
		GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0 * PI/180;

		GyroX = GyroX - GyroErrorX;
		GyroY = GyroY - GyroErrorY;
		GyroZ = GyroZ - GyroErrorZ;
	}

	void gyroQuaternion(){
		float GyroLength = sqrt(pow((GyroX),2)+pow((GyroY),2)+pow((GyroZ),2));
		if (GyroLength > 0.0001f) {
			/*Serial.print(GyroX);
			Serial.print("  ");
			Serial.print(GyroY);
			Serial.print("  ");
			Serial.println(GyroZ);*/
			float halfTheta = GyroLength*mpuDur/2.0f;
			float sinHalf = sin(halfTheta);
			
			float dGQX = GyroX/GyroLength * sinHalf;
			float dGQY = GyroY/GyroLength * sinHalf;
			float dGQZ = GyroZ/GyroLength * sinHalf;
			float dGQT = cos(halfTheta);

			//apply quaternion to old 
			float newGQT = GQT*dGQT - GQX*dGQX - GQY*dGQY - GQZ*dGQZ;
			float newGQX = GQT*dGQX + GQX*dGQT + GQY*dGQZ - GQZ*dGQY;
			float newGQY = GQT*dGQY - GQX*dGQZ + GQY*dGQT + GQZ*dGQX;
			float newGQZ = GQT*dGQZ + GQX*dGQY - GQY*dGQX + GQZ*dGQT;

			float newLength = sqrt(pow((newGQT),2)+pow((newGQX),2)+pow((newGQY),2)+pow((newGQZ),2));
			GQT = newGQT/newLength;
			GQX = newGQX/newLength;
			GQY = newGQY/newLength;
			GQZ = newGQZ/newLength;
		}
	}

	void inertialAcc(){
		float tx = 2.0f * ( GQY *  AccZ -  GQZ *  AccY);
		float ty = 2.0f * ( GQZ *  AccX -  GQX *  AccZ);
		float tz = 2.0f * ( GQX *  AccY -  GQY *  AccX);

		inAccX =  AccX +  GQT * tx + ( GQY * tz -  GQZ * ty);
		inAccY =  AccY +  GQT * ty + ( GQZ * tx -  GQX * tz);
		inAccZ =  AccZ + GQT * tz + ( GQX * ty -  GQY * tx);
	}

	void updateVelPos(){
		if(abs(inAccX - GravX)>0.01f){
		VelX = VelX + mpuDur*(inAccX - GravX);
		}
		if(abs(inAccY - GravY)>0.01f){
		VelY = VelY + mpuDur*(inAccY - GravY);
		}
		if(abs(inAccZ - GravZ)>0.01f){
		VelZ = VelZ + mpuDur*(inAccZ - GravZ);
		}

		mpuX = mpuX+VelX*mpuDur;
		mpuY = mpuY+VelY*mpuDur;
		mpuZ = mpuZ+VelZ*mpuDur;
	}