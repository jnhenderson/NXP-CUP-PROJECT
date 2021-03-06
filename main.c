//**************************************************************************/
//						Linescan Camera Control		(LSC Control)												/
// *************************************************************************/


#include "MKL25Z4.h"
#include "stdio.h"
#include "UART0_TxRx.h"
#include "ServoInit.h"
#include "motor.h"

#define LSC_SI_HIGH PTD->PSOR=(0x01<<7);  //Defined calls for common port functions
#define LSC_SI_LOW PTD->PCOR=(0x01<<7);   //Camera SI starts the capture/integration time (HI/LO states)

#define LSC_CLK_HIGH PTE->PSOR=(0x01<<1); //Sets the clock for the camera capture, cycled later in the code to
#define LSC_CLK_LOW PTE->PCOR=(0x01<<1);  //provide timing

#define FRQ_MCGFLLCLK 20971520 						//Reference value for clock/timer division

short int readADC(short ChID);

void delay(int n);
void LSC_Init(void);
void LSC_ReadImage(volatile short int *imgData);

void TPM0_init(short int initMODvalue);
void TPM0_DelayOnce(void);

volatile short int imageData0[128];
volatile short int imageData1[128];
volatile short int imageDataOut[128];

int No;

	  char buf [100];   // UART buffer 
    int n,i, pos;            // number of characters in b2097uf to be sent
	
    int nImgRd; // number of image reading (how many times has the image been red) 
	
		int bit;
		int output[128]; // sets output array
		int z = 2500; // line threshold 
		int arraySize = 127; // equal to size of linescan array - 1
		int s;
		int x;
		int PosRight = 3300;
		int PosLeft = 5000;
		int PosCentre = 4150;
		int LowPos;
		int Pos;

int main (void)
{

	LowPos = imageData0[19];
    
		int leftline = 0;
		int rightline = 0;
	
	short unsigned int initMODValue;
	
	  UART0_init();  // Initialized UART0, 57600 baud
		ServoInit();
		Motor_Init();
		//H_Bridge_Disable();
		Motor_ON();
	
	  LSC_Init();  // configure camera
	  nImgRd=0;
	
    
	// initialize TPM0 for 1 us delay
	  initMODValue=0.5*(float)(FRQ_MCGFLLCLK)/1000000.0; //MOD value for 1us delay
	  TPM0_init(initMODValue);
		
  	sendHelloWorld();
 	while(1){
	  	n=sprintf(buf, "\n \n nImgRd=%d\r\n", nImgRd);
		sendStr(buf, n);	
		  nImgRd++;		
		  LSC_ReadImage(imageData0);
		  
			
		
		
		 //send the image data to PC
			for (i=0;i<128;i++) //delay around 20ms
			{	n = sprintf(buf, "%d ", (imageData0[i]/3500)); // convert integer value into ASCII
			  sendStr(buf, n);
			}
		
		
			
		
		leftline = 0;
		rightline = 0;
		
			for (i=80;i<128;i++)
			{ rightline = rightline + (imageData0[i]/3500);}
			
			for (i=0;i<48;i++)
			{ leftline = leftline + (imageData0[i]/3500);}
			
		if((rightline-leftline > 3) | (leftline-rightline > 3)){
			
			if(rightline > leftline)
				{TPM1->CONTROLS[0].CnV =	PosLeft;}
		
			if((rightline < leftline))
				{TPM1->CONTROLS[0].CnV =	PosRight;}
				
			else
				{TPM1->CONTROLS[0].CnV =	PosRight;}
		
		}	
		else
		{TPM1->CONTROLS[0].CnV =	PosCentre;}
	
}
}
		

				
		

		
#define DIFF_SINGLE 0x00
#define DIFF_DIFFERENTIAL (0x01<<5)

// #define ADC_SC1_ADCH_MASK      0x1Fu   
// #define ADC_SC1_ADCH(x)     (((uint32_t)(x))&ADC_SC1_ADCH_MASK)

