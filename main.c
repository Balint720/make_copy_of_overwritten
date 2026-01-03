#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int numFilesBackedUp = 0;
int numFilesDeleted = 0;

void printErrorDir(const char* directory) {
    printf("Error with opening directory!\n");
    switch (errno) {
        case EACCES:
            printf("Search permission is denied for the component of the path prefix of %s or read permission is denied for %s.\n", directory, directory);
            break;
        case ENAMETOOLONG:
            printf("Length of directory name exceeds PATH_MAX or pathname component is longer than NAME_MAX.\n");
            break;
        case ENOTDIR:
            printf("A component of %s is not a directory.\n", directory);
            break;
        case ENFILE:
            printf("Too many files are open in the system.\n");
            break;
        default:
            printf("Error number: %i\n", errno);
            break;
    }
}

int deleteFilesFromDirectory(const char* directory, bool force) {
    // Give a prompt
    if (!force) {
        printf("Backup folder \"%s\" already exists, would you like to delete it's contents? (y: yes, n: no, c: cancel): ", directory);
        fflush(stdin);
        switch (getchar()) {
            case 'y':
                break;
            case 'n':
                printf("Files not deleted.\n");
                return -2;
                break;
            case 'c':
                printf("Backup cancelled.\n");
                return -1;
                break;
            default:
                printf("Undefined character passed, cancelling backup.");
                return -1;
                break;
        }
    }
    if (!force) printf("\nFile deletion started...\n");
    // Open directory
    DIR* dir = opendir(directory);
    if (errno != 0) {
        printErrorDir(directory);
        return -1;
    }

    printf("Opened directory\n");
    struct dirent* dirstruct = {0};
    struct stat fileStat = {0};
    while ((dirstruct = readdir(dir)) != NULL) {
         // Ignore . and ..
        if ((strcmp(dirstruct->d_name, ".") == 0) || (strcmp(dirstruct->d_name, "..") == 0))
            continue;
        char filePath[200];
        strcpy(filePath, directory); strcat(filePath, "\\"); strcat(filePath, dirstruct->d_name);
        if (stat(filePath, &fileStat)) printf("Err number: %i", errno);

        switch (fileStat.st_mode & S_IFMT) {
            case S_IFDIR:
                printf("%s is a directory, deleting subdirectory contents first...\n", dirstruct->d_name);
                deleteFilesFromDirectory(filePath, true);
                if (!rmdir(filePath)) {
                    printf("%s directory successfully deleted.\n", dirstruct->d_name);
                }
                break;
            default:
                printf("Deleting file named %s...\n", dirstruct->d_name);
                if (!unlink(filePath))
                    numFilesDeleted++;
                else
                    printf("File was not deleted.\n");
                break;
        }
    }
    if (!force) printf("File deletion over. Number of files deleted: %i\n", numFilesDeleted);
    return 0;
}

void backupMatchingFiles(const char* pathFrom, DIR* dirFrom, const char* pathTo, DIR* dirTo, const char* pathBackup);

int main() {
    printf("Hello World!\n");

    // Variables
    char from[200], to[200], backup[200];

    printf("Enter the directory you are going to copy: ");
    gets(from);
    printf("Enter the directory you are going to copy into: ");
    gets(to);
    printf("Enter where you would like the files backed up: ");
    gets(backup);

    // Open directory streams
    DIR* dirFrom = opendir(from);
    if (errno != 0) printErrorDir(from);
    DIR* dirTo = opendir(to);
    if (errno != 0) printErrorDir(to);
    // Create backup directory if it doesn't exist, but if it does, delete contents
    struct stat st = {0};
    if (stat(backup, &st) == -1) mkdir(backup);
    else {
        switch (deleteFilesFromDirectory(backup, false)) {
            case -1:
                printf("Program terminated due to backup folder opening error.");
                exit(-1);
                break;
            case -2:
                printf("Files in backup folder might be overwritten by files in the \"to\" folder.\nIs that ok? (y or n): ");
                fflush(stdin);
                if (getchar() != 'y') {
                    printf("Backup cancelled.\n");
                    exit(-2);
                }
                break;
        }
    }
    errno = 0;
    DIR* dirBackup = opendir(backup);
    if (errno != 0) printErrorDir(backup);

    if (errno == 0) printf("\nSuccessfully opened from, to and backup directories!\n");
    else {
        printf("\nFailed to open directories, program terminated.\n");
        return -1;
    }

    printf("\nStarting backup...\n\n");
    backupMatchingFiles(from, dirFrom, to, dirTo, backup);
    printf("\nBackup done, number of files backed up: %i.\n", numFilesBackedUp);

    // Close directory streams
    closedir(dirFrom);
    closedir(dirTo);
    closedir(dirBackup);

    fflush(stdin);
    getchar();

    return 0;
}

