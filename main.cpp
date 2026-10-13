#include <Arduino.h>
#include <Servo.h>
#include <Tiny4kOLED.h>

typedef struct Aileron_s {
	Servo servo;
	unsigned char angle;
	unsigned int delay;
	unsigned int then;
	int pin;
} Aileron_t;

Aileron_t left_aileron;
Aileron_t right_aileron;
Aileron_t rudder;

unsigned char i = 90;
unsigned char i2 = 90;
unsigned long then1;

char selected = 0;
char d234 = 0x00;

unsigned char Bound(unsigned char a, unsigned char min, unsigned char max){
	if(a < min){
		return min;
	}

	if(a > max){
		return max;
	}

	return a;
}

void Aileron_Attach(Aileron_t* air, int pin){
	air->pin = pin;
	air->servo.attach(air->pin);
}

void Aileron_Rotate(Aileron_t* air, unsigned char angle){
	unsigned int now = millis();
	if(now - air->then > air->delay){
		air->then = now;
		air->angle = Bound(angle,0,180);
		air->servo.write(air->angle);
	}
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
	Aileron_Attach(&left_aileron,A0);
	Aileron_Attach(&right_aileron,A1);

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
			if(selected){
				i = Bound(i + 10,0,180);
			}else{
				i2 = Bound(i2 + 10,0,180);
			}
			d234 = d234 & ~0x02;
		}
	}else{
		d234 = d234 | 0x02;
	}

	if(digitalRead(2) == LOW){
		if(d234 & 0x04 > 0){
			if(selected){
				i = i - 10;
				if(i > 180){
					i = 0;
				}
			}else{
				i2 = i2 - 10;
				if(i2 > 180){
					i2 = 0;
				}
			}
			d234 = d234 & ~0x04;
		}
	}else{
		d234 = d234 | 0x04;
	}


	Aileron_Rotate(&left_aileron,i);
	Aileron_Rotate(&right_aileron,i2);

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
