#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */
#include "Hw1TextFileReader.h"
#include "TestChecker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>
#include <tchar.h>

int main(int argc, char *argv[]) 
{

	TestLine *test_line_array;
	int num_of_threads = 0;
	int *num_of_threads_ptr = &num_of_threads;
	TextFileReader textFileReader;
	if ( argc == 1 )
	{
		printf("ERROR - Not enough arguments\n");
		exit(1);
	}
	textFileReader = CreateTextFileReader(argv[2]);
	if (textFileReader.IsInitialized == FALSE)
	{
		printf("Failed to create the Text File Reader\n");
		exit(1);
	}
	if (textFileReader.NumOfLines == 0)
		exit(1);

	test_line_array = (TestLine*) malloc(textFileReader.NumOfLines * sizeof(*test_line_array));
	
	if (test_line_array == NULL)
		exit(1);

	if (!TextFileReaderToTestLineArray(textFileReader, test_line_array,argv[1],num_of_threads_ptr))
		exit(1);

	if (RunThreads(*num_of_threads_ptr, test_line_array))
		exit(1);

	if (PrintResultToFile(argv[3], argv[4], *num_of_threads_ptr, test_line_array, *num_of_threads_ptr))
		exit(1);
	
	
	free(test_line_array);
	return 0;
}

