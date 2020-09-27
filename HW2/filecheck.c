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

	printf("inode types: ");	
	for(int i = 0; i < 15; i++)
	{
		printf("%d ", inodeTypes[i]);
	}
	printf("\n");

	printf("numOfFileBytes: %lld\n", numOfFileBytes);
	printf("numOfFileBlocks: %lld\n", numOfFileBlocks);
	printf("numOfMultiLinkInodes: %ld\n", numOfMultiLinkInodes);
	printf("numOfInvalidSymLinks: %d\n", numOfInvalidSymLinks);
	printf("numOfProblematicDirNames: %d\n", numOfProblematicDirNames);

}

void readDir(char *dn) {
	DIR *dirp;
	struct dirent *de;
	struct stat st;


	readStat(dn);
	errno = 0;


	if (!(dirp=opendir(dn)))
	{
		fprintf(stderr, "Not a directory: %s\n", dn);
		//printf("strerror: %s\n\n", strerror(errno));
		return;
	}
	errno = 0;

	
	while (de=readdir(dirp))
	{
		printf("inode name: %s\n", de->d_name);
		if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			char nextDir [BUFSIZE]= "";
			printf("nextDir address: %p\n", &nextDir);
			strcat(strcat(strcat(nextDir, dn), "/"), de->d_name);

			printf("de->d_name: %s\n", de->d_name);
			printf("next directory being examined: %s\n\n", nextDir);
			readDir(nextDir);
			//printf("name: %s\n", de->d_name);
		}
		else {
			printf("\n");
		}
	}
	if(errno)
	{
		fprintf(stderr, "Error reading file\n");
	}
	closedir(dirp);
	return;
}

int readStat(char *dn) {
	struct stat st;
	if(stat(dn, &st) != 0)
		return -1;
	for(int i = 0; i < (sizeof(dn)/sizeof(dn[0])); i++) {
		if(!(isprint(dn[i])||isspace(dn[i]))) {
			numOfProblematicDirNames++;
			break;
		}	
	}
	
	printf("inode: %ld\n", (long) st.st_ino);
	printf("link count: %ld\n", (long) st.st_nlink);
	if(st.st_nlink > 2) {
		printf("multilinkinode: %s\n", dn);
		numOfMultiLinkInodes++;
	}
	printf("mode: %lo (octal)\n", (unsigned long) st.st_mode);


	printf("inode type: ");
	switch(st.st_mode & S_IFMT) {
		case S_IFREG:  
    		printf("regular file\n");    
    		readFileInfo(st);     
    		inodeTypes[0]++;
    		break;
    	case S_IFDIR:  
    		printf("directory\n");   
    		inodeTypes[1]++;
    		break;
    	case S_IFLNK:  
    		printf("symlink\n");  
    		char buffer[BUFSIZE] = "";
    		if(readlink(dn, buffer, BUFSIZE) == -1)
    			numOfInvalidSymLinks++;    
			inodeTypes[2]++;           
    		break;
    	case S_IFCHR:  
    		printf("character device\n");
    		inodeTypes[3]++;          
			break;
		case S_IFBLK:  
			printf("block device\n");
			inodeTypes[4]++;              
			break;
    	case S_IFIFO:  
    		printf("FIFO/pipe\n");
    		inodeTypes[5]++;                 
    		break;  	
    	case S_IFSOCK: 
    		printf("socket\n");
    		inodeTypes[6]++;                    
    		break;
    	default:       
    		printf("unknown?\n");                
    		break;
	}
	// fd=open("output1.txt", O_RDONLY);
	// fstat(fd, &st);	
}

void readFileInfo(struct stat st) {
	printf("bytes allocated: %lld\n", (long long) st.st_size);
	printf("blocks allocated: %lld\n", (long long) st.st_blocks);

	numOfFileBytes += (long long) st.st_size;
	numOfFileBlocks += (long long) st.st_blocks;
}