#include <cstdio>
#include <mbed.h>

#include "PM2_Drivers.h"


bool do_execute_main_task = false;
bool do_reset_all_once = false;

    //  Konstanten für DC Motor aus PM2_pes_board_example kopiert
    const float max_voltage = 12.0f;               // define maximum voltage of battery packs, adjust this to 6.0f V if you only use one batterypack
    const float counts_per_turn = 20.0f * 78.125f; // define counts per turn at gearbox end: counts/turn * gearratio
    const float kn = 180.0f / 12.0f;               // define motor constant in RPM/V
    const float k_gear = 100.0f / 78.125f;         // define additional ratio in case you are using a dc motor with a different gear box, e.g. 100:1
    const float kp = 0.002f;                         // define custom kp, this is the default speed controller gain for gear box 78.125:1
    const float Wegkorr= 152.7f;                   //Konstanten für Umrechnung Drehung in Strecke
    const float EPS = 0.005;
    float Rest =0;
    float weg = 0;
    
  // Konstanten für Stepper-Motor
    const float winkelcorr= 0.14/180;
    float Dutycicle=0.0f;

DebounceIn user_button(PC_13);  //Blauerknopf
DigitalOut user_led(LED1);        // LED Objekt auf STM Board. LED1 ist Connectet zu PA_5
DigitalOut additional_led(PB_9); // Zusätzliches externes LED Objekt. PB_9 ist noch nicht fix.
Servo servo(PB_12);                // Servo Objekt. PB_2 ist noch nicht fix.
FastPWM pwm_M3(PA_10);            // DC Motor Objekt. Winkel kontrolliert durch Encoder. PA_10 noch nicht fix.
EncoderCounter encoder_M3(PA_0, PA_1); // Encoder Objekt. PA_0 und PA_1 ist noch nicht fix.
DigitalOut enable_motors(PB_15); // create DigitalOut object to enable dc motors
PositionController positionController_M3(counts_per_turn * k_gear, kn / k_gear, max_voltage, pwm_M3, encoder_M3);     
    //positionController_M3.setSpeedCntrlGain(kp * k_gear); 

void user_button_pressed_fcn();

void move(float dist, float vel, float acc) {
     printf("Weg: %3.3f %3.3f %3.3f %3.3f %3.3f\n", dist,vel,acc,Rest+(dist),positionController_M3.getRotation());
    enable_motors = 1;
    positionController_M3.setDesiredRotation(Rest+(dist));
    positionController_M3.setMaxVelocityRPS(vel);         
    positionController_M3.setMaxAccelerationRPS(acc); 
    ThisThread::sleep_for(5000ms);
    Rest=positionController_M3.getRotation();
   
   

}

void lift(float ang){

        while (Dutycicle <(ang*winkelcorr)) { //Robi Neutrale Stellung
        servo.setNormalisedAngle(Dutycicle);
        Dutycicle +=0.0002f;
        ThisThread::sleep_for(25ms);}
}

void fall(float ang){

        while (Dutycicle >(ang*winkelcorr)) { //Robi Neutrale Stellung
        servo.setNormalisedAngle(Dutycicle);
        Dutycicle -=0.0002f;
        ThisThread::sleep_for(21ms);}
}

int main() {

user_button.fall(&user_button_pressed_fcn);

    // Erstellte konstanten für Switch Zustände
    const int arm_strecken_bis_flach = 1;
    const int bis_zum_Hindernis_fahren = 2;
    const int arm_heben_bis_hoehe_hindernis = 3;
    const int fahren_bis_gelenk_ueber_hindernis = 4;
    const int arm_strecken_bis_bodenkontakt = 5;
    const int fahren_bis_hinderniskante = 6;
    const int arm_heben_bis_flach = 7;
    const int fahren_bis_ziel = 8;
    const int arm_heben_in_ausgangsposition = 9;


    int zustand = arm_strecken_bis_flach;
    bool schleife = true;
    servo.disable();
    enable_motors = 0; 

    additional_led=0;
    printf("Warten\n");
    while(do_execute_main_task==false){
        additional_led=!additional_led;
        ThisThread::sleep_for(100ms);
    }
        
  
    servo.enable();
    servo.setNormalisedAngle(80);       //Robi Startposition 90Grad nach unten
    
    

    while (schleife) {


        additional_led=1;
        printf("Start\n");

        // Switch Funktion für verschiedenen Teilabläufe
         switch (zustand) {


                case arm_strecken_bis_flach: 
                printf("Case 1\n");

                fall(80);
                ThisThread::sleep_for(4000ms);
                zustand = bis_zum_Hindernis_fahren;
                break;

                
                case bis_zum_Hindernis_fahren:
                printf("Case 2\n");

                move(5.2f, 1.0f, 0.5f);
                ThisThread::sleep_for(4000ms);


                zustand = arm_heben_bis_hoehe_hindernis;
                break;



                case arm_heben_bis_hoehe_hindernis:
                printf("Case 3\n");
                
                lift(100);
                move(0.5f, 0.6f, 0.5f);
                ThisThread::sleep_for(6000ms);
                
                zustand = fahren_bis_gelenk_ueber_hindernis;
                break;

                case fahren_bis_gelenk_ueber_hindernis:
                printf("Case 4\n");


                move(4.0f, 0.5f, 0.6f);
                ThisThread::sleep_for(5000ms);

                zustand = arm_strecken_bis_bodenkontakt;
                break;


                case arm_strecken_bis_bodenkontakt:
                printf("Case 5\n");

                fall(0);
                ThisThread::sleep_for(3000ms);

                
                zustand = fahren_bis_hinderniskante;
                break;

                move(5.0f, 0.4f, 0.4f);
                ThisThread::sleep_for(5000ms);

                case fahren_bis_hinderniskante:
                printf("Case 6\n");

                                     
                zustand = arm_heben_bis_flach;
                break;

                lift(80);
                //ThisThread::sleep_for(5000ms);

                case arm_heben_bis_flach:
                printf("Case 7\n");


                ThisThread::sleep_for(1000ms);
                zustand = fahren_bis_ziel;
                break;

                move(5.0f, 1.0f, 0.3f);
                case fahren_bis_ziel:
                printf("Case 8\n");


                ThisThread::sleep_for(1000ms);
                zustand = arm_heben_in_ausgangsposition;
                break;



                case arm_heben_in_ausgangsposition:
                printf("Case 9\n");

                lift(130);
                ThisThread::sleep_for(1000ms);
                zustand = arm_strecken_bis_flach;
                servo.disable();
                break;



                default:
                printf("Default\n");
                break;
            }

    

    }
    

printf("Ende gelende \n");
}

void user_button_pressed_fcn()
{
    // do_execute_main_task if the button was pressed
    do_execute_main_task = !do_execute_main_task;
    if (do_execute_main_task) do_reset_all_once = true;
}