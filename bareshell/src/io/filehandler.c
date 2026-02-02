#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

char** list_files(const char *path){
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    int files = 0;


    if (d){
        
        while ((dir = readdir(d)) != NULL){
            files+=1;
        }
        closedir(d);        
    } else {
        perror("Error opening directory");
    }

    char **file_pointers = malloc((files + 1) * sizeof(char*));

    d = opendir(path);


    int i = 0;
    if (d){
        
        while ((dir = readdir(d)) != NULL){
            file_pointers[i] = strdup(dir->d_name);
            i+=1;
        }
        closedir(d);        
    } else {
        perror("Error opening directory");
    }

    file_pointers[i] = NULL;

    return file_pointers;
    
}
