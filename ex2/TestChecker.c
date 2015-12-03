#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */

#include "TestChecker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>
#include <tchar.h>


#define TIMEOUT_IN_MILLISECONDS 5000
#define BRUTAL_TERMINATION_CODE 0x55

FILETIME SubtractFiletimes(FILETIME Late, FILETIME Early);
char* ConcatComandLine(	char* a, char* b, ConstChar* c);

int CreateTestLine(TestLine* testLine)
{
	testLine = (TestLine*) malloc(sizeof(*testLine));
	if (testLine == NULL)
		exit(1);
	testLine->ThreadInd=0;
	testLine->ResultMsg= "test #";
	return 1;
}

int SaveTestLineProperty(TestLine* test_line, ConstLine file_line, int words_in_line, const char *exe_file_name)
{
	int word_ind = 0;

	char* command_line = (char*) malloc(strlen(exe_file_name) * sizeof(char));
	if (command_line == NULL)
		exit(1);
	strcpy (command_line,exe_file_name);

	for (word_ind = 0; word_ind <words_in_line; word_ind++)
	{
		command_line = ConcatComandLine(command_line," ",file_line[word_ind]);
		if (word_ind == words_in_line -1)
		{
			test_line->OutputFileName = (char*) malloc (sizeof(char) * strlen(file_line[word_ind]));
			if (test_line->OutputFileName == NULL)
				exit(1);
			strcpy(test_line->OutputFileName,file_line[word_ind]);
			test_line->CmdInput = (char*) malloc (sizeof(char) * strlen(command_line));
			if (test_line->CmdInput == NULL)
				exit(1);
			strcpy(test_line->CmdInput,command_line);
		}
	}

	//free (command_line);
	return 1;
}

int TextFileReaderToTestLineArray(TextFileReader text_file_reader, TestLine* test_line_array, const char *exe_file_name ,int *num_of_threads)
{
	int word_size = 0;
	int file_line_ind;
	int test_line_ind = 0;

	for (file_line_ind = 0; file_line_ind < text_file_reader.NumOfLines; file_line_ind++)
	{
		if (text_file_reader.WordsInLine[file_line_ind] > 0)
		{
			if (text_file_reader.WordsInLine[file_line_ind] > 1)
			{
				if (!CreateTestLine(&test_line_array[test_line_ind]))
					return 0;
				if (!SaveTestLineProperty(&test_line_array[test_line_ind], text_file_reader.WordsArr[file_line_ind], text_file_reader.WordsInLine[file_line_ind],exe_file_name))
				{
					return 0;
				}
			}
			else if (strlen(text_file_reader.WordsArr[file_line_ind][0]) > 2)
			{
				test_line_array[test_line_ind].ExpectedFileName = (char*) malloc (sizeof(char) * strlen(text_file_reader.WordsArr[file_line_ind][0]));
				if (test_line_array[test_line_ind].ExpectedFileName == NULL)
					exit(1);
				strcpy(test_line_array[test_line_ind].ExpectedFileName,text_file_reader.WordsArr[file_line_ind][0]);
				test_line_array[test_line_ind].ThreadInd = test_line_ind;
				test_line_ind++;
			}
		}
	}
	*num_of_threads = test_line_ind;
	return 1;
}



HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE StartAddress,
	LPVOID ParameterPtr,
	LPDWORD ThreadIdPtr)
{
	return CreateThread(
		NULL,            /*  default security attributes */
		0,                /*  use default stack size */
		StartAddress,    /*  thread function */
		ParameterPtr,    /*  argument to thread function */
		0,                /*  use default creation flags */
		ThreadIdPtr);    /*  returns the thread identifier */
}

BOOL CreateNewProcess(LPTSTR CommandLine, PROCESS_INFORMATION *ProcessInfoPtr)
{
	STARTUPINFO	startinfo = { sizeof(STARTUPINFO), NULL, 0 }; 

	return CreateProcess(
		NULL,					/*  module name (use command line). */
		CommandLine,			/*  Command line. */
		NULL,					/*  Process handle not inheritable. */
		NULL,					/*  Thread handle not inheritable. */
		FALSE,					/*  Set handle inheritance to FALSE. */
		NORMAL_PRIORITY_CLASS,	/*  creation/priority flags. */
		NULL,					/*  Use parent's environment block. */
		NULL,					/*  Use parent's starting directory. */
		&startinfo,				/*  Pointer to STARTUPINFO structure. */
		ProcessInfoPtr			/*  Pointer to PROCESS_INFORMATION structure. */
		);
}

