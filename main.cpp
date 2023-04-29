#include <cstdio>
#include <mbed.h>

#include "PM2_Drivers.h"


// Knopf
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
    float Dutycicle=0.14f;

//Board Input

    DebounceIn user_button(PC_13);  //Blauerknopf
    DigitalOut user_led(LED1);        // LED Objekt auf STM Board. LED1 ist Connectet zu PA_5
    DigitalOut additional_led(PB_9); // Zusätzliches externes LED Objekt. PB_9 ist noch nicht fix.
    Servo servo(PB_12);                // Servo Objekt. PB_2 ist noch nicht fix.
    FastPWM pwm_M3(PA_10);            // DC Motor Objekt. Winkel kontrolliert durch Encoder. PA_10 noch nicht fix.
    EncoderCounter encoder_M3(PA_0, PA_1); // Encoder Objekt. PA_0 und PA_1 ist noch nicht fix.
    DigitalOut enable_motors(PB_15); // create DigitalOut object to enable dc motors
    PositionController positionController_M3(counts_per_turn * k_gear, kn / k_gear, max_voltage, pwm_M3, encoder_M3);//positionController_M3.setSpeedCntrlGain(kp * k_gear);    
    

void user_button_pressed_fcn();

void move(float dist, float vel, float acc) {
     printf("Weg: %3.3f %3.3f %3.3f %3.3f %3.3f\n", dist,vel,acc,Rest+(dist),positionController_M3.getRotation());
    enable_motors = 1;
    Rest=positionController_M3.getRotation();
    ThisThread::sleep_for(1000ms);
    positionController_M3.setDesiredRotation(Rest+(dist));
    positionController_M3.setMaxVelocityRPS(vel);         
    positionController_M3.setMaxAccelerationRPS(acc); 
    
    
   
   

}

void lift(float ang){

        while (Dutycicle <(ang*winkelcorr)) { //Robi Neutrale Stellung
        servo.setNormalisedAngle(Dutycicle);
        Dutycicle +=0.0003f;
        ThisThread::sleep_for(10ms);}
}

void fall(float ang){

        while (Dutycicle >(ang*winkelcorr)) { //Robi Neutrale Stellung
        servo.setNormalisedAngle(Dutycicle);
        Dutycicle -=0.0003f;
        ThisThread::sleep_for(10ms);}
}

int main() {




user_button.fall(&user_button_pressed_fcn);

    int zustand = 1;
    bool schleife = true;
    
    servo.enable();
    servo.setNormalisedAngle(0.14f);
    enable_motors = 0; 
    


    additional_led=0;
    printf("Warten\n");
    while(do_execute_main_task==false){
        additional_led=!additional_led;
        ThisThread::sleep_for(100ms);
    }

    while (schleife) {

        additional_led=1;
        printf("Start\n");

        // Switch Funktion für verschiedenen Teilabläufe
         switch (zustand) {


                case 1: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 1\n");

                move(-1.0f, 1.0f, 0.7f);
                
                ThisThread::sleep_for(3000ms);

                zustand ++;
                break;
                case 2: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 2\n");

                fall(93.0);
                move(6.4f, 1.0f, 0.7f);
                ThisThread::sleep_for(8000ms);

                zustand ++;
                break;
                case 3: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 3\n");
                
                lift(120.0);
                ThisThread::sleep_for(2000ms);
                
                zustand ++;
                break;
                case 4: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 4\n");


                move(2.2f, 0.6f, 0.6f);
                ThisThread::sleep_for(6000ms);

                zustand ++;
                break;
                case 5: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 5\n");

                fall(70.0);
                move(2.4f, 0.6f, 0.6f);
                ThisThread::sleep_for(6000ms);

                
                zustand ++;
                break;
                case 6: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 6\n");

               
                lift(93.0);
                ThisThread::sleep_for(2000ms);
                                     
                zustand ++;
                break;
                case 7: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 7\n");

                move(1.2f, 0.5f, 0.7f);
                ThisThread::sleep_for(3000ms);

                zustand ++;
                break;
                case 8: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 8\n");

                
                lift(120.0);
                move(2.0f, 0.5f, 0.8f);
                ThisThread::sleep_for(3000ms);


                zustand ++;
                break;
                case 9: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 9\n");

                
                move(8.5f, 1.0f, 0.5f);
                fall(93.0);
                ThisThread::sleep_for(12000ms);

                zustand ++;
                break;
                case 10: ///////////////////////////////////////////////////////////////////////////////////////
                printf("Case 10\n");

                //move(7.0f, 1.0f, 0.7f);
                //ThisThread::sleep_for(5000ms);


                zustand ++;
                break;
                case 11: ///////////////////////////////////////////////////////////////////////////////////////

                lift(180.0);
                schleife=false;
                break;

                default:
                printf("Default\n");
                break;
            }
    }
    
printf("Ende gelende \n");
}

void user_button_pressed_fcn(){

    // do_execute_main_task if the button was pressed
    do_execute_main_task = !do_execute_main_task;
    if (do_execute_main_task) do_reset_all_once = true;
}