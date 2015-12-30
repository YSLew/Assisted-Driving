/** Header-File for pseudo floating average functions (.h)
*
* Contains function to generate, manipulate and read a buffer for pseudo floating averages.
* Attention: all functions will call it a floating average, because of the underlain basics.
* CheckAVG will NOT get the Average value, but will get "1" if the most values are true.
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////////////////

#define SIZE_OF_AVG 5
#define MIN_FOR_DETECT 0.5

//////////////////////////////////////////////////////////////////////////////////////
//TYPEDEF
//////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	bool Data[SIZE_OF_AVG];
	char IndexNextValue;
} tFloatAvgFilter;

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

void AddToFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter, bool i_NewValue);
void InitFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter, bool i_DefaultValue);
bool CheckAVG(tFloatAvgFilter * io_pFloatAvgFilter);
void GetValue(tFloatAvgFilter * io_pFloatAvgFilter);
