#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <softPwm.h>
#include <termios.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define IMG_Width 640
#define IMG_Height 480

#define GPIO0 0  //Physical 11
#define GPIO3 2  //Physical 15

///////////////////GPIO Motor Control ///////////////////
#define ENA 26 // Physical 
#define IN1 4  // Physical 
#define IN2 5  // Physical 

#define ENB 0  // Physical 
#define IN3 2  // Physical 
#define IN4 3  // Physical 

#define MAX_PWM_DUTY 100
/////////////////// Ultrasonic Sensor///////////////////

#define TRIG 21
#define ECHO 22

/////////////////// Serial Com.///////////////////
#define BAUD_RATE 115200

int getch(void)
{
    int ch;
    struct termios buf;
    struct termios save;

    tcgetattr(0, &save);
    buf = save;
    buf.c_lflag &= ~(ICANON|ECHO);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &buf);
    ch = getchar();
    tcsetattr(0, TCSAFLUSH, &save);
    return ch;
}

int GPIO_control_setup(void)
{

    if (wiringPiSetup() == -1)
    {
        printf("wiringPi Setup error!\n");
        return -1;
    }

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);

    softPwmCreate(ENA, 1, MAX_PWM_DUTY);
    softPwmCreate(ENB, 1, MAX_PWM_DUTY);

    softPwmWrite(ENA, 0);
    softPwmWrite(ENB, 0);

    return 0;
}

void motor_control_r(int pwm)
{
    if (pwm > 0)
    {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        softPwmWrite(ENA, pwm);
    }
    else if (pwm == 0)
    {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        softPwmWrite(ENA, 0);
    }
    else
    {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        softPwmWrite(ENA, -pwm);
    }
}

void motor_control_l(int pwm)
{
    if (pwm > 0)
    {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        softPwmWrite(ENB, pwm);
    }
    else if (pwm == 0)
    {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        softPwmWrite(ENB, 0);
    }
    else
    {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        softPwmWrite(ENB, -pwm);
    }
}


float ultrasonic_sonsor(void)
{
    long start_time, end_time;
    long temp_time1, temp_time2;
    int duration;
    float distance;
    
    digitalWrite(TRIG,LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG,HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG,LOW);
    
    
    delayMicroseconds(200);     //wait for burst signal. 40kHz x 8 = 8 x 25us = 200
    //printf("200msec \n");
    temp_time1 = micros();
    printf("%d\n",temp_time1);
    while(digitalRead(ECHO) == LOW)     // wait until ECHO pin is HIGH 
    {
        
        temp_time2 = micros();
        duration = temp_time2 - temp_time1;
        if(duration >1000) return -1;
    
    
    }
    
    
    
    start_time = micros();
    //printf("echo signal high \n");
    
    while(digitalRead(ECHO) == HIGH)     // wait until ECHO pin is LOW 
    {
        temp_time2 = micros();
        duration = temp_time2 - start_time;
        if(duration >2000) return -1;
    }
    
    
    end_time = micros();
    //printf("echo signal low \n");
    duration = end_time - start_time;
    
    distance = duration/ 58;    
    
    return distance;
}

int main(void)
{
    int pwm_r = 0;
    int pwm_l = 0;
    unsigned char test;
    
    // Initialize camera and set parameters
    int img_width = 640;
    int img_height = 480;
    

   
    
    // Setup motor control
    if (GPIO_control_setup() == -1)
    {
        return -1;
    }
    

    
    while (1)
    {   
        
        
        printf("%6.3lf [cm] \n",ultrasonic_sonsor() );
        delay(100);
        //test = getch();
        test = 'y';
        
        switch (test)
        {
            case 'w':   // Forward
                motor_control_r(pwm_r);
                motor_control_l(pwm_l);
                pwm_r++;
                pwm_l++;
                if (pwm_r > 100) pwm_r = 100;
                if (pwm_l > 100) pwm_l = 100;
                break;
            case 's':   // Stop
                motor_control_r(0);
                motor_control_l(0);
                break;
            case 'x':   // Backward
                motor_control_r(-pwm_r);
                motor_control_l(-pwm_l);
                pwm_r--;
                pwm_l--;
                if (pwm_r < -100) pwm_r = -100;
                if (pwm_l < -100) pwm_l = -100;
                break;
            case 'a':   // Left
                motor_control_r(pwm_r);
                motor_control_l(-pwm_l);
                pwm_r++;
                pwm_l--;
                if (pwm_r > 100) pwm_r = 100;
                if (pwm_l < -100) pwm_l = -100;
                break;
            case 'd':   // Right
                motor_control_r(-pwm_r);
                motor_control_l(pwm_l);
                pwm_r--;
                pwm_l++;
                if (pwm_r < -100) pwm_r = -100;
                if (pwm_l > 100) pwm_l = 100;
                break;
            case 'p':   // Pause and exit
                motor_control_r(0);
                motor_control_l(0);
                return 0;
                break;
        }
        
       
    }
    
    return 0;
}
