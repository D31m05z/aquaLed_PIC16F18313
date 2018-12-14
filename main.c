/*
 * 4 channel LED PWM driver
 *
RA0/ICSPDAT  ->  Programmer PGD and 1. channel (Green)
RA1/ICSPCLK  ->  Programmer PGC and 2. channel (Red)
RA2          ->  3. channel (Blue)
MCLR/RA3     ->  pull-up resistor and programmer MCLR
RA4          ->  4. channel (White)
RA5          ->  button
VDD          ->  5V
VSS          ->  GND
 *
 */

#include "mcc_generated_files/mcc.h"

//Global variables
char state = 0;

typedef enum PwmChannel {
    ALL  = 0,
    PWM1 = 1,
    PWM2 = 2,
    PWM5 = 3,
    PWM6 = 4
} PwmChannel_t;

/**
 * Read button state and debugging. Only the press is detected.
 * Blocking function! Not returns until button is not released!
 * @return button pressed or not
 */
bool ButtonChangeCheck(void);

/**
 * Initialize led driver
 */
void initialize(void)
{
    // initialize the device
    SYSTEM_Initialize();

    // start TMR2 timer
    TMR2_StartTimer();

    // initialize state machine
    state = 0;
}

/**
 * Light driving logic
 */
void loop(void);

/**
 * Main
 */
void main(void)
{
    // initialize
    initialize();

    // main loop
    while (true) {
        if(ButtonChangeCheck()) {
            // if button press detected change state
            ++state;
        }

        // execute state machine
        loop();
    }
}

void setPWMValues(uint16_t dutyValue, const PwmChannel_t pwmMode) {
    switch(pwmMode) {
        case ALL:
            PWM1_LoadDutyValue(dutyValue);
            PWM2_LoadDutyValue(dutyValue);
            PWM5_LoadDutyValue(dutyValue);
            PWM6_LoadDutyValue(dutyValue);
            break;
        case PWM1:
            PWM1_LoadDutyValue(dutyValue);
            PWM2_LoadDutyValue(0);
            PWM5_LoadDutyValue(0);
            PWM6_LoadDutyValue(0);
            break;
        case PWM2:
            PWM1_LoadDutyValue(0);
            PWM2_LoadDutyValue(dutyValue);
            PWM5_LoadDutyValue(0);
            PWM6_LoadDutyValue(0);
            break;
        case PWM5:
            PWM1_LoadDutyValue(0);
            PWM2_LoadDutyValue(0);
            PWM5_LoadDutyValue(dutyValue);
            PWM6_LoadDutyValue(0);
            break;
        case PWM6:
            PWM1_LoadDutyValue(0);
            PWM2_LoadDutyValue(0);
            PWM5_LoadDutyValue(0);
            PWM6_LoadDutyValue(dutyValue);
            break;
        default:
            PWM1_LoadDutyValue(dutyValue);
            PWM2_LoadDutyValue(dutyValue);
            PWM5_LoadDutyValue(dutyValue);
            PWM6_LoadDutyValue(dutyValue);
    }
}

void random(void) {
    uint16_t maxValue = 40;
    setPWMValues(rand() % maxValue, rand() % 5); // 0-39
   __delay_ms(1000);
}

void blinking(void) {
    static uint16_t dutyCycleMin = 0;
    static uint16_t dutyCycleMax = 128;
    static char pwmID = 1;

    for(uint16_t dutyCycle = dutyCycleMin; dutyCycle < dutyCycleMax; dutyCycle++) {
        setPWMValues(dutyCycle, pwmID);
        __delay_ms(10);
    }

    for(uint16_t dutyCycle = dutyCycleMax; dutyCycle > dutyCycleMin; dutyCycle--) {
        setPWMValues(dutyCycle, pwmID);
        __delay_ms(10);
    }

    if(++pwmID > 4) {
        pwmID = 1;
    }
}

void loop(void) {
    switch(state) {
        case 0: // initialize state
            setPWMValues(0x00, ALL); //Switch off
            state = 1;
            break;
        case 1: // random led
            random();
            break;
        case 2: // blinking led
            blinking();
            break;
        case 3:
            setPWMValues(0x00FF, PWM1);
            break;
        case 4:
            setPWMValues(0x00FF, PWM2);
            break;
        case 5:
            setPWMValues(0x00FF, PWM5);
            break;
        case 6:
            setPWMValues(0x00FF, PWM6);
            break;
        default:
            setPWMValues(0x00, ALL); //Switch off
            state = 0;
    }
}

bool ButtonChangeCheck(void) {
    if(!Button_GetValue()) {
        //button is active low
        __delay_ms(10); //wait for prell
        while(!Button_GetValue())
            ; //wait until release
        __delay_ms(10); //wait for prell
        return true; // button pressed
    }
    return false;
}

/**
 End of File
*/