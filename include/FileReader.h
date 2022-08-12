#ifndef FILE_READER_H
#define FILE_READER_H

typedef struct FixedPointsData {
    int x;
    int y;
    int r;
    int g;
    int b;
}FixedPoint;

typedef struct FileReaderData {
    int imageSize;
    int qtdFixedPoints;
    FixedPoint* fixedPoints;
}FileData;

void readFile(char* path, FileData* data);
void printFileData(FileData* data);

#endif