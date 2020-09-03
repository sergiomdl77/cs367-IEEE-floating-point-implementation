/***********************************************
 *  Sergio Delgado
 *  G#: 00512529
 *  CS367-001
 *  Spring, 2019
 *
 *  Project 1
***********************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fp.h"

#define POS_INFINITY 3840   // integer value that stores 0 1111 0000 0000
#define BIAS 7     //  BIAS = e^(4-1)- 1
#define DENOMINATOR 256  // 8 bits (frac denominator is always 256)
#define INFINITY_EXP 15	// value of field exp that will indicate Infinity

/*********************************************************************
 * IMPORTANT NOTE FOR CODE STYLE REVIEW:
 * 
 * This code looks best (from vim) when using "set tabstop=4"
*********************************************************************/



// Function that receives an integer variable as a parameter by reference and 
// inserts the binary representation also received as a parameter (leftshift) in
// starting bit position stated by leftShift. 
void insertBinRep(int b, int *encodedFP, int leftShift)
{
	b = b << leftShift;  //  Positioning data on the 4-byte int in righ bit position
	*(encodedFP) = *(encodedFP) | b;   // inserting the bits
	return;
}



// Function that receives the M and exp components of the 13-bit floating point
// encoded representation.  It then inserts such components in an integer variable
// that will be returned by the function as the encoded floating point.
int normalized(float M, int exp)
{
	int encodedFP = 0;
	float frac = (M-1) * DENOMINATOR;  // extracting the numerator of frac from M
	int binFrac = (int)frac;			// Obtaining the floor of the numerator
										// as in rounding down by truncating.
	insertBinRep(binFrac, &encodedFP, 0);	// creating the encodedFP (adding frac)

	insertBinRep(exp, &encodedFP, 8);		//                    and (adding exp)

	return encodedFP;
}	


// Function that receives the a value of M as a parameter and returns the adjusted
// value of M that will will be appropriate  to a denormalized floating point 
// representation.
float correctDenormaM(float M)
{
	for(int i=0; i > 1-BIAS; i--)
		M = M * 2;
	return M;
}


// Function that receives a floating point value as a parameter and returns an 
// integer value with the  encoded representation.  
int denormalized(float val)
{
	int encodedFP = 0;
	float M = correctDenormaM(val);
	float frac = M * DENOMINATOR;		// extracting the numerator of frac from M
	int binFrac = (int)frac;			// obtaining the floor of the numerator
										// as when rounding down by truncating
	insertBinRep(binFrac, &encodedFP, 0);	// creating the encodedFP (adding frac)

	return encodedFP;
}


// Function that receives a floating point as a parameter and encodes it into a 
// 13-bit floating point representation, which will be saved in an integer variable
// which will be returned by the function.
int computeFP(float val)
{
	int encodedFP = 0;
	int E = 0;
	float M = val;	
	int exp = 0;

	while (M < 1)   // Adjusting M from a number less than 1 to greater than 1
	{
		E--;
		M = M * 2;
	} 	
	
	while (M >= 2)  // Adjusting M from a number greater than 2 to less than 2
	{
		E++;
		M = M / 2;
	}

	exp = E + BIAS;	// calculating exp

	if (exp >= INFINITY_EXP)		// if exp overflows
		return POS_INFINITY; 		// return positive infinity 

	else if (exp <= 0)				// if exp is <= 0 then 
		return denormalized(val);	// encode as denormalized

	else							// else
		return normalized(M, exp);	// encode as normalized

}


// Function that receives the integer value, which stores the encoded floating point
// of a number, and extracts the fraction (frac) field from it.
int getFrac(int encodedFP)
{
	int decValue = 0xff;
	decValue = decValue & encodedFP;

	return decValue;
}


// Function that receives the integer value, with the encoded floating point of a 
// number, and extracts the exponent (exp) field from it.
int getExp(int encodedFP)
{
	int decValue = encodedFP >> 8;

	return decValue;
}