void LSC_Init(void){
//Initialise/assign ports for LSC
	
	SIM->SCGC5 |=(0x1<<12 | 0x1<<13); //Enable clock to PORTD and PORTE

	
    PORTD->PCR[7] = 0x100;     // Set PTD7 pin as GPIO 
    PTD->PDDR |= (0x1<<7);     // Assign PTD7 as output pin
	
    PORTE->PCR[1] = 0x100;     // Set PTE1 pin as GPIO 
    PTE->PDDR |= (0x1<<1);     // Assign PTE1 as output pin
	
		PTD->PDDR &= ~(0x1<<5);    // Set PTD5 as input pin
		PORTD->PCR[5] = 0; 				 // Assign PTD5 as analog input PTD5.MUX[10 9 8]=000
	
	
	  SIM->SCGC6 |= 0x08000000;   // enable clock to ADC0 ; 0x8000000u
	
	  // Configure ADC as it will be used, but because ADC_SC1_ADCH is 31,
    // the ADC will be inactive.  Channel 31 is just disable function.
    // There really is no channel 31.
	  // disable AIEN, Signle-ended, channel 31
    ADC0->SC1[0] = DIFF_SINGLE|ADC_SC1_ADCH(31);  
		// TODO: use hardware triger, TPM0
	  ADC0->SC2 &= ~0x40;   // ADTRG=0, software trigger
	
	  // clock div by 4, long sample time, single ended 12 bit, bus clock 
    ADC0->CFG1 =(0x1<<6 | 0x1<<4 |0x1<<2); //0b01010100; 
	  //  ADC0->CFG1 = 0x40 | 0x10 | 0x04 | 0x00;
	
	  // select the B set of ADC input channels for PTD5 (SE6b)
	  ADC0->CFG2 |=(0x1<<4); //CFG2.MUXSEL=1, ADxxb channels are selected; 
	
	  LSC_SI_LOW;
	  LSC_CLK_LOW;

}   
		
void LSC_ReadImage(volatile short int *imgData)
{  
	unsigned int i;
	// SI (PTD7) Digital output, CLK (PTE1) Digitaloutput
	// AO (PTD5) Analgoue input (channel 6)
			TPM0_DelayOnce();
	TPM0_DelayOnce();
	 LSC_SI_HIGH;
	 TPM0_DelayOnce();
	 LSC_CLK_HIGH;
	 TPM0_DelayOnce();
	 LSC_SI_LOW;
	 TPM0_DelayOnce();
	 
	// the following reading sequence is different from the diagram
	// Fig. 11, page 10, AMU's TLS1401CL.pdf. 
	// image sensor is red at the falling edge of CLK
	// howver, Fig. 11 suggests reading image at rising edge
	// read the 128 pixel image data
	 // imgData[0]=(short int)readADC(6); 
	 // LSC_CLK_LOW;
			LSC_CLK_LOW;
		TPM0_DelayOnce();
		TPM0_DelayOnce();
		
	for (i=0;i<128;i++)
	{
		imgData[i]=(short int)readADC(6); 
		LSC_CLK_HIGH;
		TPM0_DelayOnce();
		TPM0_DelayOnce();
		LSC_CLK_LOW;
		TPM0_DelayOnce();
		TPM0_DelayOnce();
	}
	
	// additional one CLK to allow ??
	  LSC_CLK_HIGH;
		TPM0_DelayOnce();
		TPM0_DelayOnce();
		LSC_CLK_LOW;
	  TPM0_DelayOnce();
		TPM0_DelayOnce();
}   
		

		
short int readADC(short ChID) 
{
	short int result;     	
	
	ADC0->SC1[0] = ChID; //software triger conversion on channel 13, SE13
	while(!(ADC0->SC1[0] & 0x80)) { } /* wait for conversion complete */
	result = ADC0->R[0];        /* read conversion result and clear COCO flag */
	return result;
}

// Initialize the TPM0 to generate a specified delay in number of MCGFLLCLK clocks
// By default, the MCGFLLCLK set by system setup is 20.97152MHz
void TPM0_init(short int initMODvalue)
{
	
	SIM->SCGC6 |= (0x01<<24); // 0x01000000;, enale clk to TPM0
	SIM->SOPT2 |=(0x01<<24); // 0x01000000, use MCGFLLCLK as timer counter clk
	TPM0->SC = 0; // diable timer when configuring
	// TPM0->MOD = 0xFFFF; // by default, set the 16-bit modulo value to maximum
	                    // thus results in maximum delay
	TPM0->MOD = initMODvalue;
	TPM0->SC|=0x80; // clear TOF
	// TPM0->SC|=0x08; // enable timer free-rnning mode
}

// Initialize the TPM0 to generate a specified delay in number of MCGFLLCLK clocks
// By default, the MCGFLLCLK set by system setup is 20.97152MHz

void TPM0_DelayOnce(void)
{
	TPM0->SC|=0x80; // clear TOF
	TPM0->SC|=0x08; // enable timer free-rnning mode
	while((TPM0->SC & 0x80) == 0) { } // wait until the TOF is set
	TPM0->SC = 0; // diable timer when configuring
}


void delay(int n) {
 int i;
 for(i = 0; i < n; i++) ;
}

