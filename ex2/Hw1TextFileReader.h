/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
This file is the Header for the Text File Reader module. The Text File
Reader is a tool that simplifies the reading of text files. It reads
the text file into dynamic memory, and allows reading-access to each word 
in the file using array-like indexing. For example, the following commands 
create a text file reader to read the text file "Book.txt", and prints to
screen the 7th word on the 3rd line:

    Example usage 1
    ---------------
    const TextFileReader Reader = CreateTextFileReader( "Book.txt" );
    
    printf("%s\n", Reader.WordsArr[3][7] );

The number of lines, and the number of words for each line are supplied by the 
Text File Reader. This makes it possible to use for loops to iterate over the
text file, as follows:

    Example usage 2
    ---------------
    const TextFileReader Reader = CreateTextFileReader( "Book.txt" );

    for ( LineInd = 0 ; LineInd < Reader.NumOfLines ; LineInd++ )
    {
        for ( WordInd = 0; WordInd < Reader.WordsInLine[LineInd] ; WordInd++ )
        {
            printf("%s ", Reader.WordsArr[LineInd][WordInd]);
        }
        printf("\n");
    }
    
A Text File Reader may not always be created successfully. To test this, use code
as in the following example:

    Example usage 3
    ---------------
    const TextFileReader Reader = CreateTextFileReader( "Book.txt" );

    if ( Reader.IsInitialized == FALSE )
    {
        printf("Failed to create the Text File Reader");
        exit(1);
    }

The Text File Reader stores the content of the text file in dynamic memory.
When you are done using it, release the memory using DeleteTextFileReader(), 
as in the following example:

    Example usage 4
    ---------------
    const char* WordToSearch = "investigation";

    const TextFileReader Reader = CreateTextFileReader( "Book.txt" );

    if ( Reader.IsInitialized == FALSE )
    {
        printf("Failed to create the Text File Reader");
        exit(1);
    }

    for ( LineInd = 0 ; LineInd < Reader.NumOfLines ; LineInd++ )
    {
        for ( WordInd = 0; WordInd < Reader.WordsInLine[LineInd] ; WordInd++ )
        {
            if ( strcmp( Reader.WordsArr[LineInd][WordInd], WordToSearch ) == 0 ) 
            {
                printf("The first appearance of the word %s in the text file "
                "is as word number %d on line %d\n", 
                WordToSearch, WordInd, LineInd );
            }
        }
    }

    DeleteTextFileReader( Reader );

Created by Amnon Drory for the course "Introduction To Systems Programming", 
School of Electrical Engineering, Tel-Aviv University. winter 2012.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef HW1_TEXT_FILE_READER_H
#define HW1_TEXT_FILE_READER_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

typedef enum { FALSE = 0, TRUE = 1 } BOOL;

/* the following const-type definitions make sure that the user can */
/* use the Text File Reader for reading only, and cannot change its */
/* internal structure */
typedef const char ConstChar;
typedef const int ConstInt;
typedef ConstChar* const ConstWord;
typedef ConstWord* const ConstLine;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

typedef struct 
{
    ConstLine* WordsArr;
    ConstInt*  WordsInLine;
    int           NumOfLines;
    BOOL       IsInitialized;
    void*       PrivateData; /* behind-the-scenes data: should */
                             /* not be accessed by user. */
} TextFileReader;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/**
 * CreateTextFileReader creates a Text File Reader, an object that simplifies
 * reading from text files. It copies a text file's content into dynamic memory,
 * and creates a data structure that makes it possible to access each word in the
 * file using array-like indexing. See above for details.
 *
 * Accepts:
 * -------
 * TextFileName - the name of the text file to read.
 *
 * Returns:
 * -------
 * A Text File Reader object. If setup was successful, the the TextFileReader's
 * 'IsInitialized' will be set to TRUE. Otherwise, it will be set to FALSE.
 */
const TextFileReader CreateTextFileReader( const char *TextFileName );

/**
 * DeleteTextFileReader frees the dynamic memory that was allocated for the
 * Text File Reader.
 *
 * Accepts:
 * -------
 * Reader - a Text File Reader object. 
 */ 
void DeleteTextFileReader( const TextFileReader Reader );

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif /* HW1_TEXT_FILE_READER_H */