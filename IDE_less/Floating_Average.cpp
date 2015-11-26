#include "Floating_Average.h"

#include <stdio.h>

void AddToFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter,
	bool i_NewValue)
{
	// Neuen Wert an die dafuer vorgesehene Position im Buffer schreiben.
	io_pFloatAvgFilter->Data[io_pFloatAvgFilter->IndexNextValue] =
		i_NewValue;
	// Der naechste Wert wird dann an die Position dahinter geschrieben.
	io_pFloatAvgFilter->IndexNextValue++;
	// Wenn man hinten angekommen ist, vorne wieder anfangen.
	io_pFloatAvgFilter->IndexNextValue %= SIZE_OF_AVG;
}

void InitFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter,
	bool i_DefaultValue)
{
	// Den Buffer mit dem Initialisierungswert fuellen:
	for (char i = 0; i < SIZE_OF_AVG; ++i)
	{
		io_pFloatAvgFilter->Data[i] = i_DefaultValue;
	}
	// Der naechste Wert soll an den Anfang des Buffers geschrieben werden:
	io_pFloatAvgFilter->IndexNextValue = 0;
}

bool CheckAVG(tFloatAvgFilter * io_pFloatAvgFilter)
{
	int trues = 0;
	double o_Result = true;
	// Durchschnitt berechnen
	for (int i = 0; i < SIZE_OF_AVG; ++i)
	{
		if ((io_pFloatAvgFilter->Data[i]) == true)
			trues++;
	}
	if (trues >= (SIZE_OF_AVG * MIN_FOR_DETECT))
		return true;
	else
		return false;
}

void GetValue(tFloatAvgFilter * io_pFloatAvgFilter)
{
	for (int i = 0; i < SIZE_OF_AVG; ++i)
	{
		printf("%d", io_pFloatAvgFilter->Data[i]);
	}
}