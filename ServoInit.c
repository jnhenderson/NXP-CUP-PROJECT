#include <MKL25Z4.H>


void ServoInit (){
 

	
SIM->SCGC5 |= (0x1<<10); // enable clock to Port B
PORTB->PCR[0] = (0x3<<8); // PTB0 used by TPM1, TPM1 value
SIM->SCGC6 |= (0x1<<25); // enable clock to TPM1, sets D24 to 0 selecting TPM0 as clock enabled change to tpm1
SIM->SOPT2 |= (0x1<<24); // use MCGFLLCLK as timer counter clock, sets D25 and D24 or TPM SRC Reg to 01 = MCGFLLCLK tpm1
TPM1->SC = 0; // disable timer while configuring
TPM1->CONTROLS[0].CnSC = 0x20 | 0x08; // edge-aligned, pulse high 0x20 = 0010 0000 0x08 = 1000
TPM1->MOD = 26213; // Set up modulo register TPM1_MOD + 1 = timefreq/PWM freq,
TPM1->CONTROLS[0].CnV =4100; // Default Duty cycle, roughly in the middle ready for calibration
TPM1->SC = 0x0B; // enable TPM1 with prescaler /8

}

