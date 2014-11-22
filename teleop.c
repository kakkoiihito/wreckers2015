#pragma config(Hubs,  S1, HTMotor,  HTMotor,  HTServo,  HTMotor)
#pragma config(Hubs,  S2, HTMotor,  none,     none,     none)
#pragma config(Sensor, S1,     ,               sensorI2CMuxController)
#pragma config(Sensor, S2,     ,               sensorI2CMuxController)
#pragma config(Sensor, S3,     gyro,              sensorAnalogInactive)
#pragma config(Motor,  mtr_S1_C1_1,     grabber,       tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S1_C1_2,     nothing,       tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S1_C2_1,     FR,            tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S1_C2_2,     BR,            tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S1_C4_1,     BL,            tmotorTetrix, openLoop, reversed)
#pragma config(Motor,  mtr_S1_C4_2,     FL,            tmotorTetrix, openLoop, reversed)
#pragma config(Motor,  mtr_S2_C1_1,     elevator,      tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S2_C1_2,     arm,           tmotorTetrix, openLoop)
#pragma config(Servo,  srvo_S1_C3_1,    lServo,               tServoStandard)
#pragma config(Servo,  srvo_S1_C3_2,    rServo,               tServoStandard)
#pragma config(Servo,  srvo_S1_C3_3,    sGyro,                tServoStandard)
#pragma config(Servo,  srvo_S1_C3_4,    servo4,               tServoStandard)
#pragma config(Servo,  srvo_S1_C3_5,    hatch,                tServoNone)
#pragma config(Servo,  srvo_S1_C3_6,    servo6,               tServoNone)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "JoystickDriver.c"

#include "drivers/hitechnic-gyro.h"

#include "ref/teleop_ref.h"

////////Constants
//Joystick threshold
const int THRESHOLD = 20;

////////Function Prototypes
void setMotor(mVals *m);
void setMotor(float fl, float fr, float bl, float br);

////////Tasks
task btnListener() {
	bool grabCylinderDown = false;
	bool pressed3 = false;

	while(true) {
		getJoystickSettings(joystick);

		//Elevator
		if (joy1Btn(7))
			motor[elevator] = -50;
		else if (joy1Btn(8))
			motor[elevator] = 50;
		else
			motor[elevator] = 0;

		//Elevator Arm
		if (joy1Btn(5))
			motor[arm] = -100;
		else if (joy1Btn(6))
			motor[arm] = 100;
		else
			motor[arm] = 0;

		//Left and right cylinder grabbers
		if (joy1Btn(3)) {
			if (!pressed3) {
				if (!grabCylinderDown) {
					grabCylinderDown = true;
					servo[rServo] = 110;
					servo[lServo] = 110;
				}
				else {
					grabCylinderDown = false;
					servo[rServo] = 0;
					servo[lServo] = 0;
				}
			}
			pressed3 = true;
		}
		else {
			pressed3 = false;
		}

		//Grabber intake/outake
		if (joy1Btn(2))
			motor[grabber] = 100;
		else if (joy1Btn(4))
			motor[grabber] = -100;
		else
			motor[grabber] = 0;


	}
}


task main()
{
	//Initialize components
	setMotor(0, 0, 0, 0);

	servo[rServo] = 0;
	servo[lServo] = 0;

	servo[sGyro] = 30;

	//Init Gyro
	wait1Msec(500);
	HTGYROstartCal(gyro);

	//Wait for Comp. to start
#ifdef wait_for_start
	waitForStart();
#endif

	StartTask(btnListener);

	//Main loop
	while(true) {
		getJoystickSettings(joystick);

		/////////////////////////////JOYSTICK MOVEMENT//////////////////////////

		//Vals on joy1_x/y1 will be translatational
		float tX = joystick.joy1_x1;
		float tY = joystick.joy1_y1;

		//Val on joy1_x2 will be for x rotation
		float rotX = joystick.joy1_x2;


		//Var to hold motor vals
		mVals *m;
		//clearDebugStream();

		//No unwanted movement
		m = (abs(tX) > THRESHOLD || abs(tY) > THRESHOLD || abs(rotX) > THRESHOLD) ? calcJoyMove(tX, tY, rotX): setMVals(0, 0, 0, 0);

		writeDebugStreamLine("%d", degreesToRadians(HTGYROreadRot(gyro)));

		////////////////////////D PAD MOVEMENT//////////////////////////////
		//D Pad will always override joystick
		//Check to see if top hat isnt centered (centered yields -1
		if (joystick.joy1_TopHat != -1) {

			switch (joystick.joy1_TopHat) {
			case 0: //Top
				m = forward();
				break;

			case 1: //Top Right
				m = diagFR();
				break;

			case 2: //Right
				m = strafeR();
				break;

			case 3: //Bottom Right
				m = diagBR();
				break;

			case 4: //Bottom
				m = backward();
				break;

			case 5: //Bottom Left
				m = diagBL();
				break;

			case 6: //Left
				m = strafeL();
				break;

			case 7: //Top Left
				m = diagFL();
				break;

			default:
				break;
			}
		}

		//Super Slowmo
		if (joy1Btn(5)) {
			m = setMVals(0.1*m->fl, 0.1*m->fr, 0.1*m->bl, 0.1*m->br);
		}

		setMotor(m);

	}

}

void setMotor(mVals *m) {
	//Pass along to func with the four motor vals
	setMotor(m->fl, m->fr, m->bl, m->br);
}

void setMotor(float fl, float fr, float bl, float br) {
	motor[FL] = fl;
	motor[FR] = fr;
	motor[BL] = bl;
	motor[BR] = br;
}