// Function that, using the defined representation, computes the floating point
// value.  For 0, simply returns 0. For Infinity, returns -1. The only parameter
// is the integer value that holds the encoded representation of a floating point.
float getFP(int val)
{
	float M;
	int E;
	int numerator = getFrac(val);
	int exp = getExp(val);
	
	if (val == POS_INFINITY)//    if the value encoded was positive infinity
	{
		return -1.0;
	}

	else if (exp == 0)		// If value was encoded as a denormalized number
	{
		M = (float)numerator/DENOMINATOR;
		E = 1-BIAS;
	}

	else					// If value enconded was normalized
	{
		M = (float)numerator;
		M = (M/DENOMINATOR) + 1;
		E = exp - BIAS;
	}

				
	if(E > 0)	// calculates the value represented by the encoded FP with positive E
	{
		for (int i=0; i<E; i++)
			M = M * 2;
	}
	else		// calculates the value represented by the encoded FP with negative E
	{
		for (int i=0; i>E; i--)
			M = M / 2;
	}	

	return M;
}


// Function that extracts the E and M components from the encoded floating point.
// It receives the parameters E and M by reference, while parameter (val) is just
// received by value.
void getEandM (int val, int* E, float *M)
{

	int numerator = getFrac(val);
	int exp = getExp(val);
	
	if (exp == 0)		// If value was encoded as a denormalized number
	{
		*M = (float)numerator/DENOMINATOR;
		*E = 1-BIAS;
	}

	else					// If value enconded was normalized
	{
		*M = (float)numerator;
		*M = (*M/DENOMINATOR) + 1;
		*E = exp - BIAS;
	}
	return;
}


// Function that brings the value of M within the range [1,2) while
// adjusting the value of E.  It recieves values by reference as parameters.
void adjustM (float* M, int* E)
{
	while (*M < 1)   // Adjusting M from a number less than 1 to greater than 1
	{
		*E = *E - 1;
		*M = *M * 2;
	} 	
	
	while (*M >= 2)  // Adjusting M from a number greater than 2 to less than 2
	{
		*E = *E + 1;
		*M = *M / 2;
	}
	return;
}


// Function that multiplies two encoded floating point values.
// It implements this for positive source values
// If the sum results in overflow, return Infinity
// If the sum is 0, return 0
int multVals(int source1, int source2)
{
	if (source1 == POS_INFINITY 
		|| source2 == POS_INFINITY)			// if either value is infinity, 
		return POS_INFINITY;				// then result of multiplication is infinity

	int exp;
	int denormE, E1, E2, E;
	float denormM, M1, M2, M;

	getEandM(source1, &E1, &M1);	// extracting M and E from operand 1
	getEandM(source2, &E2, &M2);	// extracting M and E from operand 2

	E = E1 + E2;					// precessing multiplication of encoded FP values
	M = M1 * M2;					//

	denormM = M;				// saving original values of resulting M and E
	denormE = E;				// in case they need to be encoded as denormalized
	
	adjustM(&M, &E);			// adjusting M to be within [1,2).

	exp = E + BIAS;

	if (exp >= INFINITY_EXP)		// If exp is overflows after adjusting M
		return POS_INFINITY;		// return positive infinity 

	else if (exp <= 0)				// If exp underflows, encode as denormalized
	{
		for (int i=denormE; i<(1-BIAS); i++)// adjust M (denormM)to be encoded 
			denormM = denormM / 2;			// as a denormalized floating point.

		int encodedFP = 0;
		float frac = denormM * DENOMINATOR;	// extracting numerator of frac from M
		int binFrac = (int)frac;			// obtaining the floor of the numerator
											// as when rounding down by truncating

		insertBinRep(binFrac, &encodedFP, 0);// creating the encodedFP (adding frac)

		return encodedFP;
	}

	else
		return normalized(M, exp);	
}