void backupMatchingFiles(const char* pathFrom, DIR* dirFrom, const char* pathTo, DIR* dirTo, const char* pathBackup) {
        // Compare files
    struct dirent* fromDirStruct, *toDirStruct;
    
    // Get file names from the "from" directory
    while ((fromDirStruct = readdir(dirFrom)) != NULL) {
        // Ignore . and ..
        if ((strcmp(fromDirStruct->d_name, ".") == 0) || (strcmp(fromDirStruct->d_name, "..") == 0))
            continue;
        
        // Start reading files from beginning
        rewinddir(dirTo);

        // Get file names from the "to" directory
        while ((toDirStruct = readdir(dirTo)) != NULL) {
            // If file name matches, make a copy in backup folder
            if (strcmp(toDirStruct->d_name, fromDirStruct->d_name) == 0) {
                printf("Both directories contain file named '%s', making copy.\n", toDirStruct->d_name);

                // File copy
                // Creating file path strings
                char filePathTo[200];
                strcpy(filePathTo, pathTo); strcat(filePathTo, "\\"); strcat(filePathTo, toDirStruct->d_name);
                char filePathBackup[200];
                strcpy(filePathBackup, pathBackup); strcat(filePathBackup, "\\"); strcat(filePathBackup, toDirStruct->d_name);

                // Checking if file is a file or a directory
                struct stat fileStat = {0};
                stat(filePathTo, &fileStat);

                // Proceeding depending on if it's a file or a directory
                if ((fileStat.st_mode & S_IFMT) == S_IFDIR) {
                        printf("%s is actually a directory, copying subdirectory...\n", toDirStruct->d_name);

                        // Creating file path that is missing
                        char filePathFrom[200];
                        strcpy(filePathFrom, pathFrom); strcat(filePathFrom, "\\"); strcat(filePathFrom, toDirStruct->d_name);

                        // Opening new directories: if they fail, we continue the backup, ignoring the folder
                        DIR* subDirFrom = opendir(filePathFrom);
                        if (errno != 0) {
                            printErrorDir(filePathFrom);
                            printf("Continuing backup skipping folder.\n");
                            continue;
                        }
                        DIR* subDirTo = opendir(filePathTo);
                        if (errno != 0) {
                            printErrorDir(filePathFrom);
                            closedir(subDirFrom);
                            printf("Continuing backup skipping folder.\n");
                            continue;
                        }

                        // Create folder in backup if it doesn't exist
                        struct stat st = {0};
                        if (stat(filePathBackup, &st) == -1) mkdir(filePathBackup);

                        // Recursion
                        backupMatchingFiles(filePathFrom, subDirFrom, filePathTo, subDirTo, filePathBackup);

                        // Close the directories
                        closedir(subDirFrom);
                        closedir(subDirTo);

                        printf("%s subdirectory has been copied, continuing from parent folder.\n", toDirStruct->d_name);
                }
                else {
                    // Open files, write all characters from the "to" file into the "backup" file
                    FILE* toFile = fopen(filePathTo, "rb");
                    FILE* backupFile = fopen(filePathBackup, "wb");
                    for (;;) {
                        char buffer[8];
                        size_t n;

                        n = fread(buffer, 1, sizeof buffer, toFile);
                        if (n == 0) break;

                        fwrite(buffer, 1, n, backupFile);
                    }

                    // Close files
                    fclose(toFile);
                    fclose(backupFile);

                    numFilesBackedUp++;
                }

            }
        }
    }
}