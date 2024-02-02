#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{
    char string[100];

    FILE* wFile;
    wFile = fopen(argv[1], "w");
    if(wFile == NULL)
    {
        printf("File couldn't be opened, program terminating.");
        exit(0);
    }
    else {
        printf("File has been opened\n");
    }
    int counter = 0;
    while(1) {
    // System input
    fgets(string, sizeof(string), stdin);

    // Output the String
    fputs(string, stdout);

    // write to file
    fputs(string, wFile);  
    fflush(wFile);
    }

    fclose(wFile);
    
    return 0;
}
