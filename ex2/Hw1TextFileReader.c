/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
This file contains the implementation of the Text File Reader module. 
See TextFileReader.h for general details and usage.

Implementation details: 
1. The content of the text file is copied into dynamic memory.
2. A parser recognizes words as sequences that are separated by whitespaces.
3. An '\0' symbol is inserted at the end of each word, making it possible to
   use it as a string.
4. A structure of pointers to word-starts and to line-start is created,
   making it possible to access each word using array-like indexing (the
   structure is similar to the one in the module Dynamic2DIntArr ).
5. Before handing over the object to the user, all data structures are made
   const. This prevents the user from inadvertently destroying the internal 
   structure. This is equivalent to giving the user read-only access to the 
   contents of the text file.
   
This File was written as part of HW assignment 2 in the course "Introduction
To Systems Programming" at Tel-Aviv University's School of Electrical 
Engineering, Winter 2011, by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//int main() {
//
//}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Hw1TextFileReader.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

typedef char* Word;
typedef Word* Line;

/* TextReaderRawData is used to pass intermediate results between internal */
/* functions. It contains more implementation details than the */
/* TextFileReader struct */
typedef struct 
{
    Line *FirstWordInLine;
    Word *WordStarts;
    int* WordsInLine;
    int NumOfLines;
    int TotalNumOfWords;
} TextReaderRawData ;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/* WHITE_SPACE symbols separate between words. EOL separate between words AND
   end a line */
#define WHITE_SPACE " \t" 
#define EOL "\r\n"
#define WHITE_SPACE_OR_EOL ( WHITE_SPACE EOL ) 

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/* ============================== */
/* Internal Function Declarations */
/* ============================== */

/** 
 * SkipWhiteSpaceSequence returns a pointer to the first place in the text 
 * after pc that does not contains a WHITE_SPACE character. If pc is not 
 * pointing to a WHITE_SPACE character, then pc itself is returned.
 **/
char* SkipWhiteSpaceSequence( char* pc );

/** 
 * SkipWord returns a pointer to the first place in the text 
 * after pc that contains a WHITE_SPACE or END_OF_LINE character. The output
 * can be pc itself.
 **/
char* SkipWord( char* pc );

/** 
 * IsEOL returns TRUE if the character that pc is pointing to is part of an 
 * End-Of-Line marker. 
 **/
BOOL IsEOL( char *pc );

/** 
 * SkipEOL returns a pointer to the first place in the text after pc that 
 * does not contains an End-Of-Line marker. If pc is not pointing at an EOL
 * marker, then pc itself is returned.
 **/
char* SkipEOL( char* pc );

/**
 * This function accepts a buffer containing text, and creates 
 * a data structure that makes it simple to access each word on each line.
 *
 * Accepts:
 * -------
 * R - input and output parameter. A pointer to a struct that contains pointers
 *     to previously allocated buffers. These buffer will be filled by this
 *     function, and will together become a data structure that allows access
 *     to each word in the file using simple array-like indexing.
 * Text - Input and output parameter. A buffer of chars containing text, and 
 *        ending with a zero ('\0'). This function will replace a whitespace 
 *        at the end of each word by a zero ('\0'), which will make it possible
 *        to use each word as a string.      
 *
 * Remark 1: words are assumed to be separated by a sequence of whitespace 
 *           characters. The number of whitespaces doesn't matter. 
 * Remark 2: End-of-line markers can also separate between words, but they also 
 *           mark the end of a line. Empty lines are handled correctly 
 *           (not ignored).
 */ 
void CreatePointerArrays( TextReaderRawData* R, char *Text );

/**
 * CalculateFileSize returns the size of a file that is already open.
 *
 * Accepts:
 * -------
 * File - a FILE* to a file that is already open in binary read mode.
 *        remark: this function may work in other modes as well, but was 
 *                only tested with files opened in binary read modes. 
 * Returns:
 * -------
 * On success, the size of the file in bytes.
 * On failure, (-1).
 */
int CalculateFileSize( FILE* File ); 

/**
 * InitializeTextFileReader_CleanupOnError() is a helper function for
 * InitializeTextFileReader(). It releases dynamic memory and closes files
 * used by InitializeTextFileReader, if necessary.
 */
void InitializeTextFileReader_CleanupOnError( FILE* TextFile,
                                              char *Buffer,
                                              TextReaderRawData R );
