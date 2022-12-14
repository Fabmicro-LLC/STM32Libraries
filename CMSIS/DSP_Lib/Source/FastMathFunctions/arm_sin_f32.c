/* ----------------------------------------------------------------------    
* Copyright (C) 2010 ARM Limited. All rights reserved.    
*    
* $Date:        15. February 2012  
* $Revision: 	V1.1.0  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_sin_f32.c    
*    
* Description:	Fast sine calculation for floating-point values.   
*    
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*  
* Version 1.1.0 2012/02/15 
*    Updated with more optimizations, bug fixes and minor API changes.  
*   
* Version 1.0.10 2011/7/15  
*    Big Endian support added and Merged M0 and M3/M4 Source code.   
*    
* Version 1.0.3 2010/11/29   
*    Re-organized the CMSIS folders and updated documentation.    
*     
* Version 1.0.2 2010/11/11    
*    Documentation updated.     
*    
* Version 1.0.1 2010/10/05     
*    Production release and review comments incorporated.    
*    
* Version 1.0.0 2010/09/20     
*    Production release and review comments incorporated.    
* -------------------------------------------------------------------- */

#include "arm_math.h"

/**    
 * @ingroup groupFastMath    
 */

/**    
 * @defgroup sin Sine    
 *    
 * Computes the trigonometric sine function using a combination of table lookup   
 * and cubic interpolation.  There are separate functions for   
 * Q15, Q31, and floating-point data types.   
 * The input to the floating-point version is in radians while the   
 * fixed-point Q15 and Q31 have a scaled input with the range   
 * [0 +0.9999] mapping to [0 2*pi), Where range excludes 2*pi.   
 *   
 * The implementation is based on table lookup using 256 values together with cubic interpolation.   
 * The steps used are:   
 *  -# Calculation of the nearest integer table index   
 *  -# Fetch the four table values a, b, c, and d     
 *  -# Compute the fractional portion (fract) of the table index.   
 *  -# Calculation of wa, wb, wc, wd    
 *  -# The final result equals <code>a*wa + b*wb + c*wc + d*wd</code>   
 *   
 * where   
 * <pre>    
 *    a=Table[index-1];    
 *    b=Table[index+0];    
 *    c=Table[index+1];    
 *    d=Table[index+2];    
 * </pre>   
 * and   
 * <pre>    
 *    wa=-(1/6)*fract.^3 + (1/2)*fract.^2 - (1/3)*fract;    
 *    wb=(1/2)*fract.^3 - fract.^2 - (1/2)*fract + 1;    
 *    wc=-(1/2)*fract.^3+(1/2)*fract.^2+fract;    
 *    wd=(1/6)*fract.^3 - (1/6)*fract;    
 * </pre>    
 */

/**    
 * @addtogroup sin    
 * @{    
 */


/**   
 * \par    
 * Example code for Generation of Floating-point Sin Table:   
 * tableSize = 256;    
 * <pre>for(n = -1; n < (tableSize + 1); n++)    
 * {    
 *	sinTable[n+1]=sin(2*pi*n/tableSize);    
 * }</pre>    
 * \par    
 * where pi value is  3.14159265358979    
 */

