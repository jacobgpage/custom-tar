/*

    Name: Tarx // Lab 4b
    Author: Jacob Page
    File reads data from a tarc file from stdin and recreates the directory just as it was

*/
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>

#include "/home/jplank/cs360/include/fields.h"
#include "/home/jplank/cs360/include/dllist.h"

typedef struct {

    char name[256];
    int mode;
    long iNode;
    long modTime;
    int isHardLink;

} FileData;

/**
* @name isDirectory
* @brief Used to determine if the given path name is a file or directory
* @param[in] mode The mode of the file read from the tarc file
* @return Returns 1 if it is a directory
*/
int isDirectory(int mode) {
    return S_ISDIR(mode);
}

/**
* @name findINode
* @brief Looks to see if the inode of the file already exists
* @param[in] iNodes the list of all current inodes
* @param[in] iNode the current inode that you are looking for
* @param[in] numFiles how many files have currently been read in
* @return Returns 0 if the file wasn't found, otherwise it returns the location of the matching inode
*/
int findINode(long iNodes[350], long iNode, int numFiles) {
    int i;
    for (i = 0; i < numFiles; i++) {
        if (iNode == iNodes[i]) {
            return(i);
        }
    }
    return(0);
}

int main(int argc, char* argv[]) {
    FileData fileData[350];

    long iNodes[350];
    int intBuffer, numFiles = 0, nameLength, i;
    long longBuffer;
    char* buffer;
	int fileNameSize;

    while((fileNameSize = fread(&intBuffer, 1, sizeof(int), stdin))) {
		if (fileNameSize != 4) { //Invalid tarc file, there wasn't enough bytes to read the file name length
			perror("Error");
		}
		
        nameLength = intBuffer;
		int readSize;

        buffer = (char*) malloc(sizeof(char) * nameLength + 1);
        readSize = fread(buffer, sizeof(char), nameLength, stdin);
		
		if (nameLength != readSize) { //Invalid tarc file, there wasn't enough bytes to read the file name
			perror("");
		}
		
        buffer[readSize] = '\0';
        if (strlen(buffer) > nameLength) {
            i = strlen(buffer) - 1;
            while (strlen(buffer) != nameLength) {
                buffer[i] = '\0';
                i--;
            }

        }

        strcpy(fileData[numFiles].name, buffer);

        fileNameSize = fread(&longBuffer, 1, sizeof(long), stdin); //Reads the Inode
        fileData[numFiles].iNode = longBuffer;
        iNodes[numFiles] = longBuffer;
		
		if(fileNameSize != 8){ //Invalid tarc file, there wasn't enough bytes to read the file inode
			perror("");
		}

        fileData[numFiles].isHardLink = 0; 

        if (findINode(iNodes, fileData[numFiles].iNode, numFiles) == 0) { //If there is no inode in the Inodes list then its not a hardlink
            fileNameSize = fread(&intBuffer, 1, sizeof(int), stdin); //Reads the mode
            fileData[numFiles].mode = intBuffer;
			
			if(fileNameSize != 4){ //Invalid tarc file, there wasn't enough bytes to read the file mode
				perror("");
			}

            fread(&longBuffer, sizeof(long), 1, stdin); //Reads the modification time
            fileData[numFiles].modTime = longBuffer;

            if (isDirectory(intBuffer) == 0) { //If true it is a file
                char* fileBuffer;

                fileNameSize = fread(&longBuffer, 1, sizeof(long), stdin); //Reads the size of the files contents and then reads the files contents
				
				if (fileNameSize != 8) { //Invalid tarc file, there wasn't enough bytes to read the file length
					perror("");
				}
				
                fileBuffer = (char*) malloc(sizeof(char) * longBuffer);
                fileNameSize = fread(fileBuffer, 1, longBuffer, stdin);
				
				if (fileNameSize != longBuffer) { //Invalid tarc file, there wasn't enough bytes to read the file contents
					perror("");
				}

                FILE* fp; //Opens file and writes the contents to it
                fp = fopen(fileData[numFiles].name, "w");

                if (fp != NULL) {
                    fwrite(fileBuffer, sizeof(char), longBuffer, fp);
                    fclose(fp);
                }
	
				free(fileBuffer);
				free(buffer);

            } else { //It was a directory
                mkdir(fileData[numFiles].name, 0700);
                free(buffer);
            }

        } else { //The file is a hardlink
            int originLoc;
            originLoc = findINode(iNodes, fileData[numFiles].iNode, numFiles);

			remove(fileData[numFiles].name);
            link(fileData[originLoc].name, fileData[numFiles].name); //Creates a hard link between two files
            fileData[numFiles].isHardLink = 1;
        
        }
        numFiles++;
    }

    for(i = 0; i < numFiles; i++) { //Loops through all the files and sets the modification time and mode
        if (fileData[i].isHardLink == 0) {
			struct stat result;
            struct utimbuf setModTime;
			
			stat(fileData[i].name, &result);
            setModTime.modtime = fileData[i].modTime;
			setModTime.actime = result.st_atime;

            utime(fileData[i].name, &setModTime);
            chmod(fileData[i].name, fileData[i].mode);
        } 
    }
    return(0);
}