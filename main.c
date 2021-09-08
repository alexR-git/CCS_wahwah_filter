/* ------------------------------------------------------------------------------------------
				UV GIET
				PROYECTO TDS: FILTRO WAH WAH
 	 	 	 	ALEJANDRO RODR�GUEZ
------------------------------------------------------------------------------------------ */

// INCLUDES

#include "stdio.h"
#include "usbstk5505.h"
//#include "usbstk5515.h"
#include "aic3204.h"
#include "PLL.h"
#include "stereo.h"
#include "math.h"
//#include "oled.h"
//#include "usbstk5515_led.h"
#include "pushbuttons.h"
//#include "bargraph.h"


// VARIABLES DE ENTRADA Y SALIDA DE LA SE�AL

Int16 left_input;
Int16 right_input;
Int16 left_output;
Int16 right_output;
Int16 mono_input;
unsigned int Step = 0;	// Variable para la botonera

// DEFINES

#define SAMPLES_PER_SECOND 48000
#define GAIN_dB 0
#define PI 3.1415926

// DECLARACI�N Y ASIGNACI�N DE VARIABLES

double Fs = 48000;
double minf = 500;
double maxf = 3000;
double Fw = 2000;                 		// Frec.
double damp = 0.05;               		// Factor de amortiguaci�n
double Fc;								// FRECUENCIA CENTRAL DE LA BANDA PASANTE: Fc = minf:delta:maxf;
double F1;								// PAR�METRO F1
double delta;
double Q1;

// PREPARACI�N DE LA ENTRADA AL BUCLE: MUESTRA ANTERIOR = 0

double ybn_1 = 0;
double yln_1 = 0;

double yhn;        			// Primera variable auxiliar
double ybn;               	// Segunda variable auxiliar. Salida del sistema
double yln;               	// Tercera variable auxiliar


/* ------------------------------ MAIN ------------------------------*/

int main(void) {

	// INICIALIZACI�N DE LA PLACA Y DEL CODEC

	/* Initialize BSL */
	USBSTK5505_init( );

	/* Initialize PLL */
	pll_frequency_setup(100);

	/* Initialise hardware interface and I2C for code */
	aic3204_hardware_init();

	/* Initialise the AIC3204 codec */
	aic3204_init();

	// INFORMACI�N DE LA INICIALIZACI�N A LA CONSOLA

	printf("\n\nRunning Getting Started Project\n");

	// [OPCIONAL] FIJACI�N DE LA FRECUENCIA DE MUESTREO Y LA GANANCIA

	set_sampling_frequency_and_gain(SAMPLES_PER_SECOND, GAIN_dB);		// 0 para no amplificar la se�al filtrada

	// APAGADO DEL LED DE LA PLACA

	asm(" bclr XF");

	// VARIACI�N EN HZ DE LA FRECUENCIA CENTRAL POR MUESTRA

	delta = Fw/Fs;

	// DEFINICI�N DE LA FC ANTES DE ENTRAR POR PRIMERA VEZ AL BUCLE

	Fc = minf;

	// ANCHURA DE LA BANDA PASANTE

	Q1 = 2*damp;


	// BUCLE PRINCIPAL

	while (1){

		// AJUSTE DE DELTA A LOS L�MITES EXACTOS ANTES DE ENTRAR AL ALGORITMO PRINCIPAL

		if (Fc > maxf){

			Fc = maxf;
			delta = -delta;

		}

		if (Fc < minf){

			Fc = minf;
			delta = -delta;

	    }

		// LECTURA DE LA SE�AL DE ENTRADA

		aic3204_codec_read(&left_input, &right_input);

		mono_input = stereo_to_mono(left_input, right_input);

		// DEFINICI�N Y ASIGNACI�N DEL FACTOR F1

	    F1 = 2*sin((PI*Fc)/Fs);

	    // ALGORITMO DE FUNCIONAMIENTO DEL FILTRO

	    yhn = mono_input - yln_1 - Q1*ybn_1;        // Primera variable auxiliar
	    ybn = F1*yhn + ybn_1;               		// Segunda variable auxiliar. Salida del sistema
	    yln = F1*ybn + yln_1;               		// Tercera variable auxiliar

	    // ACTUALIZACI�N DE LAS MUESTRAS

	    yln_1 = yln;
	    ybn_1 = ybn;

	    // ACTUALIZACI�N DE Fc

	    if (Fc > maxf){

	    	delta = -delta;

	    } else if (Fc < minf){

	    	delta = -delta;

	    } else {

	    	Fc = Fc + delta;

	    }

	    //Fc = Fc + delta;

	    // ASGINACI�N DE LA BOTONERA Y SALIDA DE LA SE�AL FILTRADA

	    Step = pushbuttons_read(4);

	    if (Step == 1){

	    	left_output = mono_input;
	    	right_output = mono_input;
	    }

	    if (Step == 2){

	    	left_output = ybn;
	    	right_output = ybn;
	    }

	    aic3204_codec_write(left_output, right_output);


	    // IMPLEMENTACI�N PARA ESCUCHAR POR AURICULARES: CANAL IZQUIERDO, SE�AL ORIGINAL; DERECHO: FILTRADA

	    //left_output = mono_input;							// Se�al sin filtrar por el canal izquierdo
	    //right_output = ybn;								// Se�al filtrada por el canal derecho

	    //aic3204_codec_write(left_output, right_output);	// Escritura en el buffer de salida

	}

	return 0;
}
