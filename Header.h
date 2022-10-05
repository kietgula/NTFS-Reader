#pragma once
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>


void ReadRecord(LPCWSTR drive, long long int clusterOffset, BYTE record[1024]);

class BPB
{
private:
    WORD bytesPerSector;
    BYTE sectorsPerCluster;
    WORD reservedSectors;
    //BYTE always0_0x10[3];
    //WORD notUsed_0x13;
    BYTE mediaDescriptor;
    //WORD notUsed_0x16;
    WORD sectorsPerTrack;
    WORD numberOfHeads;
    DWORD hiddenSectors;
    //DWORD notUsed_0x20; 
    DWORD signature;
    long long int totalSectors;
    long long int clusterOffset_MFT; //vị trí bắt đầu của MFT tính bằng cluster -> phải đổi ra sector -> đổi ra byte
    long long int clusterOffset_MFTMirr;
    DWORD clustersPerFRS;
    BYTE clustersPerIndexBlock;
    //BYTE notUsed_0x45[3];
    long long int volumeSerialNumber; //hex
    DWORD checksum;
public:
    BPB(LPCWSTR  drive);
    void Print();
    long long int GetMFTOffset();
};

class Record
{
private:
    WORD flag;
    bool InUse;
    DWORD ID;
    DWORD ParentID;
    WORD SequenceNumber;
    WORD ParentSequenceNumber;
    unsigned short FileNameLength;
    char* FileName=NULL;

    int dataSize;
    char* Data = NULL;

    long long int FileSize = 0;

public:
    Record(BYTE buffer[1024]);
    ~Record();
    std::string dataString();
    void Print();
    long long int GetFileSize();
    DWORD getParentID();
    DWORD getID();
    std::string filename();
    int getSequenceNumber();
    int getParentSequenceNumber();
    bool isUsed();
    bool isDirectory();
};

class MFT
{
private:
    std::vector<Record*> recordList; //cai dau tien la $MFT
public:
    MFT(LPCWSTR drive, long long int clusterOffset);
    ~MFT();
    Record* createRecord(BYTE buffer[1024]);
    bool isRootDirectory(Record* record);
    Record* lookRecordID(DWORD id);
    std::string buildPath(Record* record);
    void PrintAll();
    void PrintPerDirectory(int folderID);
    Record* GetRecordByName(std::string name, int folderID);
    Record* getMFTRecord();
};


void OpenRecord(BPB& bpb, MFT& mft, Record* record);
bool checkIfTxtRecord(Record* record);
void OpenTxtFile(Record* record);
void Menu(BPB& bpb, MFT& mft);
void BPB_Menu(BPB& bpb, MFT& mft);
void Directory_Menu(BPB& bpb, MFT& mft, int currentParentID);
