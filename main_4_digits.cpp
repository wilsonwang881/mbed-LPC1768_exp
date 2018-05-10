#include "mbed.h"
#include "Adafruit_SSD1306.h"

//Switch input definition
#define SW_PIN0 p21
#define SW_PIN1 p22
#define SW_PIN2 p23
#define SW_PIN3 p24

//Sampling period for the switch oscillator (us)
#define SW_PERIOD 20000
#define PWM_PERIOD 1000 

//Display interface pin definitions
#define D_MOSI_PIN p5
#define D_CLK_PIN p7
#define D_DC_PIN p8
#define D_RST_PIN p9
#define D_CS_PIN p10

#define DIGITAL_OUT p25


//an SPI sub-class that sets up format and clock speed
class SPIPreInit : public SPI
{
public:
    SPIPreInit(PinName mosi, PinName miso, PinName clk) : SPI(mosi,miso,clk)
    {
        format(8,3);
        frequency(2000000);
    };
};

//Interrupt Service Routine prototypes (functions defined below)
void sedgeC3();
void sedgeC2();
void sedgeC4();
void sedgeC5();
void toutC3();
void toutC2();
void toutC4();
void toutC5();
void wavegenerator();

//Output for the alive LED
DigitalOut alive(LED1);

DigitalOut dig_out(DIGITAL_OUT);

//External interrupt input from the switch oscillator
InterruptIn swinC3(SW_PIN1);
InterruptIn swinC2(SW_PIN0);
InterruptIn swinC4(SW_PIN2);
InterruptIn swinC5(SW_PIN3);

//Switch sampling timer
Ticker swtimerC3;
Ticker swtimerC2;
Ticker swtimerC4;
Ticker swtimerC5;
Ticker wavetimer;

PwmOut PwmPin(p26);

//Registers for the switch counter, switch counter latch register and update flag
volatile uint16_t scounterC2=0;
volatile uint16_t scountC2=0;
volatile uint16_t updateC2=0;

volatile uint16_t scounterC3=0;
volatile uint16_t scountC3=0;
volatile uint16_t updateC3=0;

volatile uint16_t scounterC4=0;
volatile uint16_t scountC4=0;
volatile uint16_t updateC4=0;

volatile uint16_t scounterC5=0;
volatile uint16_t scountC5=0;
volatile uint16_t updateC5=0;

volatile int wave_count = 0;
volatile int total = 1000000;

//Initialise SPI instance for communication with the display
SPIPreInit gSpi(D_MOSI_PIN,NC,D_CLK_PIN); //MOSI,MISO,CLK

//Initialise display driver instance
Adafruit_SSD1306_Spi gOled1(gSpi,D_DC_PIN,D_RST_PIN,D_CS_PIN,64,128); //SPI,DC,RST,CS,Height,Width

