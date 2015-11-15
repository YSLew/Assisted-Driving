
#define SIZE_OF_AVG 5
#define MIN_FOR_DETECT 0.5

typedef struct
{
	bool Data[SIZE_OF_AVG];
	char IndexNextValue;
} tFloatAvgFilter;

void AddToFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter, bool i_NewValue);
void InitFloatAvg(tFloatAvgFilter * io_pFloatAvgFilter, bool i_DefaultValue);
bool CheckAVG(tFloatAvgFilter * io_pFloatAvgFilter);
void GetValue(tFloatAvgFilter * io_pFloatAvgFilter);
