#include <Arduino.h>
#include <Servo.h>
#include <Tiny4kOLED.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN 125
#define SERVOMAX 625

typedef struct Aileron_s {
	Adafruit_PWMServoDriver* board;
	unsigned char angle;
	unsigned int delay;
	unsigned int then;
	int index;
} Aileron_t;

Aileron_t left_aileron;
Aileron_t right_aileron;
Aileron_t rudder;

unsigned long then1;

char selected = 1;
char d234 = 0x00;

typedef char dir_t;
#define LEFT_NONE -2
#define LEFT -1
#define NONE 0
#define RIGHT 1
#define RIGHT_NONE 2

unsigned char roll_speed = 10;
unsigned char roll_offset = 60;
dir_t roll_dir = LEFT;

Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);

unsigned char Bound(unsigned char a, unsigned char min, unsigned char max){
	if(a < min){
		return min;
	}

	if(a > max){
		return max;
	}

	return a;
}

char Dir(dir_t d){
	// Left is d < 0
	// Right is d > 0
	// None is d == 0
	if(d == 0){
		return NONE;
	}

	return (d < 0)?(LEFT):(RIGHT);
}

int angleToPulse(int ang) {
	// ang = Bound(ang, 0, 180);
	return map(ang, 0, 180, SERVOMIN, SERVOMAX);
}

void Aileron_Update_Delay(Aileron_t* air){
	unsigned int now = millis();
	if(now - air->then > air->delay){
		air->then = now;
		(*air->board).setPWM(air->index,0,angleToPulse(air->angle));
	}
}

void Aileron_Update(Aileron_t* air){
	(*air->board).setPWM(air->index,0,angleToPulse(air->angle));
}


void Aileron_Rotate(Aileron_t* air, unsigned char angle){
	air->angle = Bound(angle,0,180);
	Aileron_Update(air);
}

void Ailerons_Roll(Aileron_t* lair, Aileron_t* rair, dir_t direction){
	// Roll in the direction
	switch(Dir(direction)){
		case LEFT:
			roll_dir = LEFT;
		break;

		case RIGHT:
			roll_dir = RIGHT;
		break;

		case NONE:
			if(roll_dir == LEFT){
				roll_dir = LEFT_NONE;
			}

			if(roll_dir == RIGHT){
				roll_dir = RIGHT_NONE;
			}
		break;
	}

	switch(roll_dir){
		case LEFT:
			lair->angle = lair->angle + roll_speed;
			rair->angle = rair->angle + roll_speed;
			lair->angle = Bound(lair->angle, 90 - roll_offset, 90 + roll_offset);
			rair->angle = Bound(rair->angle, 90 - roll_offset, 90 + roll_offset);
		break;

		case RIGHT:
			lair->angle = lair->angle - roll_speed;
			rair->angle = rair->angle - roll_speed;
			lair->angle = Bound(lair->angle, 90 - roll_offset, 90 + roll_offset);
			rair->angle = Bound(rair->angle, 90 - roll_offset, 90 + roll_offset);
		break;

		case NONE:
			// lair->angle = 90;
			// rair->angle = 90;
		break;

		case LEFT_NONE:
			lair->angle = lair->angle - roll_speed;
			rair->angle = rair->angle - roll_speed;
			if(lair->angle <= 90){
				lair->angle = 90;
				rair->angle = 90;
				roll_dir = NONE;
			}
		break;

		case RIGHT_NONE:
			lair->angle = lair->angle + roll_speed;
			rair->angle = rair->angle + roll_speed;
			if(rair->angle <= 90){
				lair->angle = 90;
				rair->angle = 90;
				roll_dir = NONE;
			}
		break;
	}

	Aileron_Update(lair);
	Aileron_Update(rair);
}

void display_rotation(){
	oled.clear();
	oled.setCursor(0, 0);
	if(selected){
		oled.print("> AL: ");
	}else{
		oled.print("AL: ");
	}
	oled.print(left_aileron.angle);
	oled.setCursor(0, 1);
	if(selected){
		oled.print("AR: ");
	}else{
		oled.print("> AR: ");
	}
	oled.print(right_aileron.angle);
	oled.setCursor(0, 3);
	oled.print("RU: ");
	oled.print(rudder.angle);
	oled.switchFrame();
}

void setup(){
	Serial.begin(9600);
	left_aileron.delay = 0;
	right_aileron.delay = 0;
	left_aileron.angle = 90;
	right_aileron.angle = 90;
	left_aileron.index = 0;
	right_aileron.index = 1;
	left_aileron.board = &pca9685;
	right_aileron.board = &pca9685;

	pca9685.begin();
	pca9685.setPWMFreq(60);

	pinMode(2, INPUT);
	pinMode(3, INPUT);
	pinMode(4, INPUT);

	oled.begin();
	oled.setFont(FONT6X8);
	oled.clear();
	oled.on();
	oled.switchRenderFrame();
}

void loop(){
	unsigned long now = millis();
	char noMove = 1;

	if(digitalRead(4) == LOW){
		if(d234 & 0x01 > 0){
			selected = !selected;
			d234 = d234 & ~0x01;
		}
	}else{
		d234 = d234 | 0x01;
	}

	if(digitalRead(3) == LOW){
		if(d234 & 0x02 > 0){
			Ailerons_Roll(&left_aileron,&right_aileron,LEFT);
			noMove = 0;
			d234 = d234 & ~0x02;
		}
	}else{
		d234 = d234 | 0x02;
	}

	if(digitalRead(2) == LOW){
		if(d234 & 0x04 > 0){
			Ailerons_Roll(&left_aileron,&right_aileron,RIGHT);
			noMove = 0;
			d234 = d234 & ~0x04;
		}
	}else{
		d234 = d234 | 0x04;
	}


	if(noMove){
		Ailerons_Roll(&left_aileron,&right_aileron,NONE);
	}

	if(now - then1 >= 100){
		display_rotation();
	}
}

int main(){
	init();
	setup();

	for(;;){
		loop();
	}
}
