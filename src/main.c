#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "declarations.h"

// USE:
// for pack:
// arc --create --files file1.mp3 file2.gif --path C:\Users\User\archives
// for unpack:
// arc --extract --files C:\Users\User\archives\binary.arc --path C:\Users\User\unpack

//=========================================================================================================
int main(int argc, char* argv[])
{
    // Suppotred args:
    // --create, --extract, --files, --path

    char** files = NULL;
    char* path = NULL;
    char* real_bin_file = NULL; // path and name of the resulting archive
    uint32_t files_count = 0;

    if (argc <= 1)
    {
        puts("No args entered.");
        return EXIT_FAILURE;
    }

    printf("\n========================== ARC ==========================\n\n");

    if (argc > 1)
    {
        bool flag_files = false, flag_path = false; // read / write mode flags
        Action action;                   // type: packing or unpacking

        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "--create") == 0)
            {
                action = Create;
                flag_files = flag_path = false;
            }

            if (strcmp(argv[i], "--extract") == 0)
            {
                action = Extract;
                flag_files = flag_path = false;
            }

            if (strcmp(argv[i], "--files") == 0)
            {
                flag_files = true;
                flag_path = false;
                continue;
            }

            if (strcmp(argv[i], "--path") == 0)
            {
                flag_path = true; flag_files = false;
                continue;
            }


            if (flag_path)
            {
                // catch the last argument containing the path
                path = (char*)calloc(strlen(argv[i])+1, sizeof(char));
                strcpy(path, argv[i]);
            }

            if (flag_files)
            {
                // here you need to find out the number of files
                ++files_count;
            }
        }

        // should check if there are at least 6 arguments when the key --files is entered
        if (files_count > 0 && argc < 6)
        {
            puts("Too few arguments using --files key. Need at least 6.");
            return EXIT_FAILURE;
        }

        files = (char**)calloc(files_count, sizeof(char*));
        for (uint32_t i = 3; i < files_count+3; ++i) // 3 is the beginning of the file arguments
        {
            files[i-3] = (char*)calloc(strlen(argv[i])+1, sizeof(char));
            strcpy(files[i-3], argv[i]);
        }

        initArc(path, real_bin_file);

        if (action == Create)
            packing(path, files, files_count, real_bin_file);
        if (action == Extract)
            unpacking(files[0], path);
    }
    else
        puts("Parameters --create/--extract , --files, --path required!");

    puts("=========================================================");

    free(path);
    for (uint32_t i = 0; i < files_count; ++i)
        free(files[i]);
    free(files);
}
