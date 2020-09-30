#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#define BUFSIZE 4096


void readDir(char *dn);
int readStat(char *dn);
void readFileInfo(struct stat st);

static int inodeTypes[15] = {};
static long long int numOfFileBytes = 0;
static long long int numOfFileBlocks = 0;
static int long numOfMultiLinkInodes = 0;
static int numOfInvalidSymLinks = 0;
static int numOfProblematicDirNames = 0;


int main(int argc, char * argv[]) {

	char startDirectoryName [BUFSIZE];
	strcpy(startDirectoryName, argv[1]);
	readDir(startDirectoryName);

	printf("inode types:\n");
	printf("S_IFREG: %d\n", inodeTypes[0]);
	printf("S_IFDIR: %d\n", inodeTypes[1]);
	printf("S_IFLNK: %d\n", inodeTypes[2]);
	printf("S_IFCHR: %d\n", inodeTypes[3]);
	printf("S_IFBLK: %d\n", inodeTypes[4]);	
	printf("S_IFIFO: %d\n", inodeTypes[5]);	
	printf("S_IFSOCK: %d\n", inodeTypes[6]);	

	printf("Total number of file bytes: %lld\n", numOfFileBytes);
	printf("Total number of file blocks: %lld\n", numOfFileBlocks);
	printf("Total number of inodes with multiple links: %ld\n", numOfMultiLinkInodes);
	printf("Total number of invalid sym links: %d\n", numOfInvalidSymLinks);
	printf("Total number of problematic directory names: %d\n", numOfProblematicDirNames);

}

void readDir(char *dn) {
	DIR *dirp;
	struct dirent *de;
	struct stat st;


	switch(readStat(dn)) {
		case -1:
			fprintf(stderr, "Error reading file: %s\n", dn);
			return;
		case 1:
			break;
		case 2:
			return;
		}
	errno = 0;


	if (!(dirp=opendir(dn)))
	{
		fprintf(stderr, "Error opening file: %s\n", dn);
		return;
	}
	errno = 0;

	
	while (de=readdir(dirp))
	{
		if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			char nextDir [BUFSIZE]= "";
			strcat(strcat(strcat(nextDir, dn), "/"), de->d_name);
			readDir(nextDir);
		}
	}
	if(errno)
	{
		fprintf(stderr, "Error reading directory: %s\n", dn);
	}
	closedir(dirp);
	return;
}

int readStat(char *dn) {
	struct stat st;
	char buffer[BUFSIZE] = "";
	if(stat(dn, &st) != 0) {
		fprintf(stderr, "Cannot stat file %s\n", dn);	
	}
	for(int i = 0; i < (sizeof(dn)/sizeof(dn[0])); i++) {
		if(!(isprint(dn[i])||isspace(dn[i]))) {
			numOfProblematicDirNames++;
			break;
		}	
	}
	
	if(st.st_nlink > 2) {
		numOfMultiLinkInodes++;
	}


	switch(st.st_mode & S_IFMT) {
		case S_IFREG:  
    		readFileInfo(st);     
    		inodeTypes[0]++;
    		return 2;
    	case S_IFDIR:   
    		inodeTypes[1]++;
    		return 1;
    	case S_IFLNK:   	
    		if(readlink(dn, buffer, BUFSIZE) == -1) {
    			numOfInvalidSymLinks++;
    			memset(buffer, 0, sizeof(buffer));    
    		}
			inodeTypes[2]++;           
    		return 1;
    	case S_IFCHR:  
    		inodeTypes[3]++;          
			return 1;
		case S_IFBLK:  
			inodeTypes[4]++;              
			return 1;
    	case S_IFIFO:  
    		inodeTypes[5]++;                 
    		return 1;  	
    	case S_IFSOCK: 
    		inodeTypes[6]++;                    
    		return 1;
    	default:                  
    		return -1;
	}
	
}

void readFileInfo(struct stat st) {
	numOfFileBytes += (long long) st.st_size;
	numOfFileBlocks += (long long) st.st_blocks;
}