static const float32_t sinTable[259] = {
  -0.024541229009628296f, 0.000000000000000000f, 0.024541229009628296f,
  0.049067676067352295f, 0.073564566671848297f, 0.098017141222953796f,
  0.122410677373409270f, 0.146730467677116390f,
  0.170961886644363400f, 0.195090323686599730f, 0.219101235270500180f,
  0.242980182170867920f, 0.266712754964828490f, 0.290284663438797000f,
  0.313681751489639280f, 0.336889863014221190f,
  0.359895050525665280f, 0.382683426141738890f, 0.405241310596466060f,
  0.427555084228515630f, 0.449611335992813110f, 0.471396744251251220f,
  0.492898195981979370f, 0.514102756977081300f,
  0.534997642040252690f, 0.555570244789123540f, 0.575808167457580570f,
  0.595699310302734380f, 0.615231573581695560f, 0.634393274784088130f,
  0.653172850608825680f, 0.671558976173400880f,
  0.689540565013885500f, 0.707106769084930420f, 0.724247097969055180f,
  0.740951120853424070f, 0.757208824157714840f, 0.773010432720184330f,
  0.788346409797668460f, 0.803207516670227050f,
  0.817584812641143800f, 0.831469595432281490f, 0.844853579998016360f,
  0.857728600502014160f, 0.870086967945098880f, 0.881921291351318360f,
  0.893224298954010010f, 0.903989315032958980f,
  0.914209783077239990f, 0.923879504203796390f, 0.932992815971374510f,
  0.941544055938720700f, 0.949528157711029050f, 0.956940352916717530f,
  0.963776051998138430f, 0.970031261444091800f,
  0.975702106952667240f, 0.980785250663757320f, 0.985277652740478520f,
  0.989176511764526370f, 0.992479562759399410f, 0.995184719562530520f,
  0.997290432453155520f, 0.998795449733734130f,
  0.999698817729949950f, 1.000000000000000000f, 0.999698817729949950f,
  0.998795449733734130f, 0.997290432453155520f, 0.995184719562530520f,
  0.992479562759399410f, 0.989176511764526370f,
  0.985277652740478520f, 0.980785250663757320f, 0.975702106952667240f,
  0.970031261444091800f, 0.963776051998138430f, 0.956940352916717530f,
  0.949528157711029050f, 0.941544055938720700f,
  0.932992815971374510f, 0.923879504203796390f, 0.914209783077239990f,
  0.903989315032958980f, 0.893224298954010010f, 0.881921291351318360f,
  0.870086967945098880f, 0.857728600502014160f,
  0.844853579998016360f, 0.831469595432281490f, 0.817584812641143800f,
  0.803207516670227050f, 0.788346409797668460f, 0.773010432720184330f,
  0.757208824157714840f, 0.740951120853424070f,
  0.724247097969055180f, 0.707106769084930420f, 0.689540565013885500f,
  0.671558976173400880f, 0.653172850608825680f, 0.634393274784088130f,
  0.615231573581695560f, 0.595699310302734380f,
  0.575808167457580570f, 0.555570244789123540f, 0.534997642040252690f,
  0.514102756977081300f, 0.492898195981979370f, 0.471396744251251220f,
  0.449611335992813110f, 0.427555084228515630f,
  0.405241310596466060f, 0.382683426141738890f, 0.359895050525665280f,
  0.336889863014221190f, 0.313681751489639280f, 0.290284663438797000f,
  0.266712754964828490f, 0.242980182170867920f,
  0.219101235270500180f, 0.195090323686599730f, 0.170961886644363400f,
  0.146730467677116390f, 0.122410677373409270f, 0.098017141222953796f,
  0.073564566671848297f, 0.049067676067352295f,
  0.024541229009628296f, 0.000000000000000122f, -0.024541229009628296f,
  -0.049067676067352295f, -0.073564566671848297f, -0.098017141222953796f,
  -0.122410677373409270f, -0.146730467677116390f,
  -0.170961886644363400f, -0.195090323686599730f, -0.219101235270500180f,
  -0.242980182170867920f, -0.266712754964828490f, -0.290284663438797000f,
  -0.313681751489639280f, -0.336889863014221190f,
  -0.359895050525665280f, -0.382683426141738890f, -0.405241310596466060f,
  -0.427555084228515630f, -0.449611335992813110f, -0.471396744251251220f,
  -0.492898195981979370f, -0.514102756977081300f,
  -0.534997642040252690f, -0.555570244789123540f, -0.575808167457580570f,
  -0.595699310302734380f, -0.615231573581695560f, -0.634393274784088130f,
  -0.653172850608825680f, -0.671558976173400880f,
  -0.689540565013885500f, -0.707106769084930420f, -0.724247097969055180f,
  -0.740951120853424070f, -0.757208824157714840f, -0.773010432720184330f,
  -0.788346409797668460f, -0.803207516670227050f,
  -0.817584812641143800f, -0.831469595432281490f, -0.844853579998016360f,
  -0.857728600502014160f, -0.870086967945098880f, -0.881921291351318360f,
  -0.893224298954010010f, -0.903989315032958980f,
  -0.914209783077239990f, -0.923879504203796390f, -0.932992815971374510f,
  -0.941544055938720700f, -0.949528157711029050f, -0.956940352916717530f,
  -0.963776051998138430f, -0.970031261444091800f,
  -0.975702106952667240f, -0.980785250663757320f, -0.985277652740478520f,
  -0.989176511764526370f, -0.992479562759399410f, -0.995184719562530520f,
  -0.997290432453155520f, -0.998795449733734130f,
  -0.999698817729949950f, -1.000000000000000000f, -0.999698817729949950f,
  -0.998795449733734130f, -0.997290432453155520f, -0.995184719562530520f,
  -0.992479562759399410f, -0.989176511764526370f,
  -0.985277652740478520f, -0.980785250663757320f, -0.975702106952667240f,
  -0.970031261444091800f, -0.963776051998138430f, -0.956940352916717530f,
  -0.949528157711029050f, -0.941544055938720700f,
  -0.932992815971374510f, -0.923879504203796390f, -0.914209783077239990f,
  -0.903989315032958980f, -0.893224298954010010f, -0.881921291351318360f,
  -0.870086967945098880f, -0.857728600502014160f,
  -0.844853579998016360f, -0.831469595432281490f, -0.817584812641143800f,
  -0.803207516670227050f, -0.788346409797668460f, -0.773010432720184330f,
  -0.757208824157714840f, -0.740951120853424070f,
  -0.724247097969055180f, -0.707106769084930420f, -0.689540565013885500f,
  -0.671558976173400880f, -0.653172850608825680f, -0.634393274784088130f,
  -0.615231573581695560f, -0.595699310302734380f,
  -0.575808167457580570f, -0.555570244789123540f, -0.534997642040252690f,
  -0.514102756977081300f, -0.492898195981979370f, -0.471396744251251220f,
  -0.449611335992813110f, -0.427555084228515630f,
  -0.405241310596466060f, -0.382683426141738890f, -0.359895050525665280f,
  -0.336889863014221190f, -0.313681751489639280f, -0.290284663438797000f,
  -0.266712754964828490f, -0.242980182170867920f,
  -0.219101235270500180f, -0.195090323686599730f, -0.170961886644363400f,
  -0.146730467677116390f, -0.122410677373409270f, -0.098017141222953796f,
  -0.073564566671848297f, -0.049067676067352295f,
  -0.024541229009628296f, -0.000000000000000245f, 0.024541229009628296f
};


