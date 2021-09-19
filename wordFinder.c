/**
 * This program takes a word and up to 5 input files as command line arguments.
 * It then searches each line in each file for the specified word and prints every line the word appears on.
 * Each file is searched by a different thread.
 *
 * This program was tested on a Linux Lab Machine (Realm RH6) environment on NCSU’s virtual computer lab (VCL) utilizing a SSH connection.
 * Compilation and run commands are detailed below and were entered into the shell. 
 * Compile with: gcc -Wall -std=c99 find2.c -o find2 -lpthread
 * Run with: ./find2 WORD FILE1.EXT FILE2.EXT FILE3.EXT FILE4.EXT FILE5.EXT
 */

#include <pthread.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** Need a global variable for the word since threads will only pass the file name to be analyzed */
char* word;

/** 
 * Check that a file to be scanned is valid.
 * If it is valid and can be opened scan each line. If the word being searched
 * for is on the line print it to the standard output
 *
 * @param params The parameter being passed by the calling function. In this case the file name to be scanned and analyzed.
 * @return Void pointer - Not used
 */
void* runner(void* params)
{
  //Cast the parameter to a char* so file readers can open it
  char* paramsName = (char *) params;
  
  //Set up a file pointer for scanning entire lines
  FILE *fpLines = fopen(paramsName, "r");

  //If file cannot be opened exit with a failure
  if (!fpLines) {
    fprintf(stderr, "File %s cannot be opened.\n", paramsName);
    exit(EXIT_FAILURE);
  }

  //Get the first character in the file
  int next = fgetc(fpLines);
  //Scan the whole file
  while (next != EOF) {
    //Allocate a string with a small initial capacity
    int capacity = 5;
    //The number of characters currently in the buffer
    int len = 0;
    //The buffer that will contain the line
    char *buffer = (char *) malloc (capacity + 1);
    //Add characters to the buffer until \n or EOF is reached in a line
    while (next != '\n' && next != EOF) { 
      //Double buffer capacity as needed
      if (len >= capacity) {
        capacity *= 2;
        buffer = (char *) realloc (buffer, capacity + 1);
      }
      buffer[len++] = (char) next;
      next = fgetc(fpLines);
    }
    //Terminate the buffer's string
    buffer[len] = '\0';
    //Convert the string to lower-case in a new buffer
    char *bufferLower = (char *) malloc (len);
    for (int i = 0; buffer[i]; i++) {
      bufferLower[i] = tolower(buffer[i]);
    }
    //Convert the search word to lower case as well
    char *wordLower = (char *) malloc (strlen(word));
    for (int i = 0; word[i]; i++) {
      wordLower[i] = tolower(word[i]);
    }
    wordLower[strlen(word)] = '\0';
    //Search the buffer for the word of choice and print the line if the word is in it
    if (strstr(bufferLower, wordLower) != NULL) {
      fprintf(stdout, "%s - %s\n", paramsName, buffer);
    }
    //Free the malloc'd space for the buffers and word
    free(buffer);
    free(bufferLower);
    free(wordLower);
    //Advance the character in the file
    next = fgetc(fpLines);
  }
  //Print an extra line for readability
  fprintf(stdout, "\n");
  //End the thread
  pthread_exit(0);
}

/** 
 * Starting point for the program
 *
 * @param argc The number of arguments passed on the command line
 * @param argv The array of argument values passed on the command line
 */
int main(int argc, char* argv[])
{
  //Print a line for readability
  fprintf(stdout, "\n");
  
  //Error checking on whether a word and the correct number of files were passed in the command line
  //There must be a word and one to five files to search. If this is not the case exit the program with a failure
  if (argc < 3 || argc > 7) {
    //Exit if less than 1 or more than 5 files passed
    fprintf(stderr, "Too few or too many arguments. Must have one word and at least one but no more than five files.\n");
    exit(EXIT_FAILURE);
  }
  
  //Set up an array of thread identifiers. Each thread will read a distinct file.
  //Subtract two to account for the program name (./find2.c) and the word to be found
  pthread_t tid[argc - 2];
  //Do the same for the thread attributes
  pthread_attr_t attr[argc - 2];
  
  //Set up the word to be searched for
  word = (char *) malloc (strlen(argv[1]));
  strcpy(word, argv[1]);
  
  //Standardizing for-loop counter to start at 2 to get to the first file to be processed.
  for (int i = 2; i < argc; i++) {
    //Initialize the thread's attributes
    pthread_attr_init(&attr[i - 2]);
    //Create the thread and have it scan files and report the lines containing the word of choice
    pthread_create(&tid[i - 2], &attr[i - 2], runner, argv[i]);
  }
  
  //Wait for each thread to exit
  for (int i = 2; i < argc; i++) {
    pthread_join(tid[i - 2],NULL);
  }
  //Free the space for the word
  free(word);
  //Return if program exited correctly
  return EXIT_SUCCESS;
}
