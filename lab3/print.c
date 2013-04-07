#include <stdio.h>

int main()
{
	FILE* filelog;
	filelog	= fopen("log.txt", "w");

	fprintf(filelog, "Hello world");
}