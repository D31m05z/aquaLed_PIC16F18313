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
uint8_t state = 0;
uint16_t dataeeAddr = 0xF010;

// initialize eeprom with zeroes 0xF000 - 0xF01F
__eeprom unsigned char eeprom_values[32] =
        {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //  0xF000 - 0xF007
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //  0xF008 - 0xF00F

            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //  0xF010 - 0xF017
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   //  0xF018 - 0xF01F
        };

typedef enum PwmChannel {
    ALL  = 0,
    PWM1 = 1,
    PWM2 = 2,
    PWM5 = 3,
    PWM6 = 4
} PwmChannel_t;

typedef enum PanelType {
    SMALL   = 0,
    BIG     = 1
} PanelType_t;

/**
 * Read button state and debugging. Only the press is detected.
 * Blocking function! Not returns until button is not released!
 * @return button pressed or not
 */
bool ButtonChangeCheck(void);

void setPWMValues(uint16_t dutyValue, const PwmChannel_t pwmMode);

/**
 * Initialize led driver
 */
void initialize(void)
{
    // initialize the device
    SYSTEM_Initialize();

    // start TMR2 timer
    TMR2_StartTimer();

    // initialize state machine from memory
    state = DATAEE_ReadByte(dataeeAddr);
}

/**
 * Light driving logic
 */
void loop_small(void);
void loop_big(void);

/**
 * Main
 */