// Function that adds two encoded floating point values.
// It implements this for positive source values
// If the sum results in overflow, return Infinity
// If the sum is 0, return 0
int addVals(int source1, int source2) 
{
	if (source1 == POS_INFINITY 
		|| source2 == POS_INFINITY)	// if either value is infinity, 
		return POS_INFINITY;		// then result of multiplication is infinity

	int exp;
	int denormE, E1, E2, E;
	float denormM, M1, M2, M;

	getEandM(source1, &E1, &M1);	// getting E and M components from operand 1
	getEandM(source2, &E2, &M2);	// getting E and M components from operand 2

	if (E1 > E2)						// Adjusting E2 to be same as E1
		while (E2 < E1)
		{
			M2 = M2/2;
			E2++;
		}
	else if (E2 > E1)					// Adjusting E1 to be same as E2
		while (E1 < E2)
		{
			M1 = M1/2;
			E1++;
		}
	
	E = E1;								// processing addition of encoded FP values
	M = M1 + M2;						//

	denormM = M;					// saving original values of resulting M and E
	denormE = E;					// in case of encoding them as denormalized FP
	
	adjustM(&M, &E);				// adjusting M to be within [1,2)

	exp = E + BIAS;

	if (exp >= INFINITY_EXP)		// If exp is overflows after adjusting M
		return POS_INFINITY;		// return positive infinity

	else if (exp <= 0)				// If exp underflows, encode as denormalized
	{
		for (int i=denormE; i<(1-BIAS); i++)// adjust M (denormM)to be encoded 
			denormM = denormM / 2;			// as a denormalized floating point.

		int encodedFP = 0;
		float frac = denormM * DENOMINATOR;	// extracting numerator of frac from M
		int binFrac = (int)frac;			// obtaining the floor of the numerator
											// as when rounding down by truncating

		insertBinRep(binFrac, &encodedFP, 0);// creating the encodedFP (adding frac)

		return encodedFP;
	}

	else
		return normalized(M, exp);
	
}



/*              ***********************************************
 *              This was used for the testing of my functions.
 *              ***********************************************
int main()
{

	printf("%d\n", computeFP(0.00006));
	printf("%f\n", getFP(addVals(computeFP(5),computeFP(10))));

	printf("%f\n", getFP(computeFP(13.45)));

	printf("%f\n", getFP(computeFP(0.67)));

	
	printf("Addition %f\n", addVals(1623,2734));

	printf("Multiplicati %f\n", multVals(1623,2734));
	
	printf("0.67 = %d\n", computeFP(0.67));
	
	printf("13.45 = %d\n", computeFP(13.45));

	printf("%d\n", computeFP(0.26));
	printf("%d\n", computeFP(15.25));
	printf("%d\n", computeFP(0.009765));
	printf("%d\n", computeFP(0.004882));
	printf("%d\n", computeFP(0.000004882));
	printf("%d\n", computeFP(999999999));
	printf("%d\n", computeFP(255.99999));
	printf("%d\n", computeFP(1));
	printf("%5.7f\n", getFP(1290));
	printf("%5.7f\n", getFP(2792));
	printf("%5.7f\n", getFP(159));
	printf("%5.7f\n", getFP(79));
	printf("%5.7f\n", getFP(0));
	printf("%5.7f\n", getFP(-1));
	printf("%5.7f\n", getFP(3839));
	printf("%5.7f\n", getFP(1792));


	printf("Multi = %5.7f\n", getFP(multVals(3839, 79)));
	printf("Multi = %5.7f\n", getFP(multVals(159, 79)));
	printf("%5.7f\n", getFP());
	printf("Multi = %5.7f\n", getFP(multVals(2792, 159)));

	printf("%5.7f\n", getFP());
	printf("%5.7f\n", getFP());

	printf("Adding = %5.7f\n", getFP(addVals(1290,2792))); 

	printf("Adding = %5.7f\n", getFP(addVals(1792,3839))); 
	printf("Adding = %5.7f\n", getFP(addVals(159,79))); 

	printf("Adding = %5.7f\n", getFP(addVals(3839,1290))); 
	printf("Adding = %5.7f\n", getFP(addVals(-1,79))); 


	printf("Mult = %5.7f\n", getFP(multVals(1623, 2734)));

	printf("Add = %5.7f\n", getFP(addVals(1623, 2734)));

	return 0;


}
*/