int main() { 
    //Initialisation
    gOled1.setRotation(2); //Set display rotation
    
    //Attach switch oscillator counter ISR to the switch input instance for a rising edge
    swinC3.rise(&sedgeC3);
    swinC2.rise(&sedgeC2);
    swinC4.rise(&sedgeC4);
    swinC5.rise(&sedgeC5);
    
    //Attach switch sampling timer ISR to the timer instance with the required period
    swtimerC3.attach_us(&toutC3, SW_PERIOD);
    swtimerC2.attach_us(&toutC2, SW_PERIOD);
    swtimerC4.attach_us(&toutC4, SW_PERIOD);
    swtimerC5.attach_us(&toutC5, SW_PERIOD);
//    wavetimer.attach_us(&wavegenerator, PWM_PERIOD);
    
    
    //Write some sample text
    //gOled1.printf("%ux%u C3 is: \r\n", 64, 128);
    
    int valC3 = 0; //MSB
    bool C3on_pre = false;
    bool C3on_after = false;
    int valC2 = 0;
    bool C2on_pre = false;
    bool C2on_after = false;
    int valC4 = 0;
    bool C4on_pre = false;
    bool C4on_after = false;
    int valC5 = 0; //LSB
    bool C5on_pre = false;
    bool C5on_after = false;
    dig_out = 0;
    double final_freq = 0;
    double time_period = 0.05;
    
    
    //Main loop
    while(1)
    {
        
        
        //Has the update flag been set?       
        if (updateC3) {
            gOled1.clearDisplay();
            //Clear the update flag
            updateC3 = 0;
            
            //Set text cursor
            gOled1.setTextCursor(0,0);
            
            int freqC3 = scountC3 * 50;
            
            if(freqC3 > 60400)
            {
                gOled1.printf("\nC3 (1000Hz) is OFF ", gOled1.width(), gOled1.height());
                C3on_pre = true;
             
            }
            else if(freqC3 < 60000)
            {
                gOled1.printf("\nC3 (1000Hz) is ON  ", gOled1.width(), gOled1.height());    
                C3on_after = true;             
            }
            else
            {
                gOled1.printf("\nC3 (1000Hz) unknown", gOled1.width(), gOled1.height());
            }
            
            
            //Toggle the alive LED
            alive = !alive;
        }
        
        if (updateC2) {
            //Clear the update flag
            updateC2 = 0;
            
            int freqC2 = scountC2 * 50;
            
                if(freqC2 > 52750)
              {
                 gOled1.printf("\nC2 (100Hz) is OFF ", gOled1.width(), gOled1.height());
                 C2on_pre = true;
              }
               else if(freqC2 < 52600)
              {
                gOled1.printf("\nC2 (100Hz) is ON   ", gOled1.width(), gOled1.height());
                C2on_after = true; 
              }
               else
           {
                gOled1.printf("\nC2 (100Hz) unknown", gOled1.width(), gOled1.height());
            }

                      
            //Toggle the alive LED
            alive = !alive;
        }
        
         if (updateC4) {
            //Clear the update flag
            updateC4 = 0;
            
            int freqC4 = scountC4 * 50;
            
                if(freqC4 > 49000)
              {
                 gOled1.printf("\nC4 (10Hz) is OFF   ", gOled1.width(), gOled1.height());
                  C4on_pre = true;
              }
               else if(freqC4 < 48750)
              {
                gOled1.printf("\nC4 (10Hz) is ON     ", gOled1.width(), gOled1.height());
                C4on_after = true; 
              }
               else
           {
                gOled1.printf("\nC4 (10Hz) is unknown", gOled1.width(), gOled1.height());
            }
        
                      
            //Toggle the alive LED
            alive = !alive;
        }
        
         if (updateC5) {
            //Clear the update flag
            updateC5 = 0;
            
            int freqC5 = scountC5 * 50;
            
                if(freqC5 > 56550)
              {
                 gOled1.printf("\nC5 (1Hz) is OFF   ", gOled1.width(), gOled1.height());
                 C5on_pre = true;
              }
               else if(freqC5 < 56320)
              {
                gOled1.printf("\nC5 (1Hz) is ON     ", gOled1.width(), gOled1.height());
                C5on_after = true; 
              }
               else
           {
                gOled1.printf("\nC5 (1Hz) is unknown", gOled1.width(), gOled1.height());
            }
                      
            //Toggle the alive LED
            alive = !alive;
        }
        
        if(C3on_pre && C3on_after)
        {
            C3on_pre = false;
            C3on_after = false;
            valC3 = valC3 + 1;
            if(valC3 == 20){
                valC3 = 0;
            }
        }
        
        if(C4on_pre && C4on_after)
        {
            C4on_pre = false;
            C4on_after = false;
            valC4 = valC4 + 1;
            if(valC4 == 20){
                valC4 = 0;
            }
        }
        
        if(C5on_pre && C5on_after)
        {
            C5on_pre = false;
            C5on_after = false;
            valC5 = valC5 + 1;
            if(valC5 == 20){
                valC5 = 0;
            }
        }
        
        if(C2on_pre && C2on_after)
        {
            C2on_pre = false;
            C2on_after = false;
            valC2 = valC2 + 1;
            if(valC2 == 20){
                valC2 = 0;
            }
        }
               

        int val_C3out = valC3/2;
        int val_C2out = valC2/2;
        int val_C4out = valC4/2;
        int val_C5out = valC5/2;
        
        gOled1.setTextCursor(0,32);
        
        gOled1.printf("\nOutput f = %01u  ",val_C3out);
        gOled1.printf("%01u  ",val_C2out);
        gOled1.printf("%01u  ",val_C4out);
        gOled1.printf("%01u  ",val_C5out);
        
        final_freq = (val_C3out * 1000) + (val_C2out * 100) + (val_C4out * 10) + (val_C5out);
        
        if(final_freq != 0)
        {
            time_period = 1/(2*final_freq);
            wavetimer.detach();
            wavetimer.attach(&wavegenerator, time_period);
        }
        
        gOled1.display();
        
    }
    
}


//Interrupt Service Routine for rising edge on the switch oscillator input
void sedgeC3() {
    //Increment the edge counter
    scounterC3++;    
}

//Interrupt Service Routine for the switch sampling timer
void toutC3() {
    //Read the edge counter into the output register
    scountC3 = scounterC3;
    //Reset the edge counter
    scounterC3 = 0;
    //Trigger a display update in the main loop
    updateC3 = 1;
}

//Interrupt Service Routine for rising edge on the switch oscillator input
void sedgeC2() {
    //Increment the edge counter
    scounterC2++;    
}

//Interrupt Service Routine for the switch sampling timer
void toutC2() {
    //Read the edge counter into the output register
    scountC2 = scounterC2;
    //Reset the edge counter
    scounterC2 = 0;
    //Trigger a display update in the main loop
    updateC2 = 1;
}

//Interrupt Service Routine for rising edge on the switch oscillator input
void sedgeC4() {
    //Increment the edge counter
    scounterC4++;    
}

//Interrupt Service Routine for the switch sampling timer
void toutC4() {
    //Read the edge counter into the output register
    scountC4 = scounterC4;
    //Reset the edge counter
    scounterC4 = 0;
    //Trigger a display update in the main loop
    updateC4 = 1;
}

//Interrupt Service Routine for rising edge on the switch oscillator input
void sedgeC5() {
    //Increment the edge counter
    scounterC5++;    
}

//Interrupt Service Routine for the switch sampling timer
void toutC5() {
    //Read the edge counter into the output register
    scountC5 = scounterC5;
    //Reset the edge counter
    scounterC5 = 0;
    //Trigger a display update in the main loop
    updateC5 = 1;
}

void wavegenerator() {
      
    dig_out = !dig_out;
}


