#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <ctype.h>
#define BUFSIZE 4096

void readAndWrite(int inputfd, int outputfd, char *buffer, char * inputFileName);


int main(int argc, char *argv[])
{
	char buffer[BUFSIZE];
	
	char nameBuff[10];
	memset(nameBuff, 0, 10);
	strncpy(nameBuff, "asdfXXXXXX", 10);
	int stdout_temp_fd = -1;
	stdout_temp_fd = mkstemp(nameBuff);
	unlink(nameBuff);
	int opt;
	int outputfd = stdout_temp_fd;
	int inputfd;
	bool isSTDOUT = true;

	//parse -o argument
	while((opt = getopt(argc, argv, "o:")) != -1) {
		switch(opt) {
			case 'o':
				if((outputfd = open(optarg, O_CREAT|O_WRONLY|O_TRUNC, 0666)) > -1) {
					//printf("optarg: %s\n", optarg);
					isSTDOUT = false;
				}
				else {
					fprintf(stderr, "Can't open file %s: %s\n", optarg, strerror(errno));
				}
				break;
			default:
				fprintf(stderr, "%c is not an arg\n", opt);
				exit(EXIT_FAILURE);
		}
	}

	//OPEN input files and READ into buffer and WRITE to output file
	for(int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-o") == 0) {
			i++;	//also skip over output filename
			if(i >= argc) {
				readAndWrite(STDIN_FILENO, outputfd, buffer, "<standard input>");
			}
			continue;
		}
		//if argv[i] is the "-'' then read from stdin
		if(strcmp(argv[i],"-") == 0) {
			readAndWrite(STDIN_FILENO, outputfd, buffer, "<standard input>");
		}
		else {
			inputfd = open(argv[i], O_RDONLY);
			if (inputfd < 0) {
				fprintf(stderr, "Could not open input file %s correctly: %s\n", argv[i], strerror(errno));
				exit(EXIT_FAILURE);
			}
			readAndWrite(inputfd, outputfd, buffer, argv[i]);
		}
	}

	if(isSTDOUT) {
		if(argc == 1) {
			readAndWrite(STDIN_FILENO, STDOUT_FILENO, buffer, "<standard input>");
		}
		else {
			if(lseek(outputfd, 0, SEEK_SET) < 0)
			{
				fprintf(stderr, "Could not reposition file pointer correctly in output file: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			readAndWrite(outputfd, STDOUT_FILENO, buffer, "<standard input>");
		}
	}
	if(close(outputfd) < 0) {
		fprintf(stderr, "Could not close output file correctly: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(0);
}	

void readAndWrite(int inputfd, int outputfd, char *buffer, char *inputFileName)
{
	int readOutput, writeOutput;
	int readCounter = 0;
	int writeCounter = 0;
	int byteCount = 0;
	int offset;
	bool isBinary = false;
	
	while((readOutput=read(inputfd, buffer, BUFSIZE)) > 0)
	{
		readCounter++;
		for(int j = 0; j < readOutput; j++) {
			if(!(isprint(buffer[j])||isspace(buffer[j]))) {
				isBinary = true;
				break;
			}
		}
		writeOutput = write(outputfd, buffer, readOutput);	
		if(writeOutput > 0 && writeOutput < readOutput)
		{
			int offset = writeOutput;
			for(int i = 0; i < 3; i++)
			{
				fprintf(stderr, "Partial write detected, attempt rewrite %d", i);
				writeOutput = write(outputfd, buffer+offset, readOutput);
				if(writeOutput == readOutput)
					break;
			}
		}
		if(writeOutput < 0 || writeOutput != readOutput) {
			fprintf(stderr, "Could not write to specified output correctly: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		byteCount += writeOutput;
		writeCounter++;
		if(inputfd == STDIN_FILENO) {
			break;
		}
	}
	if(readOutput < 0) {
		fprintf(stderr, "Error reading file %s: %s\n", inputFileName, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(outputfd != 	STDOUT_FILENO) {
		fprintf(stderr, "\nkitty transferred %d bytes from file %s with %d read call(s) and %d write call(s)\n", byteCount, inputFileName, readCounter, writeCounter);
		if(isBinary)
			fprintf(stderr, "\nWarning: input file %s is of a binary file type\n", inputFileName);
	}
	memset(buffer, 0, sizeof(buffer));
}