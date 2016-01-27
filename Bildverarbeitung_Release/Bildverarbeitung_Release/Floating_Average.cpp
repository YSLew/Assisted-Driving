/** File for pseudo floating average functions (.cpp)
*
* Contains function to generate, manipulate and read a buffer for pseudo floating averages.
* Attention: all functions will call it a floating average, because of the underlain basics.
* CheckAVG will NOT get the Average value, but will get "1" if the most values are true.
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//HEADER
//////////////////////////////////////////////////////////////////////////////////////

#include "Floating_Average.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

/** Add value to AVG buffer
*
* @param Point  io_pFloatAvgFilter - pointer on avg structure, i_NewValue - value to write
∗ @return void
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void AddToFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter,
	bool i_NewValue)
{
	// Write new value
	io_pFloatAvgFilter -> Data[io_pFloatAvgFilter -> IndexNextValue] =
		i_NewValue;
	// next value will be written to position behind the new value
	io_pFloatAvgFilter -> IndexNextValue++;
	//if end of buffer is reached, start from beginning
	io_pFloatAvgFilter -> IndexNextValue %= SIZE_OF_AVG;
}

/** Inits AVG buffer
*
* @param Point  io_pFloatAvgFilter - pointer on avg structure, i_DefaultValue - default value to write
∗ @return void
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void InitFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter,
	bool i_DefaultValue)
{
	//fill buffer
	for (char i = 0; i < SIZE_OF_AVG; ++i)
	{
		io_pFloatAvgFilter -> Data[i] = i_DefaultValue;
	}
	// next value will be written to start of buffer.
	io_pFloatAvgFilter -> IndexNextValue = 0;
}

/** Checks AVG buffer
*
* If most entries in buffer are true, this function will return true. Otherwise false.
*
* @param Point  io_pFloatAvgFilter - pointer on avg structure
∗ @return bool - most values true?
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
bool CheckAVG(tFloatAvgFilter * io_pFloatAvgFilter)
{
	int trues = 0;
	double o_Result = true;
	for (int i = 0; i < SIZE_OF_AVG; ++i)
	{
		if ((io_pFloatAvgFilter -> Data[i]) == true)
			trues++;
	}
	if (trues >= (SIZE_OF_AVG * MIN_FOR_DETECT))
		return true;
	else
		return false;
}

/** Get value from AVG buffer (only printf)
*
* @param Point  io_pFloatAvgFilter - pointer on avg structure
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void GetValue(tFloatAvgFilter * io_pFloatAvgFilter)
{
	for (int i = 0; i < SIZE_OF_AVG; ++i)
	{
		printf("%d", io_pFloatAvgFilter -> Data[i]);
	}
}