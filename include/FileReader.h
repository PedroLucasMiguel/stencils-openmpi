#ifndef FILE_READER_H
#define FILE_READER_H

typedef struct FileReaderData {
    int imageSize;
    int qtdFixedPoints;
    int (*fixedPoints)[5];
}FileData;

void readFile(char* path, FileData* data);
void printFileData(FileData* data);

#endif