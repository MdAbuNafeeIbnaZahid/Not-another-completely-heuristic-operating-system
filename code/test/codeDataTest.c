#include <stdio.h>
using namespace std;
int ar[5];


int main()
{
	/* code */
	int i, j, k;
	for (i = 0; i < 5; i++)
	{
		ar[i] = i;
	}
	for (i = 0; i < 5; i++)
	{
		printf("%d \n", ar[i]);
	}
	return 0;
}