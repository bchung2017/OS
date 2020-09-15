#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#define BUFSIZE 4096

void readAndWrite(int inputfd, int outputfd, char *buffer);


int main(int argc, char *argv[])
{
	char buffer[BUFSIZE];
	
	char nameBuff[10];
	memset(nameBuff, 0, 10);
	strncpy(nameBuff, "asdfXXXXXX", 10);
	int stdout_temp_fd = -1;
	stdout_temp_fd = mkstemp(nameBuff);
	unlink(nameBuff);
	// const char stdout_temp[BUFSIZE];
	//FILE* stdout_temp = tmpfile();
	// read(STDIN_FILENO, buffer, 10);
	int opt;
	int outputfd = stdout_temp_fd;
	int inputfd;
	int j, k;
	bool isSTDOUT = true;


	//parse -o argument
	while((opt = getopt(argc, argv, "o:")) != -1) {
		switch(opt) {
			case 'o':
				outputfd = open(optarg, O_CREAT|O_WRONLY|O_TRUNC, 0666);
				printf("optarg: %s\n", optarg);
				isSTDOUT = false;
				//printf("optind: %d\n", optind);
				break;
			default:
				fprintf(stderr, "%c is not an arg\n", opt);
				exit(EXIT_FAILURE);
		}
	}

	//OPEN input files and READ into buffer and WRITE to output file
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-o") == 0)
		{

			i++;	//also skip over output filename
			if(i >= argc)
			{
				readAndWrite(STDIN_FILENO, outputfd, buffer);
			}
			continue;
		}

		//if argv[i] is the "-'' then read from stdin
		if(strcmp(argv[i],"-") == 0)
		{
			readAndWrite(STDIN_FILENO, outputfd, buffer);
		}

		else
		{
			inputfd = open(argv[i], O_RDONLY);
			if (inputfd < 0)
			{
				//ADD inputfd VALIDITY ERROR CHECKS
			}
			readAndWrite(inputfd, outputfd, buffer);
		}
	}
	//printf("value at optind-1: %s\n", argv[optind-1]);	//NOTE: optind starts with 1
	if(isSTDOUT)
	{
		lseek(outputfd, 0, SEEK_SET);
		readAndWrite(outputfd, STDOUT_FILENO, buffer);
	}
	close(outputfd);
}	

void readAndWrite(int inputfd, int outputfd, char *buffer)
{
	int readOutput, writeOutput;
	
	while((readOutput=read(inputfd, buffer, BUFSIZE)) > 0)
	{
		// printf("read output: %d\n", readOutput);
		// printf("read errno: %d\n", errno);
		// printf("buffer: %s\n", buffer);

		writeOutput = write(outputfd, buffer, readOutput);

		// int length = strlen(buffer);
		// printf("%d\n", length);
		// printf("write output: %d\n", writeOutput);
		// printf("write errno: %d\n", errno);

		
		if(inputfd == STDIN_FILENO)
		{
			memset(buffer, 0, sizeof(buffer));
			break;
		}
	}
	memset(buffer, 0, sizeof(buffer));
}