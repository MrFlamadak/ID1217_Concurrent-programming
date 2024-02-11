#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <fileapi.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>
#define WORD_LENGTH 40
#define MAX_THREADS 8

char **words;
char foundWords[1000][WORD_LENGTH];
int maxlength = 26000;
int n = 0;
int thread_count;
char* filename;

bool AlreadyFound(char* word)
{
    int mid = 0;
    int compare = 0;
    int leftBoundary = 0;
    int rightBoundary = 1000-1;
    while(leftBoundary <= rightBoundary)
    {
        mid =  (rightBoundary - leftBoundary)/2 + leftBoundary;
        if(strlen(foundWords[mid]) == 0)
        {
                rightBoundary = mid-1;
                continue;
        }
        //printf(words[mid]);
        compare = strcmp(word, foundWords[mid]);
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
        //printf("looking for %s\nfound %s ",word, words[mid]);
    }

        return false;
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
    //printf("%d\n", j);
    //printf("%s\n", word);
    //printf("%s", word[2]);
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
    char word[20];
    words = malloc(maxlength*sizeof(word));
    for(int i = 0; i < maxlength; i++)
    {
        words[i] = malloc((WORD_LENGTH + 1)* sizeof(word));
    }
    int i = 0;
    while(fgets(word, sizeof(word), dictionary))
    {
        word[strcspn(word, "\n")] = '\0';
        
        strcpy(words[i], strlwr(word));
        i++;
    }
    n = i-1;

    //printf("%d", i);
    fclose(dictionary);
}

void PutArrayContentIntofile()
{
    //printf("putting in file");
    FILE* outputFile = fopen("outputfile.txt", "w+");
    int j = 0;
    while(strlen(foundWords[j]) != 0)
    {
        fputs(foundWords[j], outputFile);
        fputs("\n", outputFile );
        j++;
    }
    fclose(outputFile);
    free(words);
}
bool IsPalindrome(char* word)
{
    if(AlreadyFound(word))
        return true;
    else
    {
        flipWord(word);
        return BinarySearch(word, 0, n-1);
    }
}

void findPalindromes()
{
    
    int j = 0;
    int partitionSize = n/thread_count;
    //parastuff num_threads
    double start_time = omp_get_wtime(); 
    # pragma omp parallel num_threads(thread_count)
    {
    char word[20];
    int threadid = omp_get_thread_num();
    int myFirst = threadid*partitionSize;
    //myfirst=threadid*partitionsize
    int myLast = (threadid == thread_count-1) ? n-1 : (myFirst + partitionSize - 1);
    //go
    for(int i = myFirst; i <= myLast; i++)
    {
        // word is private variable for each thread, we could prolly have some
        // type of bag with i being wordnumber or we divide and let each thread take a seqtion
        strcpy(word, words[i]);
        if(IsPalindrome(word))
        {
            //critical section
            # pragma omp critical
            {
            strcpy(foundWords[j], words[i]);
            j++;
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
    //printf("yoyo");
    findPalindromes();
    PutArrayContentIntofile();
    return 0;
}