void main(void)
{
    const PanelType_t panelType = BIG;

    // initialize
    initialize();

    // main loop
    while (true) {
        if(ButtonChangeCheck()) {
            // if button press detected change state
            ++state;

            // store state value to memory
            DATAEE_WriteByte(dataeeAddr, state);
        }

        // execute state machine
        switch(panelType) {
            case SMALL:
                loop_small();
                break;
            case BIG:
                loop_big();
                break;
            default:
                loop_small();
        }
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

// small panel
void loop_small(void) {
    switch(state) {
    case 0: // initialize state
        setPWMValues(0x00, ALL); //Switch off
        state = 1;
        break;
                                   //--+--------------------------------+
                                   //  |      SMALL     |      BIG      |
                                   //--+----------------+---------------+
    case 1:                        //  |                |               |
        PWM2_LoadDutyValue(2);     // R|   255 - 3.67V  |  255 - 3.22V  |
        PWM1_LoadDutyValue(2);     // G|   255 - 2.40V  |  255 - 1.89V  |
        PWM5_LoadDutyValue(2);     // B|   255 - 1.97V  |  255 - 1.46V  |
        PWM6_LoadDutyValue(2);     // W|   255 - 1.10V  |  255 - 1.08V  |
        break;                     //  |                |               |
    case 2:                        //--+----------------+---------------+
        PWM2_LoadDutyValue(18);    // R|    18 - 0.27V  |   18 - 0.25V  |
        PWM1_LoadDutyValue(21);    // G|    21 - 0.18V  |   21 - 0.15V  |
        PWM5_LoadDutyValue(14);    // B|    14 - 0.1V   |   14 - 0.08V  |
        PWM6_LoadDutyValue(19);    // W|    19 - 0.07V  |   19 - 0.06V  |
        break;                     //--+----------------+---------------+
    case 3:                        //  |                |               |
        PWM2_LoadDutyValue(38);    // R|    38 - 0.55V  |   38 - 0.49V  |
        PWM1_LoadDutyValue(43);    // G|    43 - 0.37V  |   43 - 0.30V  |
        PWM5_LoadDutyValue(26);    // B|    26 - 0.19V  |   26 - 0.15V  |
        PWM6_LoadDutyValue(38);    // W|    38 - 0.14V  |   38 - 0.11V  |
        break;                     //--+----------------+---------------+
    case 4:                        //  |                |               |
        PWM2_LoadDutyValue(85);    // R|    85 - 1.2V   |   85 - 1.02V  |
        PWM1_LoadDutyValue(86);    // G|    86 - 0.74V  |   86 - 0.54V  |
        PWM5_LoadDutyValue(54);    // B|    54 - 0.39V  |   54 - 0.29V  |
        PWM6_LoadDutyValue(75);    // W|    75 - 0.29V  |   75 - 0.19V  |
        break;                     //--+----------------+---------------+
    case 5:                        //  |                |               |
        PWM2_LoadDutyValue(131);   // R|   131 - 1.86V  |  131 - 1.58V  |
        PWM1_LoadDutyValue(131);   // G|   131 - 1.15V  |  131 - 0.86V  |
        PWM5_LoadDutyValue(83);    // B|   83  - 0.6V   |   83 - 0.43V  |
        PWM6_LoadDutyValue(113);   // W|   113 - 0.44V  |  113 - 0.29V  |
        break;                     //--+----------------+---------------+
    case 6:                        //  |                |               |
        PWM2_LoadDutyValue(182);   // R|   182 - 2.61V  |  182 - 2.27V  |
        PWM1_LoadDutyValue(174);   // G|   174 - 1.6V   |  174 - 1.22V  |
        PWM5_LoadDutyValue(110);   // B|   110 - 0.82V  |  110 - 0.59V  |
        PWM6_LoadDutyValue(150);   // W|   150 - 0.62V  |  150 - 0.42V  |
        break;                     //--+----------------+---------------+
    case 7:                        //  |                |               |
        PWM2_LoadDutyValue(225);   // R|   225 - 3.28V  |  225 - 2.92V  |
        PWM1_LoadDutyValue(214);   // G|   214 - 2.06V  |  214 - 1.6V   |
        PWM5_LoadDutyValue(138);   // B|   138 - 1.05V  |  138 - 0.74V  |
        PWM6_LoadDutyValue(188);   // W|   188 - 0.8V   |  188 - 0.58V  |
        break;                     //--+----------------+---------------+
    default:
        setPWMValues(0x00, ALL);   //Switch off
        state = 0;
    }
}

// big panel
void loop_big(void) {
    switch(state) {
    case 0: // initialize state
        setPWMValues(0x00, ALL); //Switch off
        state = 1;
        break;
                                   //--+--------------------------------+--------------+
                                   //  |      SMALL     |      BIG      |  MANUAL BIG  |
                                   //--+----------------+---------------+--------------+
    case 1:                        //  |                |               |              |
        PWM2_LoadDutyValue(2);     // R|   255 - 3.67V  |  255 - 3.24V  |              |
        PWM1_LoadDutyValue(2);     // G|   255 - 2.40V  |  255 - 1.92V  |              |
        PWM5_LoadDutyValue(2);     // B|   255 - 1.97V  |  255 - 1.46V  |              |
        PWM6_LoadDutyValue(2);     // W|   255 - 1.10V  |  255 - 1.08V  |              |
        break;                     //  |                |               |              |
    case 2:                        //--+----------------+---------------+--------------+
        PWM2_LoadDutyValue(20);    // R|    18 - 0.27V  |   18 - 0.25V  |   20 - 0.27V |
        PWM1_LoadDutyValue(25);    // G|    21 - 0.18V  |   21 - 0.15V  |   25 - 0.18V |
        PWM5_LoadDutyValue(17);    // B|    14 - 0.1V   |   14 - 0.08V  |   17 - 0.1V  |
        PWM6_LoadDutyValue(22);    // W|    19 - 0.07V  |   19 - 0.06V  |   22 - 0.07V |
        break;                     //--+----------------+---------------+--------------+
    case 3:                        //  |                |               |              |
        PWM2_LoadDutyValue(43);    // R|    38 - 0.55V  |   38 - 0.49V  |   43 - 0.56V |
        PWM1_LoadDutyValue(51);    // G|    43 - 0.37V  |   43 - 0.30V  |   51 - 0.37V |
        PWM5_LoadDutyValue(31);    // B|    26 - 0.19V  |   26 - 0.15V  |   31 - 0.18V |
        PWM6_LoadDutyValue(43);    // W|    38 - 0.14V  |   38 - 0.11V  |   43 - 0.13V |
        break;                     //--+----------------+---------------+--------------+
    case 4:                        //  |                |               |              |
        PWM2_LoadDutyValue(98);    // R|    85 - 1.2V   |   85 - 1.02V  |   98 - 1.21V |
        PWM1_LoadDutyValue(107);   // G|    86 - 0.74V  |   86 - 0.54V  |  107 - 0.74V |
        PWM5_LoadDutyValue(69);    // B|    54 - 0.39V  |   54 - 0.29V  |   69 - 0.39V |
        PWM6_LoadDutyValue(105);   // W|    75 - 0.29V  |   75 - 0.19V  |  105 - 0.29V |
        break;                     //--+----------------+---------------+--------------+
    case 5:                        //  |                |               |              |
        PWM2_LoadDutyValue(150);   // R|   131 - 1.86V  |  131 - 1.58V  |  150 - 1.84V |
        PWM1_LoadDutyValue(162);   // G|   131 - 1.15V  |  131 - 0.86V  |  162 - 1.15V |
        PWM5_LoadDutyValue(110);   // B|   83  - 0.6V   |   83 - 0.43V  |  110 - 0.6V  |
        PWM6_LoadDutyValue(151);   // W|   113 - 0.44V  |  113 - 0.29V  |  151 - 0.44V |
        break;                     //--+----------------+---------------+--------------+
    case 6:                        //  |                |               |              |
        PWM2_LoadDutyValue(206);   // R|   182 - 2.61V  |  182 - 2.27V  |  206 - 2.6V  |
        PWM1_LoadDutyValue(216);   // G|   174 - 1.6V   |  174 - 1.22V  |  216 - 1.59V |
        PWM5_LoadDutyValue(146);   // B|   110 - 0.82V  |  110 - 0.59V  |  146 - 0.8V  |
        PWM6_LoadDutyValue(196);   // W|   150 - 0.62V  |  150 - 0.42V  |  196 - 0.6V  |
        break;                     //--+----------------+---------------+--------------+
    case 7:                        //  |                |               |              |
        PWM2_LoadDutyValue(255);   // R|   225 - 3.28V  |  225 - 2.92V  |  255 - 3.24V |
        PWM1_LoadDutyValue(255);   // G|   214 - 2.06V  |  214 - 1.6V   |  255 - 1.92V |
        PWM5_LoadDutyValue(183);   // B|   138 - 1.05V  |  138 - 0.74V  |  183 - 1.03V |
        PWM6_LoadDutyValue(245);   // W|   188 - 0.8V   |  210 - 0.58V  |  245 - 0.78V |
        break;                     //--+----------------+---------------+--------------+
    default:
        setPWMValues(0x00, ALL);   //Switch off
        state = 0;
    }
}

void loop_for_demo(void) {
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