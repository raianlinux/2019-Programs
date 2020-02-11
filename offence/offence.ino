//version 6.1

///program for main game///
#define program_for_game //comment out this line when it is not real game

///set debug options///
#define DEBUG
#define DEBUG_Gyro_sensor //use this when debug gyro sensor in this program
#define DEBUG_color_angle //use this when debug pixy in this program

///library files///
#include <FlexiTimer2.h>
#include <avr/io.h>

///my header files///
#include "Motor_drive_VNH.h" //all function use percentage. set motor driver pin && VNH_pwm(degree,percentage),VNH1(pwm_value),VNH2(pwm_value),VNH3(vpwm_value),VNH4(pwm_value),VNH_rotate(percentage) function.
#include "White_line.h"      //use it by flexitimer2 (timer interrapt)
#include "pixy2_get_color_info.h" //set start_pixy2 , get angle orange , get angle yellow , get angle blue , get_dist_orange , get_dist_yellow , get_dist_blue
#include "esc_control.h"   //operating esc(brushless motor) , speed up or speed down
#include "communication.h" //set function get_robot_angle

///goal color///
bool goal = true; //true : yellow , false : blue

///ball status///
bool ball; //ball true or false

///angles///
//in this program,all angle defined as RAD//
float angle_orange;
float angle_blue;
float angle_yellow;

float angle; //where you want to go

float robot_angle;  //get from arduino pro mini , robots yaw degree

///distance///
float dist_orange;
float dist_blue;
float dist_yellow;

float dist_IR [4]; //get from arduino pro mini , [forward,back,right,left]

///start button///
const int start_button = 48;

///solenoid FET///
const int sloenoid_FET = 47;

///ball caught sensor///
const int ball_sensor = 49;
//////

void setup() {
  ///pin setup here///
  pinMode(PWM_1, OUTPUT); //pin 5
  pinMode(PWM_2, OUTPUT); //pin 6
  pinMode(PWM_3, OUTPUT); //pin 7
  pinMode(PWM_4, OUTPUT); //pin 8
  pinMode(A_1, OUTPUT);   //pin 23
  pinMode(A_2, OUTPUT);   //pin 27
  pinMode(A_3, OUTPUT);   //pin 31
  pinMode(A_4, OUTPUT);   //pin 35
  pinMode(B_1, OUTPUT);   //pin 25
  pinMode(B_2, OUTPUT);   //pin 29
  pinMode(B_3, OUTPUT);   //pin 33
  pinMode(B_4, OUTPUT);   //pin 37

  pinMode(start_button, INPUT_PULLUP); //start button = pin 48

  pinMode(sloenoid_FET, OUTPUT); //sloenoid_FET = pin 47

  pinMode(ball_sensor, INPUT); //ball caught sensor = pin 49

  //////

  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);

  ///change PWM freqency///
  TCCR3B = (TCCR3B & 0b11111000) | 1;
  TCCR4B = (TCCR4B & 0b11111000) | 1;

  ///set timer interrupt///
  FlexiTimer2::set(1, 1.0 / 7500, White_line);
  FlexiTimer2::start();

  ///pixy setup///
  start_pixy2();

  ///esc setup///
  //esc_setup();

///wait for start///
#ifdef program_for_game
  while (digitalRead(start_button) == 1);
#endif
}

void loop() {
#ifdef DEBUG
  #ifdef DEBUG_Gyro_sensor
    robot_angle = get_robot_angle();
  #endif

  #ifdef DEBUG_color_angle
    angle_orange = get_angle_orange();
  #endif

  #ifdef DEBUG_Gyro_sensor
    Serial.println(robot_angle * 180/PI);
  #endif

  #ifdef DEBUG_color_angle
    Serial.println(angle_orange * 180/PI);
  #endif
#endif

  angle_orange = get_angle_orange(); //get orange ball angle

  robot_go_angle(); //set where the robot go to

  ///if robot lost ball position,jump to ball_catch()///
  if(angle_orange == get_angle_orange() == get_angle_orange()){
    ball_catch();
  }

  if (-PI <= angle_orange <= PI) {
    VNH_pwm(angle, 100);
  }

  ball_catch();
   
  ///Angle adjustment once in 10 routines///
  int count = count + 1;
  if(count = 10){
    count = 0;
    if (goal = true) {
      robot_angle = get_robot_angle();
      angle_yellow = get_angle_yellow();
      while (((-PI / 2 <= angle_yellow) && (angle_yellow < PI / 3)) || ((PI / 18<= robot_angle) && (robot_angle < PI))) {
        VNH_rotate(-50);
        robot_angle = get_robot_angle();
        angle_yellow = get_angle_yellow();
      }
      while (((2 * PI / 3 <= angle_yellow) && (angle_yellow < PI)) || ((-PI <= angle_yellow) && (angle_yellow < -PI / 2)) || ((-PI <= robot_angle) && (robot_angle < -PI / 18))) {
        VNH_rotate(50);
        robot_angle = get_robot_angle();
        angle_yellow = get_angle_yellow();
      }
    }
    if (goal = false) {
      robot_angle = get_robot_angle();
      angle_blue = get_angle_blue();
      while (((-PI / 2 <= angle_blue) && (angle_blue < PI / 3)) || ((1/18 * PI <= robot_angle) && (robot_angle < PI))) {
        VNH_rotate(-50);
        robot_angle = get_robot_angle();
        angle_blue = get_angle_blue();
      }
      while (((2/3 * PI <= angle_blue) && (angle_blue < PI)) || ((-PI <= angle_blue) && (angle_blue < -PI / 2)) || ((-PI <= robot_angle) && (robot_angle < -PI / 18))) {
        VNH_rotate(50);
        robot_angle = get_robot_angle();
        angle_blue = get_angle_blue();
      }
    }
  }
  //control robot program start and stop
  if(digitalRead(start_button) == 0){
    VNH_pwm(0,0);
    while(start_button == 1);
  }
}

