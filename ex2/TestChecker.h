#ifndef TEST_CHECKER_H
#define TEST_CHECKER_H
#include "Hw1TextFileReader.h"
#include <math.h>

#define MAX_CHARS_IN_STRING 100

#define EXPECTED_NAME 50
#define CMD_LINE 100

typedef struct TestLine {
	char* CmdInput;
	char* OutputFileName;
	char* ExpectedFileName;
	char* ResultMsg;
	char* TimeMsg;
	int ThreadInd;

} TestLine;

int TextFileReaderToTestLineArray(TextFileReader text_file_reader, TestLine* test_line_array, const char *exe_file_name,int *num_of_threads);
int RunThreads(int num_of_threads, TestLine *test_line_array);
int PrintResultToFile(char *output_file_name1, char *output_file_name2, int num_of_threads_ptr, TestLine *test_line_array, int num_of_threads);


#endif