int CompareCharsInFile(TestLine* test_line)
{
	char ch1;
	char ch2;
	int since_mismatch_counter = 0;
	FILE* f_expected = fopen(test_line->ExpectedFileName, "r");
	FILE* f_output = fopen(test_line->OutputFileName, "r");

	if (f_output == NULL) {
		printf("Cannot open %s for reading ", test_line->OutputFileName);
		exit(1);
	} else if (f_expected == NULL) {
		printf("Cannot open %s for reading ", test_line->ExpectedFileName);
		exit(1);
	} else {
		ch1 = getc(f_output);
		ch2 = getc(f_expected);

		while ((ch1 != EOF) && (ch2 != EOF)) {
			if (ch1 != ch2)
			{
				since_mismatch_counter++;
				ch1 = getc(f_output);
				ch2 = getc(f_expected);
			}
			else 
			{
				ch1 = getc(f_output);
				ch2 = getc(f_expected);
				if (since_mismatch_counter != 0)
				{
					since_mismatch_counter++;
				}
			}

		}
	}
	fclose(f_output);
	fclose(f_expected);

	if (since_mismatch_counter == 0 || since_mismatch_counter < 10)
	{
		// printf("Files are identical ");
		return 0;
	}
	else 
	{
		//  printf("Files are Not identical ");
		return 1;
	}
}

void CompareResultMessage(DWORD exitcode,TestLine* test_line)
{
	test_line->ResultMsg = (char*) malloc (sizeof(char) * 32);
	if (test_line->ResultMsg == NULL)
		exit(1);

	if (exitcode != 0)
	{
		sprintf(test_line->ResultMsg, "test #%d : Crashed #%d\n", test_line->ThreadInd+1,exitcode);
	}
	else
	{
		if (!CompareCharsInFile(test_line))
			sprintf(test_line->ResultMsg, "test #%d : Secceeded\n", test_line->ThreadInd+1);
		else
			sprintf(test_line->ResultMsg, "test #%d : Failed\n", test_line->ThreadInd+1);
	}
}

int RunSnoopy (TestLine* test_line)
{
	PROCESS_INFORMATION procinfo;
	DWORD				waitcode;
	DWORD				exitcode;
	BOOL				retVal;
	SYSTEMTIME          UTC_time, UTC_total_time,local_start_time, local_user_time, local_kernel_time;
	FILETIME            start_time, exit_time, total_time, user_time, kernel_time;
	test_line->TimeMsg = (char*) malloc (sizeof(char) * 500);
	if (test_line->TimeMsg == NULL)
		exit(1);

	/*  Start the child process. */
	retVal = CreateNewProcess(test_line->CmdInput, &procinfo);


	if (retVal == 0)
	{
		printf("Process Creation Failed!\n");
		return 1;
	}


	waitcode = WaitForSingleObject(
		procinfo.hProcess,
		INFINITE); 

	if (waitcode != WAIT_OBJECT_0)
	{
		printf("Unexpected output value of 0x%x from WaitForSingleObject(). "
			"Ending function.\n", waitcode);
		CloseHandle(procinfo.hProcess);
		return 1;
	}

	GetExitCodeProcess(procinfo.hProcess, &exitcode);
	CompareResultMessage(exitcode,test_line);

	GetProcessTimes(
		procinfo.hProcess,
		&start_time,
		&exit_time,
		&kernel_time,
		&user_time
		);

	FileTimeToSystemTime(&start_time,&UTC_time);
	SystemTimeToTzSpecificLocalTime(NULL,&UTC_time, &local_start_time);

	total_time = SubtractFiletimes(exit_time, start_time);
	FileTimeToSystemTime(&total_time,&UTC_total_time);

	FileTimeToSystemTime(&user_time,&UTC_time);
	SystemTimeToTzSpecificLocalTime(NULL,&UTC_time, &local_user_time);

	FileTimeToSystemTime(&kernel_time,&UTC_time);
	SystemTimeToTzSpecificLocalTime(NULL,&UTC_time, &local_kernel_time);

	sprintf(test_line->TimeMsg, "test #%d : start_time=%02d:%02d:%02d:%d total_time=%d user_time=%d kernel_time=%d\n",
		test_line->ThreadInd,local_start_time.wHour, local_start_time.wMinute, local_start_time.wSecond, local_start_time.wMilliseconds,
		((UTC_total_time.wMinute*60000)+ (UTC_total_time.wSecond*1000) + UTC_total_time.wMilliseconds),
		((local_user_time.wMinute*60000)+ (local_user_time.wSecond*1000) + local_user_time.wMilliseconds),
		((local_kernel_time.wMinute*60000)+ (local_kernel_time.wSecond*1000) + local_kernel_time.wMilliseconds));

	CloseHandle(procinfo.hProcess); /* Closing the handle to the process */
	CloseHandle(procinfo.hThread); /* Closing the handle to the main thread of the process */

	return 0;
}