///ball caught///
void ball_catch() {
  if (digitalRead(ball_sensor) == digitalRead(ball_sensor) == digitalRead(ball_sensor) == 1) {
    bool kicked;
    //esc_speed_up();
    FlexiTimer2::stop();
    if (goal = true) {
      angle_yellow = get_angle_yellow();
      while ((-PI / 2 <= angle_yellow) && (angle_yellow < PI / 3)) {
        VNH_pwm(0,50);
        angle_yellow = get_angle_yellow();
      }
      while (((2/3 * PI <= angle_yellow) && (angle_yellow < PI)) || ((-PI <= angle_yellow) && (angle_yellow < -PI / 2))) {
        VNH_pwm(PI,50);
        angle_yellow = get_angle_yellow();
      }

      dist_yellow = get_dist_yellow();

      if (dist_yellow <= 20) {
        while (((0 <= angle_yellow) && (angle_yellow < 4/9 * PI)) || ((-PI / 2 <= angle_yellow) && (angle_yellow < 0))) {
          VNH_pwm(0,40);
          angle_yellow = get_angle_yellow();
        }

        while (((5/9 * PI <= angle_yellow) && (angle_yellow < PI)) || ((-PI <= angle_yellow) && (angle_yellow < -PI / 2))) {
          VNH_pwm(PI,40);
          angle_yellow = get_angle_yellow();
        }
          
        angle_yellow = get_angle_yellow();

        if ((4/9 * PI <= angle_yellow) && (angle_yellow <= 5/9 * PI)) {
          //esc_speed_down();
          digitalWrite(sloenoid_FET, HIGH);
          delay(10);
          digitalWrite(sloenoid_FET, LOW);
          kicked = true;
        }
      }
      else VNH_pwm(angle,70); 
    }
      
    if (goal = false) {
      angle_blue = get_angle_blue();
      while ((-PI / 2 <= angle_yellow) && (angle_yellow < PI / 3)) {
        VNH_pwm(0,50);
        angle_blue = get_angle_blue();
      }
      while (((2/3 * PI <= angle_yellow) && (angle_yellow < PI)) || ((-PI <= angle_yellow) && (angle_yellow < -PI / 2))) {
        VNH_pwm(PI,50);
        angle_blue = get_angle_blue();
      }

      dist_blue = get_dist_blue();

      if (dist_blue <= 20) {
        while (((0 <= angle_yellow) && (angle_yellow < 4/9 * PI)) || ((-PI / 2 <= angle_yellow) && (angle_yellow < 0))) {
          VNH_pwm(0,40);
          angle_blue = get_angle_blue();
        }

        while (((5/9 * PI <= angle_yellow) && (angle_yellow < PI)) || ((-PI <= angle_yellow) && (angle_yellow < -PI / 2))) {
          VNH_pwm(PI,40);
          angle_blue = get_angle_blue();
        }

        angle_blue = get_angle_blue();

        if ((4/9 * PI <= angle_yellow) && (angle_yellow <= 5/9 * PI)) {
          //esc_speed_down();
          digitalWrite(sloenoid_FET, HIGH);
          delay(10);
          digitalWrite(sloenoid_FET, LOW);
          kicked = true;
        }
      }
      else VNH_pwm(angle,70);
    }
    if(digitalRead(ball_sensor) == 0 && kicked == false){
      //esc_speed_down();
    }
  FlexiTimer2::start();
  kicked = false;
  }
}

///set where the robot have to go///
void robot_go_angle(){
  if ((angle_orange < -PI) || (PI < angle_orange)) {
    ball_catch();
  }
  else if ((PI / 4 <= angle_orange) && (angle_orange <= 3 * PI / 4)) {
      angle = angle_orange;
<<<<<<< HEAD
  }
  else if ((0 <= angle_orange) && (angle_orange < PI / 4)) {
      angle = -PI / 4;
  }
  else if ((3 * PI / 4< angle_orange) && (angle_orange <= PI)) {
      angle = -3 * PI / 4;
  }
  else if (((-PI / 4 <= angle_orange) && (angle_orange < 0)) || ((-PI < angle_orange) && (angle_orange <= -3 * PI / 4))) {
      angle = -PI / 2;
  }
  else if ((-3 * PI / 4 < angle_orange) && (angle_orange <= -PI / 2)) {
      angle = -PI / 6;
  }
  else if ((-PI / 2 < angle_orange) && (angle_orange <= -PI / 4)) {
      angle = -5 * PI / 6;
  }
}