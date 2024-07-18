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

#define ENA 26 // Physical pin 37
#define IN1 4  // Physical pin 16
#define IN2 5  // Physical pin 18

#define ENB 0  // Physical pin 11
#define IN3 2  // Physical pin 13
#define IN4 3  // Physical pin 15

#define MAX_PWM_DUTY 100
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

int motor_control_setup(void)
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

void sig_Handler(int sig)
{
    printf("\nProgram and Motor Stopped!\n");
    motor_control_r(0);
    motor_control_l(0);
    exit(0);
}

int main(void)
{
    int pwm_r = 0;
    int pwm_l = 0;
    unsigned char test;
    
    // Initialize camera and set parameters
    int img_width = 640;
    int img_height = 480;
    
    Mat mat_image_org_color;
    
    VideoCapture cap(0);
    
    cap.set(CAP_PROP_FRAME_WIDTH, img_width);
    cap.set(CAP_PROP_FRAME_HEIGHT, img_height);
    
    if (!cap.isOpened())
    {
        cerr << "Camera not opened!\n";
        return -1;
    }
    
    printf("Image size[%3d, %3d]\n", img_width, img_height);
    
    namedWindow("Display window", WINDOW_NORMAL);
    resizeWindow("Display window", img_width, img_height);
    moveWindow("Display window", 10, 10);
    
    // Setup motor control
    if (motor_control_setup() == -1)
    {
        return -1;
    }
    
    // Setup signal handler for Ctrl+C
    signal(SIGINT, sig_Handler);
    
    while (1)
    {
        test = getch();
        
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
        
        // Read frame from camera and display
        if (!cap.isOpened())
        {
            cerr << "Camera not opened!\n";
            return -1;
        }
        
        cap.read(mat_image_org_color);
        imshow("Display window", mat_image_org_color);
        
        // Check for key press to break the loop
        if (waitKey(10) > 0)
            break;
    }
    
    return 0;
}
