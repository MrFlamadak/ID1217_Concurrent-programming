#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <fileapi.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>
#define MAX_WORD_LENGTH 40
#define MAX_FILE_LENGTH 26000
#define MAX_THREADS 8

char **words;
bool palindromic[MAX_FILE_LENGTH];
int size = 0;
int thread_count;
char* filename;

bool AlreadyFound(int wordIndex)
{
    return palindromic[wordIndex];
}
bool BinarySearch(char* word, int leftBoundary, int rightBoundary)
{
    int mid = 0;

    int compare = 0;
    while(leftBoundary <= rightBoundary)
    {
        mid =  (rightBoundary - leftBoundary)/2 + leftBoundary;
        compare = strcmp(word, words[mid]);

        if(compare == 0)
        {
            return true;
        }
        else if(compare > 0)
        {
            leftBoundary = mid + 1;
        }
        else
        {
            rightBoundary = mid - 1;
        }
    }

        return false;
}


void flipWord(char* word)
{
    int i = 0;
    int j = strlen(word) - 1;
    while(i <= j)
    {
        char letter = word[i];  
        word[i] = word[j];
        word[j] = letter;
        i++;
        j--;        
    }
}


void PutFileContentIntoArray()
{
    FILE* dictionary = fopen(filename, "r+");
    char word[MAX_WORD_LENGTH];
    words = malloc(MAX_FILE_LENGTH*sizeof(word));
    for(int i = 0; i < MAX_FILE_LENGTH; i++)
    {
        words[i] = malloc((MAX_WORD_LENGTH + 1));
    }
    int i = 0;
    while(fgets(word, sizeof(word), dictionary))
    {
        word[strcspn(word, "\n")] = '\0';
        
        strcpy(words[i], strlwr(word));
        i++;
    }
    size = i-1;

    //printf("%d", i);
    fclose(dictionary);
}

void PutArrayContentIntofile()
{
    FILE* outputFile = fopen("outputfile.txt", "w+");
    for(int i = 0; i < size; i++)
    {
        if(palindromic[i])
        {
            fputs(words[i], outputFile);
            fputs("\n", outputFile );
        }
    }
    fclose(outputFile);
    free(words);
}
bool IsPalindrome(int index)
{
    if(AlreadyFound(index))
        return true;
    else
    {
        char word[MAX_WORD_LENGTH];
        strcpy(word, words[index]);
        flipWord(word);
        return BinarySearch(word, 0, size);
    }
}

void FindPalindromes()
{
    int partitionSize = size/thread_count;
    //parastuff num_threads
    double start_time = omp_get_wtime(); 
    # pragma omp parallel num_threads(thread_count)
    {
    char word[MAX_WORD_LENGTH];
    int threadid = omp_get_thread_num();
    int myFirst = threadid*partitionSize;
    int myLast = (threadid == thread_count-1) ? size-1 : (myFirst + partitionSize - 1);

    for(int i = myFirst; i <= myLast; i++)
    {
        if(IsPalindrome(i))
        {
            //critical section
            # pragma omp critical
            {
            palindromic[i] = true;
            }
            //critical section
        }
    }
    }
    double end_time = omp_get_wtime();
    printf("Execution time was: %f", end_time-start_time); 
    //endparastuff
}

int main(int argc, char const *argv[])
{
    filename = (argc > 1) ? (char*)argv[1] : "words";    
    thread_count = (argc > 2) ? atoi(argv[2]) : MAX_THREADS;

    PutFileContentIntoArray();
    //printf("%s", words[25126]);
    FindPalindromes();
    PutArrayContentIntofile();
    return 0;
}
