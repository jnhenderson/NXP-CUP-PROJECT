#include <MKL25Z4.H>
#include "motor.h"

void Motor_Init(void) {
SIM->SCGC5 |= (0x1<<11); /* enable clock to Port C */
SIM->SCGC5 |= (0x1<<13); /* enable clock to port E */
SIM->SOPT2 |= (0x1<<24); /* use MCGFLLCLK/2 as timer counter clock */
SIM->SCGC6 |= (0x1<<24); /* enable clock to TPM0 */
SIM->SCGC6 |= (0x1<<25); /* enable clock to TPM1 */

TPM0->SC = 0; /* disable timer */
TPM0->CONF = 0;
TPM1->SC = 0; /* disable timer */
TPM1->CONF = 0;

TPM0->SC = 0x02; /* enable TPM0 with prescaler /8 */
TPM0->MOD = 10486; /* Set up modulo register */
TPM1->SC = 0x0B; /* enable TPM1 with prescaler /8 */
TPM1->MOD = 10486; /* Set up modulo register */

TPM0->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK; 
TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK; /* invert the second PWM signal */
TPM0->CONTROLS[2].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
TPM0->CONTROLS[3].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK; /* invert the second PWM signal */
TPM1->CONTROLS[1].CnSC = 0x20 | 0x08; /*centre alligned pulse high for TMP1*/
	
TPM0->SC |= TPM_SC_CMOD(1);
TPM1->SC |= TPM_SC_CMOD(1);
	
PORTC->PCR[1] = PORT_PCR_MUX(4); /* Enable FTM for PTC1 */
PORTC->PCR[2] = PORT_PCR_MUX(4); /* Enable FTM for PTC2  */
PORTC->PCR[3] = PORT_PCR_MUX(4); /* Enable FTM for PTC3  */
PORTC->PCR[4] = PORT_PCR_MUX(4); /* Enable FTM for PTC4  */

PORTE->PCR[21] = (0x3<<8);/* configure PTE21 for PWM */

 }

void H_Bridge_Disable(void) {
	
TPM1->CONTROLS[1].CnV = 0; /* Sets the duty cycle for the H-Bridge to 0 */

 }

void Motor_ON(void) {
	
	float speed_max = 2500;
float speed_min = 0;
float speed_motorA = 900;
float speed_motorB = 900;
float speed_HBridge = 900;

		 if(speed_motorA > speed_max)
		 speed_motorA = speed_max;
	 else if(speed_motorA < speed_min)
		 speed_motorA = speed_min;
	 
	 		 if(speed_motorB > speed_max)
		 speed_motorB = speed_max;
	 else if(speed_motorB < speed_min)
		 speed_motorB = speed_min;
	 
	 		 if(speed_motorA >= speed_motorB)
		 speed_HBridge = speed_motorA;
	 else speed_HBridge = speed_motorB;
	 
TPM0->CONTROLS[2].CnV = speed_motorA; /* Set up MotorA value for x% dutycycle */
TPM0->CONTROLS[3].CnV = speed_motorA; /* Set up MotorA value for x% dutycycle */
TPM0->CONTROLS[0].CnV = speed_motorB; /* Set up MotorB value for x% dutycycle */
TPM0->CONTROLS[1].CnV = speed_motorB; /* Set up MotorB value for x% dutycycle */
TPM1->CONTROLS[1].CnV = speed_HBridge; /* Set up H-Bridge value for x% dutycycle */

}