/**
 * AllocateDynamicArrays is a helper function for InitializeTextFileReader().
 * It allocates some dynamic arrays needed by InitializeTextFileReader.
 *
 * Accepts:
 * -------
 * FileSizeInBytes - the size of the text file being read.
 * BufferPtr - output parameter, pointer to a char pointer which
 *             the function will be set to point to a dynamically allocated
 *             array of chars.
 * TextReaderRawData - output parameter, a pointer to a struct whose fields will 
 *                     be set by this function to point to dynamically allocated 
 *                     arrays.
 * Returns:
 * -------
 * TRUE if succeeded allocating memory. FALSE otherwise.
 */ 
BOOL AllocateDynamicArrays( int FileSizeInBytes, 
                            char** BufferPtr, 
                            TextReaderRawData* Rptr );

/**
 * MinimizeDynamicArraySizes is used after CreatePointerArrays(). 
 * CreatePointerArrays parses a text buffer and creates a data
 * structure of pointers to the beginning of each word and each line.
 * since the number of words and lines is not known in advance, 
 * CreatePointerArrays uses dynamically-allocated arrays that are usually
 * too large (large enough to handle the worst case). After CreatePointerArrays
 * runs the exact number of words and lines is known, and 
 * MinimizeDynamicArraySizes can release the unnecessary extra storage space.
 * 
 * Accepts:
 * -------
 * Rptr - Input and output parameter. a pointer to a data structure containing 
 *        the data-structure: arrays of pointers to words and line beginnings, 
 *        and records of the number of words and lines. This function uses
 *        realloc() to resize the pointer arrays to the exact necessary size,
 *        and free extra unused memory.
 */ 
BOOL MinimizeDynamicArraySizes( TextReaderRawData* Rptr );

/**
 * Returns a TextFileReader object with the IsInitialized field set to FALSE.
 */ 
const TextFileReader UninitializedTextFileReader();

/**
 * This function converts from the internal data type TextReaderRawData to 
 * a TextFileReader struct, which is part of the module's interface. The 
 * conversion process hides internal dat from the user, and converts the
 * data structure into const (read-only), to prevent the user from accidentally
 * damaging it. 
 * 
 * Accepts:
 * -------
 * R - a struct of type TextReaderRawData, conatining all the data of the 
 *     data structure created by this module.
 * Buffer - a pointer to a dynamically allocated text array that contains
 *          the text read from the text file. 
 */
const TextFileReader InitializeTextFileReaderStruct( 
    TextReaderRawData R, 
    char *Buffer );

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*                    Implementation of Interface functions                  */
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/* The macro TFR_CLEAN_UP_AND_RETURN_ERROR() is used by the function */
/* CreateTextFileReader() to perform cleanup on error. Using it makes the */
/* code of CreateTextFileReader() shorter and more readable */
#define TFR_CLEAN_UP_AND_RETURN_ERROR()                              \
{                                                                    \
    InitializeTextFileReader_CleanupOnError( TextFile, Buffer, R );  \
    return UninitializedTextFileReader();                            \
}                                    

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

const TextFileReader CreateTextFileReader( const char *TextFileName )
{    
    TextReaderRawData R = { NULL, NULL, NULL, 0, 0 }; 
    FILE *TextFile = NULL;
    int FileSizeInBytes;
    char *Buffer = NULL;
    size_t AmountRead;
    BOOL Successful;

    TextFile = fopen(TextFileName, "rb"); /* Open the file in binary mode so */
                     /* that we can read the entire file at once using fread */
    if ( TextFile == NULL ) TFR_CLEAN_UP_AND_RETURN_ERROR();

    FileSizeInBytes  = CalculateFileSize( TextFile );    
    if ( FileSizeInBytes  == (-1) ) TFR_CLEAN_UP_AND_RETURN_ERROR();

    Successful = AllocateDynamicArrays( FileSizeInBytes, &Buffer, &R );
    if ( !Successful ) TFR_CLEAN_UP_AND_RETURN_ERROR();

    /* Read entire ext file into buffer: */
    AmountRead = fread( Buffer, sizeof(char), FileSizeInBytes, TextFile );
    Buffer[FileSizeInBytes] = '\0'; /* to allow the buffer to be used as a string */
    if ( AmountRead != FileSizeInBytes ) TFR_CLEAN_UP_AND_RETURN_ERROR();

    fclose( TextFile );

    CreatePointerArrays( &R, Buffer );

    Successful = MinimizeDynamicArraySizes( &R );
    if ( !Successful ) TFR_CLEAN_UP_AND_RETURN_ERROR();

    return InitializeTextFileReaderStruct( R, Buffer );
}

