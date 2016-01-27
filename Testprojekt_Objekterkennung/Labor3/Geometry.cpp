/** File for geometry functions (.cpp)
*
* Contains functions for rotation detection of shapes.
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//HEADER
//////////////////////////////////////////////////////////////////////////////////////

#include "Geometry.h"

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

/** Check rotation of triangles
*
* Detects, if the given triangle stands on a corner ("Vorfahrt Gewähren") or on a side ("Warnung").
* This can be used to validate a traffic sign.
*
* @param Point pt0, pt1, pt2 - triangle corners
∗ @return int - 0: corner, 1: side.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
int triangle_check(cv::Point pt0, cv::Point pt1, cv::Point pt2)
{
	int i;
	int min = 1000;
	int array[3];

	int A, B, C;

	array[0] = pt0.y;
	array[1] = pt1.y;
	array[2] = pt2.y;

	for (i = 0; i<3; i++)
	{
		if (array[i]<min)
		{
			min = array[i];
		}
	}

	//C is the minimum
	if (min == array[0])
	{
		C = pt0.y; A = pt1.y, B = pt2.y;
	}
	if (min == array[1])
	{
		C = pt1.y; A = pt0.y, B = pt2.y;
	}

	if (min == array[2])
	{
		C = pt2.y; A = pt0.y, B = pt1.y;
	}

	int AC = abs(A - C);
	int AB = abs(A - B);
	int BC = abs(B - C);

	if ((AC > AB) && (BC > AB))
	{
		//attention: when using image coordinates: 0|0 is in the upper left corner!
		return 1;
	}
	else
	{
		return 0;
	}

}

/** Check rotation of rectangles
*
* Detects, if the given rectangle stands on a corner, or on a side.
* This can be used to validate a traffic sign.
*
* @param Point pt0, pt1, pt2, pt3 - rectangle corners
∗ @return int - 0: corner, 1: side.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
int rectangle_check(cv::Point pt0, cv::Point pt1, cv::Point pt2, cv::Point pt3)
{
	int i, j;
	int min = 1000;
	int array[4];
	int swap;

	int A, B, C, D;

	array[0] = pt0.y;
	array[1] = pt1.y;
	array[2] = pt2.y;
	array[3] = pt3.y;

	for (i = 0; i< 4; i++)
	{
		for (j = i + 1; j<4; j++)
		{
			if (array[i] > array[j]) 
			{
				swap = array[i];
				array[i] = array[j];
				array[j] = swap;
			}
		}
	}

	A = array[3];
	B = array[2];
	C = array[1];
	D = array[0]; 

	int AD = abs(A - D);
	int DC = abs(D - C);

	if (DC > (AD / 4))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}