int RunThreads(int num_of_threads, TestLine *test_line_array)
{
	DWORD* ThreadIDs; /* An array of threadIDs */
	HANDLE* ThreadHandles; /* An array of thread handles */
	DWORD exitcode;
	int i;

	ThreadIDs =(DWORD*) malloc(num_of_threads*sizeof(*ThreadIDs));
	if (ThreadIDs == NULL){
		exit(1);
	}
	ThreadHandles =(HANDLE*) malloc(num_of_threads*sizeof(*ThreadHandles));
	if (ThreadHandles == NULL){
		exit(1);
	}

	for (i=0; i< num_of_threads; i++)
	{
		ThreadHandles[i] = CreateThreadSimple(
			(LPTHREAD_START_ROUTINE)RunSnoopy,  /*  thread function */
			&test_line_array[i],                 /*  argument to thread function */
			&ThreadIDs[i]);                    /*  returns the thread identifier */
	}

	/* Wait for all threads to end: */
	WaitForMultipleObjects(
		num_of_threads,
		ThreadHandles,
		TRUE,       /* wait until all threads finish */
		INFINITE);

	Sleep(10);

	for (i = 0; i < num_of_threads; i++)
	{
		GetExitCodeThread(ThreadHandles[i], &exitcode);
		if (exitcode != 0)
		{
			exit(1);
		}
		CloseHandle(ThreadHandles[i]);

	}

	free (ThreadIDs);
	free (ThreadHandles);
	return 0;

}

char* ConcatComandLine(	char* a, char* b, ConstChar* c)
{
	int total_size = strlen(a)+strlen(b)+strlen(c);
	char* string = (char*) malloc(total_size* sizeof(char));
	if (string == NULL)
	{
		exit(1);
	}
	strcpy(string,a);
	strcat(string,b);
	strcat(string,c);

	return string;
}

int PrintResultToFile(char *output_file_name1, char *output_file_name2,int num_of_threads_ptr, TestLine *test_line_array, int num_of_threads)
{
	int test_line_ind = 0;
	FILE* correctness_file = fopen(output_file_name1,"w+");
	FILE* runtime_file = fopen(output_file_name2,"w+");
	if(NULL == correctness_file || NULL == runtime_file )
	{
		printf("\n fopen() Error!!!\n");
		return 1;
	}

	for (test_line_ind = 0; test_line_ind < num_of_threads; test_line_ind++)
	{
		fprintf(correctness_file,"%s",test_line_array[test_line_ind].ResultMsg);
		fprintf(runtime_file,"%s", test_line_array[test_line_ind].TimeMsg);
	}
	fclose(correctness_file);
	fclose(runtime_file);
	return 0;
}

FILETIME SubtractFiletimes(FILETIME Late, FILETIME Early)
{
	typedef unsigned __int64 Unsigned64BitType;
	Unsigned64BitType Late64BitVal;
	Unsigned64BitType Early64BitVal;
	Unsigned64BitType Difference64BitVal;
	FILETIME DifferenceAsFILETIME;
	const Unsigned64BitType Low32BitsMask = 0x00000000FFFFFFFF;

	Late64BitVal = ((Unsigned64BitType)(Late.dwHighDateTime) << 32) +
		Late.dwLowDateTime;
	Early64BitVal = ((Unsigned64BitType)(Early.dwHighDateTime) << 32) +
		Early.dwLowDateTime;
	Difference64BitVal = Late64BitVal - Early64BitVal;

	DifferenceAsFILETIME.dwLowDateTime =
		(DWORD)(Difference64BitVal & Low32BitsMask);
	DifferenceAsFILETIME.dwHighDateTime =
		(DWORD)(Difference64BitVal >> 32);

	return DifferenceAsFILETIME;
}