#undef TFR_CLEAN_UP_AND_RETURN_ERROR 

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

void DeleteTextFileReader( const TextFileReader Reader )
{
    /* The TextFileReader struct is an interface struct used by the user. */
    /* To prevent accidental damage to the structure, the data structure is */
    /* read-only (const). In order to delete the data structure, we must */
    /* convert all internal structures to non-const: */

    Line* FirstWordInLineArr = NULL;
    Word* AllWordsArr = NULL;
    char* Buffer = NULL;

    if ( Reader.IsInitialized == FALSE )
        return;

    FirstWordInLineArr = (Line*)Reader.WordsArr;
    AllWordsArr = (Word*)(FirstWordInLineArr[0]);
    Buffer = (char*)Reader.PrivateData; /* A pointer to the the Buffer */ 
    /* containing the text file is necessary for this function. However, */
    /* the user does not need to know about it. This is achieved by keeping */
    /* the pointer to the Buffer in the 'PrivateData' field of the */ 
    /* TextFileReader -  the user does not know what it is and shouldn't touch it */
    
    free(FirstWordInLineArr);
    free(AllWordsArr);    
    free(Buffer);
    free((int*)Reader.WordsInLine);
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*                    Implementation of Internal functions                  */
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/**
 * CreatePointerArrays() Implementation notes:
 * This function goes over a buffer of text, letter by letter, and recognizes
 * end of words and end of lines. At the end of each word, it places a '\0' 
 * character, to make it possible to use this word as a string. It creates 
 * a pointer to the beginning of each word and of each line, and sets them up
 * in a data structure that makes it possible to access each word using 
 * array-like indexing. It records the number of words per line, and the 
 * number of lines.
 */ 
void CreatePointerArrays( TextReaderRawData* R, char *Text )
{
    typedef enum { 
        TEXT_START, 
        AFTER_LINE_END, 
        AFTER_WORD, 
        AFTER_WHITESPACES } ParserStatus_t; /* The heart of this function */
        /* is a state-machine based parser. ParserStatus_t contains the states */
        /* that the machine can be in. */

    ParserStatus_t Status = TEXT_START;
    ParserStatus_t NextStatus = TEXT_START;

    char* CurPlace = Text; /* initialize to beginning of text */
    char* PrevPlace = NULL;
    
    int OverallWordIndex = 0;
    int LineIndex = 0;

    while( (CurPlace != NULL) && (*CurPlace != '\0') )  /* any of these can mark */
                                                        /* the end of the Text */
    {
        Status = NextStatus;
        PrevPlace = CurPlace;

        if ( IsEOL( CurPlace ) || ( Status == TEXT_START  ) )
        {
            if ( Status != TEXT_START )
                LineIndex++;

            R->FirstWordInLine[LineIndex] = &(R->WordStarts[OverallWordIndex]); 
            R->WordsInLine[LineIndex] = 0;
            NextStatus = AFTER_LINE_END;
            CurPlace = SkipEOL( CurPlace );
        }
        else
        {
            switch( Status )
            {
            case AFTER_LINE_END:
            case AFTER_WORD:
                CurPlace = SkipWhiteSpaceSequence( CurPlace );
                NextStatus = AFTER_WHITESPACES ;
                break;
            case AFTER_WHITESPACES:
                /* Handle Word Start */
                R->WordStarts[OverallWordIndex] = CurPlace;
                OverallWordIndex++;
                R->WordsInLine[LineIndex]++;
                NextStatus = AFTER_WORD ;
                CurPlace = SkipWord( CurPlace );
                break;
            default:
                assert(0);
            };
        }

        /* this must be done at the end of the loop, because it overwrites */
        /* a part of the string that we need to read first */
        if ( Status == AFTER_WORD )
            *PrevPlace ='\0';
    }

    R->NumOfLines = LineIndex + 1;
    R->TotalNumOfWords = OverallWordIndex;
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

char* SkipWhiteSpaceSequence( char* pc )
{
    size_t LengthOfWhitespaceSequence = strspn( pc, WHITE_SPACE );
    return ( pc + LengthOfWhitespaceSequence );
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

char* SkipWord( char* pc )
{
    return strpbrk( pc, WHITE_SPACE_OR_EOL );
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

BOOL IsEOL( char *pc )
{
    size_t LengthOfEOLSequence = strspn( pc, EOL );
    
    return ( LengthOfEOLSequence != 0 );
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

char* SkipEOL( char* pc )
{    
    size_t LengthOfEOLSequence = strspn( pc, EOL );
    return ( pc + LengthOfEOLSequence );
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/
 
int CalculateFileSize( FILE* File )
{
    int FunctionRes;
    int FileSizeInBytes;

    FunctionRes = fseek( File, 0, SEEK_END );
    
    if ( FunctionRes != 0 )
        return (-1);

    FileSizeInBytes = ftell( File );

    rewind( File );

    return FileSizeInBytes;
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

void InitializeTextFileReader_CleanupOnError( FILE* TextFile,
                                                     char *Buffer,
                                                     TextReaderRawData R )
{
    if ( TextFile != NULL )
        fclose( TextFile );
    
    /* free doesn't crash if it receives a NULL, so we can free all pointers */
    /* without checking if they are already pointing to allocated memory */
    free( Buffer );
    free( R.FirstWordInLine );
    free( R.WordStarts );
    free( R.WordsInLine );
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

BOOL AllocateDynamicArrays( int FileSizeInBytes, 
                                 char** BufferPtr, 
                                 TextReaderRawData* Rptr )
    {

        int MaxLines = FileSizeInBytes; /* Worst case: a file made entirely */ 
                                                           /* of '\n' signs */
        int MaxTotalWords = ( (FileSizeInBytes+1) / 2 ); /* each two words */
        /* must be separated by a space, so at most half of the bytes in */ 
        /* the file are one-letter words, and the rest are spaces. */
    
        (*BufferPtr) = (char*)malloc( (FileSizeInBytes + 1)*sizeof(char) ); 
        /* 1 is added for the '\0' at the end of the buffer, which will make */ 
        /* it possible to treat the Buffer as a string */

        Rptr->FirstWordInLine =  (Line*)malloc( MaxLines * sizeof(Line) );
        Rptr->WordStarts = (Word*)malloc( MaxTotalWords * sizeof(Word) );
        Rptr->WordsInLine = (int*)malloc( MaxTotalWords * sizeof(int) );

        if (    ( (*BufferPtr) == NULL ) ||
                ( Rptr->FirstWordInLine == NULL ) ||
                ( Rptr->WordStarts == NULL ) ||
                ( Rptr->WordsInLine == NULL ) )
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/
BOOL MinimizeDynamicArraySizes( TextReaderRawData* Rptr )
{
    Rptr->WordStarts = (Word*)realloc( Rptr->WordStarts, 
                                        Rptr->TotalNumOfWords * sizeof( Word ) );
    Rptr->FirstWordInLine = (Line*)realloc( Rptr->FirstWordInLine, 
                                            Rptr->NumOfLines * sizeof( Line ) );
    Rptr->WordsInLine = (int*)realloc( Rptr->WordsInLine, 
                                        Rptr->NumOfLines * sizeof( int ) );

    if (    ( Rptr->WordStarts      == NULL ) ||                                  
            ( Rptr->FirstWordInLine == NULL ) ||                                   
            ( Rptr->WordsInLine     == NULL ) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

const TextFileReader UninitializedTextFileReader()
{
    TextFileReader Reader = {   NULL,   /* WordsArr      */
                                NULL,   /* WordsInLine   */
                                0,      /* NumOfLines    */
                                FALSE,  /* IsInitialized */
                                NULL }; /* PrivateData   */
    return Reader;
}

/*o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.*/

const TextFileReader InitializeTextFileReaderStruct( 
    TextReaderRawData R, 
    char *Buffer )
{
    /* The data structure is converted to read-only (const) to prevent the */
    /* user from damaging it by mistake: */
    TextFileReader Reader = UninitializedTextFileReader(); /* initializes all */ 
                                                    /* pointer fields to NULL */
    Reader.NumOfLines = R.NumOfLines;
    Reader.WordsInLine = (ConstInt*)R.WordsInLine;
    Reader.WordsArr = (ConstLine*)R.FirstWordInLine;
    
    Reader.PrivateData = (void*)Buffer; /* A pointer to the the Buffer */ 
    /* containing the text file is necessary for DeleteTextFileReader(). */     
    /* However, the user does not need to know about it. This is achieved by  */
    /* keeping the pointer to the Buffer in the 'PrivateData' field of the */ 
    /* TextFileReader -  the user does not know what it is and shouldn't touch it */
    
    Reader.IsInitialized = TRUE;
    return Reader;
}
