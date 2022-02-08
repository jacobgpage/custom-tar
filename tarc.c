/*

    Name: Tarc // Lab 4a
    Author: Jacob Page
    Takes a given path name of a directory and reads it to stdout to compress the directory

*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "/home/jplank/cs360/include/fields.h"
#include "/home/jplank/cs360/include/dllist.h"

/**
* @name isDirectory
* @brief Used to determine if the given path name is a file or directory
* @param[in] path The path to the file you want to check
* @return Returns 1 if it is a directory
*/
int isDirectory(const char* path) {
    struct stat checkFile;
    stat(path, &checkFile);

    return S_ISDIR(checkFile.st_mode);

}

int main(int argc, char* argv[]) {
    DIR* dir;
    struct dirent* de;
    
    Dllist temp, dirNames;
    dirNames = new_dllist();
    
    long iNodes[256];
    char name[256], prefix[256], suffix[256];
    struct stat fileInfo;
    int length, numNodes = 0, i, slashLocation = 0;

    length = strlen(argv[1]) - 1;
    for(i = length; i > 0; i--) { //Finds the location of the last / to split the prefix and suffix
        if(argv[1][i] == '/' && i != length) {
            slashLocation = i;

            break;
        }

    }

    if(slashLocation != 0) { //Splits the prefix and the suffix if needed
        strncpy(prefix, argv[1], slashLocation + 1);
        strcpy(suffix, &argv[1][slashLocation + 1]);
    } else {
        strcpy(suffix, argv[1]);
        prefix[0] = '\0';
    }

    if(isDirectory(argv[1]) == 0) { //Makes sure the provided name is a directory
        fprintf(stderr, "Error: No Directory with name: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    } else {
        dll_append(dirNames, new_jval_s(strdup(suffix)));
    }

    //Traverses a list of the directories and as it goes into other directories and finds directories it adds them to the list
    dll_traverse (temp, dirNames) {
        char fullName[256];
        strcpy(fullName, prefix);
        strcat(fullName, temp->val.s);

        dir = opendir(fullName);
        if (dir == NULL) {
            perror("");
            exit(EXIT_FAILURE);
        }

        stat(fullName, &fileInfo);

        length = strlen(temp->val.s); //Writes the name length, name, inode, mode, and modification time of directory
        fwrite(&length, sizeof(int), 1, stdout);
        fwrite(temp->val.s, sizeof(char) * strlen(temp->val.s), 1, stdout);
        fwrite(&fileInfo.st_ino, sizeof(long), 1, stdout);
        fwrite(&fileInfo.st_mode, sizeof(int), 1, stdout);
        fwrite(&fileInfo.st_mtime, sizeof(long), 1, stdout);

        while ((de = readdir(dir)) != NULL) { //Reads through the directory
            if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
                strcpy(name, temp->val.s); //Makes the full name by adding the prefix to the file name
                strcat(name, "/");
                strcat(name, de->d_name);
                strcpy(fullName, prefix);
                strcat(fullName, name);

                if (isDirectory(fullName) == 0) { //The given name is a file
                    stat(fullName, &fileInfo);

                    length = strlen(name);
                    fwrite(&length, sizeof(char) * sizeof(int), 1, stdout); //Writes the name length, name, and inode
                    fwrite(&name, strlen(name), 1, stdout);
                    fwrite(&fileInfo.st_ino, sizeof(long), 1, stdout);

                    int findINode = 0; //Checks if this file shares its inode with any other files
                    for(i = 0; i < numNodes; i++) {
                        if(iNodes[i] == fileInfo.st_ino) {
                            findINode = 1;
                        }
                    }

                    if (findINode == 0) { //File is not a hard link
                        iNodes[numNodes] = fileInfo.st_ino;
                        numNodes++;

                        FILE* fp;
                        fp = fopen(fullName, "r");
                        if(fp == NULL) {
                            perror("Error: ");
                            exit(EXIT_FAILURE);
                        }

                        fwrite(&fileInfo.st_mode, sizeof(int), 1, stdout); //Writes the files mode, modification time, size, and data to the tarc
                        fwrite(&fileInfo.st_mtime, sizeof(long), 1, stdout);
                        fwrite(&fileInfo.st_size, sizeof(long), 1, stdout);
                        char* buffer = malloc(fileInfo.st_size);
                        fread(buffer, 1, fileInfo.st_size, fp);
                        fwrite(buffer, fileInfo.st_size, 1, stdout);

                        fclose(fp);
                        free(buffer);
                    }

                } else {
                    dll_append(dirNames, new_jval_s(strdup(name)));
                }
            }
        }
        closedir(dir);
        free(de);
    }

    dll_traverse(temp, dirNames) { //Free memory
        free(temp->val.s);

    }
    free_dllist(dirNames);

    return (0);
}