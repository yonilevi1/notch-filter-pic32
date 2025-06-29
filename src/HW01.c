// ----- הגדרות כלליות -----
#define FS 1000.0
#define BUFFER_SIZE 3

// ========== הגדרות למסך TFT ==========
char TFT_DataPort at LATE;
sbit TFT_RST  at LATD7_bit;
sbit TFT_BLED at LATD2_bit;
sbit TFT_RS   at LATD9_bit;
sbit TFT_CS   at LATD10_bit;
sbit TFT_RD   at LATD5_bit;
sbit TFT_WR   at LATD4_bit;

char TFT_DataPort_Direction at TRISE;
sbit TFT_RST_Direction  at TRISD7_bit;
sbit TFT_BLED_Direction at TRISD2_bit;
sbit TFT_RS_Direction   at TRISD9_bit;
sbit TFT_CS_Direction   at TRISD10_bit;
sbit TFT_RD_Direction   at TRISD5_bit;
sbit TFT_WR_Direction   at TRISD4_bit;

// ----- מקדמים לפילטר Notch -----
const float notch_a0 = 0.9710;
const float notch_a1 = -1.5716;
const float notch_a2 = 0.9710;
const float notch_b1 = -1.5716;
const float notch_b2 = 0.9420;

// ----- משתנים גלובליים -----
float xBuf[BUFFER_SIZE] = {0};
float yBuf[BUFFER_SIZE] = {0};
int index = 0;

float filteredValue = 0;
unsigned int adcRaw = 0;
volatile int newSampleReady = 0;

unsigned int xPos = 1;
int prevYRaw = 120;
int prevYFilt = 120;

// ----- אתחול TFT -----
void PrepareTFT(){
    TFT_BLED = 1;
    TFT_Init_ILI9341_8bit(320, 240);
    TFT_Fill_Screen(CL_YELLOW);
    TFT_Set_Pen(CL_BLUE, 1);
}

// ----- אתחול ADC -----
void PrepareADCChannel0(){
    AD1PCFG = 0xFFFE;
    JTAGEN_bit = 0;
    TRISB0_bit = 1;
    ADC1_Init();
    Delay_ms(100);
}

// ----- אתחול טיימר 2/3 לפסיקה כל 1ms -----
void InitTimer2_3(){
    T2CON = 0x8008;
    T3CON = 0x0;
    TMR2 = 0;
    TMR3 = 0;
    T3IP0_bit = 1;
    T3IP1_bit = 1;
    T3IP2_bit = 1;
    T3IF_bit = 0;
    T3IE_bit = 1;

    PR2 = 13568;
    PR3 = 12;
}

// ----- פסיקת טיימר: דגימה + פילטר -----
void Timer2_3Interrupt() iv IVT_TIMER_3 ilevel 7 ics ICS_SRS {
    T3IF_bit = 0;

    float x, y;

    adcRaw = ADC1BUF0;
    x = (float)adcRaw;

    y = notch_a0 * x +
        notch_a1 * xBuf[(index + BUFFER_SIZE - 1) % BUFFER_SIZE] +
        notch_a2 * xBuf[(index + BUFFER_SIZE - 2) % BUFFER_SIZE] -
        notch_b1 * yBuf[(index + BUFFER_SIZE - 1) % BUFFER_SIZE] -
        notch_b2 * yBuf[(index + BUFFER_SIZE - 2) % BUFFER_SIZE];

    xBuf[index] = x;
    yBuf[index] = y;
    filteredValue = y;

    index = (index + 1) % BUFFER_SIZE;
    newSampleReady = 1;
}

// ----- MAIN -----
void main()
{
    PrepareTFT();
    PrepareADCChannel0();

    // ? קריאה ראשונית מה־ADC לפני הפעלת הפסיקות
    ADC1_Get_Sample(0);

    InitTimer2_3();
    EnableInterrupts();

    while (1){
        if (newSampleReady) {
            int yRaw, yFilt;

            newSampleReady = 0;

            yRaw = 120 - (adcRaw >> 4);               // raw 12bit ל־8bit
            yFilt = 120 - ((int)filteredValue >> 4);  // filtered

            if (xPos > 0) {
                // קו כחול לאות המקורי
                TFT_Set_Pen(CL_BLUE, 1);
                TFT_Line(xPos - 1, prevYRaw, xPos, yRaw);
                prevYRaw = yRaw;

                // קו אדום לאות המסונן
                TFT_Set_Pen(CL_RED, 1);
                TFT_Line(xPos - 1, prevYFilt, xPos, yFilt);
                prevYFilt = yFilt;
            }

            xPos++;
            if (xPos >= 320) {
                xPos = 1;
                prevYRaw = 120;
                prevYFilt = 120;
                TFT_Fill_Screen(CL_YELLOW);  // אתחול המסך
            }
        }
    }
}