/**   
 * @brief  Fast approximation to the trigonometric sine function for floating-point data.   
 * @param[in] x input value in radians.   
 * @return  sin(x).   
 */

float32_t arm_sin_f32(
  float32_t x)
{
  float32_t sinVal, fract, in;                   /* Temporary variables for input, output */
  int32_t index;                                 /* Index variable */
  uint32_t tableSize = (uint32_t) TABLE_SIZE;    /* Initialise tablesize */
  float32_t wa, wb, wc, wd;                      /* Cubic interpolation coefficients */
  float32_t a, b, c, d;                          /* Four nearest output values */
  float32_t *tablePtr;                           /* Pointer to table */
  int32_t n;
  float32_t fractsq, fractby2, fractby6, fractby3, fractsqby2;
  float32_t oneminusfractby2;
  float32_t frby2xfrsq, frby6xfrsq;

  /* input x is in radians */
  /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
  in = x * 0.159154943092f;

  /* Calculation of floor value of input */
  n = (int32_t) in;

  /* Make negative values towards -infinity */
  if(x < 0.0f)
  {
    n = n - 1;
  }

  /* Map input value to [0 1] */
  in = in - (float32_t) n;

  /* Calculation of index of the table */
  index = (uint32_t) (tableSize * in);

  /* fractional value calculation */
  fract = ((float32_t) tableSize * in) - (float32_t) index;

  /* Checking min and max index of table */
  if(index < 0)
  {
    index = 0;
  }
  else if(index > 256)
  {
    index = 256;
  }

  /* Initialise table pointer */
  tablePtr = (float32_t *) & sinTable[index];

  /* Read four nearest values of input value from the sin table */
  a = tablePtr[0];
  b = tablePtr[1];
  c = tablePtr[2];
  d = tablePtr[3];


  /* Cubic interpolation process */
  fractsq = fract * fract;
  fractby2 = fract * 0.5f;
  fractby6 = fract * 0.166666667f;
  fractby3 = fract * 0.3333333333333f;
  fractsqby2 = fractsq * 0.5f;
  frby2xfrsq = (fractby2) * fractsq;
  frby6xfrsq = (fractby6) * fractsq;
  oneminusfractby2 = 1.0f - fractby2;
  wb = fractsqby2 - fractby3;
  wc = (fractsqby2 + fract);
  wa = wb - frby6xfrsq;
  wb = frby2xfrsq - fractsq;
  sinVal = wa * a;
  wc = wc - frby2xfrsq;
  wd = (frby6xfrsq) - fractby6;
  wb = wb + oneminusfractby2;

  /* Calculate sin value */
  sinVal = (sinVal + (b * wb)) + ((c * wc) + (d * wd));

  /* Return the output value */
  return (sinVal);

}

/**    
 * @} end of sin group    
 */
