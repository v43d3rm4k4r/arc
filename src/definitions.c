#include "declarations.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//=========================================================================================================
void initArc(char* path, char* real_bin_file)
{
    path = (char*)realloc(path, strlen(path)+strlen("\\")+1);
    strcat(path, "\\");

    real_bin_file = (char*)calloc(strlen(path)+strlen("binary.zipper")+1, sizeof(char));

    // make a copy of the path for assignment to real_bin_file, so as not to touch the original path (it will still be needed)
    char* path_copy = (char*)calloc(strlen(path)+1, sizeof(char));
    strcpy(path_copy, path);

    path_copy = (char*)realloc(path_copy, strlen(path_copy)+strlen("binary.zipper")+1);
    strcat(path_copy, "binary.arc");
    strcpy(real_bin_file, path_copy);

    free(path_copy);
}
//=========================================================================================================
static char* get_file_name(char* full_str)
{
    if (strrchr(full_str, '\\'))
        return strrchr(full_str, '\\');
    else
        return full_str;
}
//=========================================================================================================
// For counting the number of digits including. This will be required, for example, to write a number to the archive,
// as a dynamic character buffer
static uint8_t digs(double w)
{
    uint8_t yield = 0;
    while (w > 10)
    {
        ++yield;
        w /= 10;
    }
    return yield+1;
}
//=========================================================================================================
static void getInfo(char* path, char** files, uint32_t files_count)
{
    char* s_info = NULL;
    char* path_copy = (char*)calloc(strlen(path)+strlen("info.txt")+1, sizeof(char));
    strcpy(path_copy, path);
    strcat(path_copy, "info.txt");
    remove(path_copy);

    // saving info in our text file
    FILE* info = fopen(path_copy, "a+");

    // Opening all files one by one
    int bytes_size = 0; // information block length in bytes
    for (uint32_t i = 0; i < files_count; ++i)
    {
        FILE* f = fopen(files[i], "rb");
        if (!f)
            break;

        // get the size of the archived file
        fseek(f, 0, SEEK_END);
        int size = ftell(f);

        char* name = (char*)calloc(strlen(files[i]), sizeof(char));
        strcpy(name, files[i]);
        name = get_file_name(name);

        char* m_size = (char*)calloc(digs(size), sizeof(char));
        itoa(size, m_size, 10);
        fclose(f);

        bytes_size += digs(size);

        bytes_size += strlen(name);


        // save everything that we have dug into an intermediate buffer:
        s_info = (char*)calloc(strlen(m_size)+strlen("||")*2+strlen(name)+1, sizeof(char));
        strcpy(s_info, m_size);
        strcat(s_info, "||");
        strcat(s_info, name);
        strcat(s_info, "||");

        free(name);
        free(m_size);
    }

    //bytes_size = strlen(s_info)+2;

    char* b_buff = (char*)calloc(digs(bytes_size), sizeof(char));

    itoa(bytes_size, b_buff, 10);

    if (digs(bytes_size) < 5)
    {
        char str[5-digs(bytes_size)];
        memset(str, '0', sizeof(str));
        fputs(str, info);
    }
    fputs(b_buff, info);
    fputs("||", info);
    fputs(s_info, info);

    free(s_info);
    free(path_copy);

    fclose(info);
}
//=========================================================================================================
void packing(char* path, char** files, uint32_t files_count, char* real_bin_file)
{
    char byte[1]; // single buffer to read one byte

    getInfo(path, files, files_count); // get the necessary information about what we are archiving

    // файл - архив
    FILE* f, *bin_file = fopen(real_bin_file, "wb");
    char* path_copy = (char*)calloc(strlen(path)+strlen("info.txt")+1, sizeof(char));
    strcpy(path_copy, path);
    strcat(path_copy, "info.txt");
    FILE* info = fopen(path_copy, "rb");

    // rewrite the information into the archive
    while (!feof(info))
        if (fread(byte, 1, 1, info) == 1) fwrite(byte, 1, 1, bin_file);

    fclose(info);

    remove(path_copy);

    // sequential writing to the archive of archived files byte-by-byte:
    for (uint32_t i = 0; i < files_count; ++i)
    {
        f = fopen(files[i], "rb");
        if(!f)
        { printf("%s not found!\n", files[i]); break; }
        while (!feof(f))
        {
            if (fread(byte, 1, 1, f) == 1) fwrite(byte, 1, 1, bin_file);
        }
        printf("%s added to archive '%s'.\n", files[i], real_bin_file);
        fclose(f);
    }
    fclose(bin_file);

    free(path_copy);
}
//=========================================================================================================
void unpacking(char* binary, char* path)
{
    // open the archive in read binary mode
    FILE* bin = fopen(binary, "rb");
    if (!bin)
    {
        printf("Fatal: cant open file: %s\n", binary);
        return;
    }

    char info_block_size[5];        // information block size
    fread(info_block_size, 1, 5, bin);  // get the size
    int32_t sz = atoi(info_block_size);  // convert the buffer to a number

    // information block
    char* info_block = (char*)calloc(sz, sizeof(char));
    fread(info_block, 1, sz, bin);   // read it

    // Parsing an information block:
    char** tokens = (char**)calloc(1, sizeof(char*));
    char* tok = strtok(info_block, "||");
    uint32_t toks = 0;
    while (tok)
    {
        if (strlen(tok) == 0) break;
        tokens[toks] = (char*)calloc(strlen(tok)+1, sizeof(char));
        strcpy(tokens[toks], tok);
        tok = strtok(NULL, "||");
        toks++;
    }

    int toks_count = toks;

    if (toks % 2 == 1)
        --toks;  // remove garbage
    int files = toks / 2;  // the number of files found in the archive

    char byte[1];   // single buffer to read one byte

    // The process of unpacking all files (according to the rules obtained from the block with information):
    for (int i = 0; i < files; ++i)
    {
        const char* size = tokens[i*2];
        const char* name = tokens[i*2+1];
        char full_path[255];
        strcpy(full_path, path);
        strcat(full_path, name);
        int sz = atoi(size);
        printf("'%s' repacked to '%s'.\n", name, path);
        FILE* curr = fopen(full_path, "wb");
        for (int r = 1; r <= sz; ++r)
            if (fread(byte, 1, 1, bin) == 1)
                fwrite(byte, 1, 1, curr);
        fclose(curr);
    }
    fclose(bin);

    for (int i = 0; i < toks_count; ++i)
        free(tokens[i]);
    free(tokens);

    free(info_block);
}
