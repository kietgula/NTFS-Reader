#include "Header.h"

void ReadRecord(LPCWSTR drive, long long int clusterOffset, BYTE record[1024])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        exit(1);
    }

    SetFilePointer(device, 0, NULL, FILE_BEGIN);//Set a Point to Read

    for (int i = 0; i < clusterOffset; i++)
        SetFilePointer(device, 4096, NULL, FILE_CURRENT);

    if (!ReadFile(device, record, 1024, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
        printf("Success!\n");
    }
}


BPB::BPB(LPCWSTR  drive)
{
    BYTE sector[512];

    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        exit(1);
    }

    SetFilePointer(device, 0, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
        printf("Success!\n");
    }

    memcpy(&bytesPerSector, &sector[11], sizeof(bytesPerSector));
    memcpy(&sectorsPerCluster, &sector[13], sizeof(sectorsPerCluster));
    memcpy(&reservedSectors, &sector[14], sizeof(reservedSectors));
    //memcpy(&always0_0x10, &sector[16], sizeof(always0_0x10));
    memcpy(&mediaDescriptor, &sector[21], sizeof(mediaDescriptor));
    //memcpy(&output.notUsed_0x16, &sector[22], sizeof(notUsed_0x16));
    memcpy(&sectorsPerTrack, &sector[24], sizeof(sectorsPerTrack));
    memcpy(&numberOfHeads, &sector[26], sizeof(numberOfHeads));
    memcpy(&hiddenSectors, &sector[28], sizeof(hiddenSectors));
    //memcpy(&notUsed_0x20, &sector[32], sizeof(notUsed_0x20));
    memcpy(&signature, &sector[36], sizeof(signature));
    memcpy(&totalSectors, &sector[40], sizeof(totalSectors));
    memcpy(&clusterOffset_MFT, &sector[48], sizeof(clusterOffset_MFT));
    memcpy(&clusterOffset_MFTMirr, &sector[56], sizeof(clusterOffset_MFTMirr));
    memcpy(&clustersPerFRS, &sector[64], sizeof(clustersPerFRS));
    memcpy(&clustersPerIndexBlock, &sector[68], sizeof(clustersPerIndexBlock));
    memcpy(&volumeSerialNumber, &sector[72], sizeof(volumeSerialNumber));
    memcpy(&checksum, &sector[80], sizeof(checksum));
}
void BPB::Print()
{
    std::cout << "Bytes per Sector: " << std::dec << bytesPerSector << std::endl;
    std::cout << "Sectors per Cluster: " << std::hex << (int)sectorsPerCluster << std::endl;
    std::cout << "Reserved sectors: " << reservedSectors << std::endl;
    std::cout << "Media descriptor: " << std::dec << (int)mediaDescriptor << std::endl;
    std::cout << "Sectors per track: " << std::dec << sectorsPerTrack << std::endl;
    std::cout << "Number of heads: " << std::dec << numberOfHeads << std::endl;
    std::cout << "Hidden sectors: " << hiddenSectors << std::endl;
    std::cout << "Signature: " << signature << std::endl;
    std::cout << "Total sectors: " << totalSectors << std::endl;
    std::cout << "$MFT cluster number: " << clusterOffset_MFT << std::endl;
    std::cout << "$MFTMirr cluster number: " << clusterOffset_MFTMirr << std::endl;
    std::cout << "Clusters per File Record Segment: " << clustersPerFRS << std::endl;
    std::cout << "Clusters Index Block: " << clustersPerIndexBlock << std::endl;
    std::cout << "Volume serial number: " << std::hex << volumeSerialNumber << std::endl;
    std::cout << "Checksum: " << checksum << std::endl;
}

long long int BPB::GetMFTOffset()
{
    return clusterOffset_MFT;
}


Record::Record(BYTE buffer[1024]) //chỉ trích xuất tên file, loại file (thư mục hay file) và thư mục cha. Cái này dùng để xây dựng cây thư mục được
{
    memcpy(&SequenceNumber, &buffer[16], sizeof(SequenceNumber));
    memcpy(&flag, &buffer[22], sizeof(flag));
    memcpy(&ID, &buffer[44], sizeof(ID));

    int readPoint = 20;
    int currentAttributeLength;
    memcpy(&readPoint, &buffer[readPoint], 2); //offset cua attribute dau tien duoc luu trong readPoint
    DWORD currentAttributeID = 0;
    DWORD filenameAttributeID = 0x30;
    bool isFilenameAttribute = false;
    int count = 0; //giup vong lap ko bi treo

    while (!isFilenameAttribute && (int)currentAttributeID <= 0x30) //tim attribute Filename 0x30
    {
        memcpy(&currentAttributeID, &buffer[readPoint], 4);
        if (memcmp(&currentAttributeID, &filenameAttributeID, 4) == 0)
        {
            isFilenameAttribute = true;
        }
        else //nhảy tới attribute ke tiep
        {
            memcpy(&currentAttributeLength, &buffer[readPoint + 4], 4);
            readPoint = readPoint + currentAttributeLength;
            count++;
        }
    }

    if (isFilenameAttribute)
    {
        memcpy(&ParentID, &buffer[readPoint + 24], 4);
        memcpy(&ParentSequenceNumber, &buffer[readPoint + 26], 4);
        memcpy(&FileNameLength, &buffer[readPoint + 88], 1);
        FileName = new char[FileNameLength * 2]; //*2 vì tên nó dùng 2 byte cho 1 kí tự
        memcpy(FileName, &buffer[readPoint + 90], FileNameLength * 2);
    }


    WORD folderFlag = 3;
    count = 0;
    DWORD dataAttributeID = 0x80;
    bool isDataAttribute = false;
    while (!isDataAttribute && (int)currentAttributeID <= 0x80) //tim attribute Data 0x80
    {

        memcpy(&currentAttributeID, &buffer[readPoint], 4);
        if (memcmp(&currentAttributeID, &dataAttributeID, 4) == 0)
        {
            isDataAttribute = true;
        }
        else //nhảy tới attribute ke tiep
        {
            memcpy(&currentAttributeLength, &buffer[readPoint + 4], 4);
            readPoint = readPoint + currentAttributeLength;
            count++;
        }
    }
    if (isDataAttribute)
    {
        memcpy(&FileSize, &buffer[readPoint + 48], 8);

        unsigned short dataOffset = 0;
        memcpy(&dataOffset, &buffer[readPoint + 10], 2);
        BYTE endMark = 0xFF;
        int dataSizeCount = 0;
        while (memcmp(&buffer[readPoint + dataOffset + dataSizeCount], &endMark, 1) != 0)
        {
            dataSizeCount++;  
        }
        Data = new char[dataSizeCount];

        memcpy(Data, &buffer[readPoint + dataOffset], dataSizeCount);
        dataSize = dataSizeCount;
    }
}

Record::~Record()
{
    if (FileName!=NULL)
        delete[]FileName;
    if (Data!=NULL)
        delete[]Data;

}

std::string Record::dataString()
{
    std::string output="";
    if (Data != NULL)
    {
        for (int i = 0; i < dataSize; i++)
            output = output + Data[i];
    }
    return output;
}

void Record::Print()
{
    std::cout << "ten file: ";
    if (FileName != NULL)
        for (int i = 0; i < FileNameLength * 2; i++)
            std::cout << FileName[i];
    std::cout << std::endl;

    std::cout << "kich file: " << std::dec << FileSize << std::endl;

}

long long int Record::GetFileSize()
{
    return FileSize;
}

DWORD Record::getParentID()
{
    return ParentID;
}

DWORD Record::getID()
{
    return ID;
}

std::string Record::filename()
{
    std::string output;

    for (int i = 0; i < FileNameLength * 2; i++)
        output = output + FileName[i];

    return output;
}

int Record::getSequenceNumber()
{
    return SequenceNumber;
}

int Record::getParentSequenceNumber()
{
    return ParentSequenceNumber;
}

bool Record::isUsed()
{
    if (flag & 0x01)
    {
        return true;
    }
    return false;
}

bool Record::isDirectory()
{
    if (flag & 0x02)
    {
        return true;
    }
    return false;
}


MFT::MFT(LPCWSTR drive, long long int clusterOffset)
    {
        BYTE record[1024];
        int retCode = 0;
        DWORD bytesRead;
        HANDLE device = NULL;

        device = CreateFile(drive,    // Drive to open
            GENERIC_READ,           // Access mode
            FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
            NULL,                   // Security Descriptor
            OPEN_EXISTING,          // How to create
            0,                      // File attributes
            NULL);                  // Handle to template

        if (device == INVALID_HANDLE_VALUE) // Open Error
        {
            printf("CreateFile: %u\n", GetLastError());
            exit(1);
        }

        SetFilePointer(device, 0, NULL, FILE_BEGIN);//Set a Point to Read

        for (int i = 0; i < clusterOffset; i++)
            SetFilePointer(device, 4096, NULL, FILE_CURRENT);

        if (!ReadFile(device, record, 1024, &bytesRead, NULL))
        {
            printf("ReadFile: %u\n", GetLastError());
        }
        else
        {
            printf("Success!\n");
        }

        Record mft(record); //dua record $mft raw vao trong class record de no xu ly tim kich thuoc mft

        long long int numOfRecord = mft.GetFileSize() / 1024;
        int count = 0;
        BYTE SignatureFILE[4];

        memcpy(SignatureFILE, record, 4);

        //recordList.push_back(mft); //cho $mft vào list trước
        //recordList[0].Print();
        SetFilePointer(device, -1024, NULL, FILE_CURRENT);

        for (long long int i = 0; i < numOfRecord; i++)
        {
            ReadFile(device, record, 1024, &bytesRead, NULL);
            if (memcmp(SignatureFILE, record, 4) == 0)
            {
                recordList.push_back(createRecord(record));
            }
        }
    }

MFT::~MFT()
    {
        for (int i = 0; i < recordList.size(); i++)
            delete recordList[i];
    }



Record* MFT::createRecord(BYTE buffer[1024])
{
    Record* newRecord = new Record(buffer);
    return newRecord;
}

bool MFT::isRootDirectory(Record* record)
{
    if (this->recordList[0]->getParentID() != record->getParentID()) //check xem cái parentID của $MFT có trùng với parrentID của record đang xét
        return false; //ko trùng -> đây không phải là tệp ở thư mục gốc
    return true;      //trùng -> đây là tệp ở thư mục gốc
}

Record* MFT::lookRecordID(DWORD id)
{
    for (int i = 0; i < recordList.size(); i++)
        if (recordList[i]->getID() == id)
            return recordList[i];
    return NULL;
}

std::string MFT::buildPath(Record* record)
{
    std::string path = "";
    Record* currentRecord = record;

    while (!isRootDirectory(currentRecord))
    {
        path = "\\" + currentRecord->filename() + path;
        currentRecord = this->lookRecordID(currentRecord->getParentID());
        int sequence_number = currentRecord->getSequenceNumber();

        if (!currentRecord->isUsed())
        {
            sequence_number--;
            if (sequence_number != currentRecord->getParentSequenceNumber())
                path = "\\$OrphanFiles" + path;
        }
    }

    if (isRootDirectory(currentRecord))
    {
        path = "\\" + currentRecord->filename() + path;
        int sequence_number = currentRecord->getSequenceNumber();

        if (!currentRecord->isUsed())
        {
            sequence_number--;
            if (sequence_number != currentRecord->getParentSequenceNumber())
                path = "\\$OrphanFiles" + path;
        }
    }
    return "." + path;
}

void MFT::PrintAll()
{
    for (int i = 0; i < recordList.size(); i++)
        std::cout << buildPath(recordList[i]) << std::endl;
}

void MFT::PrintPerDirectory(int folderID)
{
    for (int i = 0; i < recordList.size(); i++)
    {
        if (recordList[i]->getParentID() == folderID)
            std::cout << buildPath(recordList[i]) <<" "<< recordList[i]->getID() << std::endl;
    }
}

Record* MFT::GetRecordByName(std::string name, int folderID)
{
    for (int i = 0; i < recordList.size(); i++)
    {
        if (recordList[i]->filename() == name && recordList[i]->getParentID() == folderID)
            return recordList[i];
    }

    return NULL;
}

Record* MFT::getMFTRecord()
{
    return recordList[0];
}

void OpenRecord(BPB& bpb, MFT& mft, Record* record)
{
    if (record->isDirectory())
        Directory_Menu(bpb, mft, record->getID());
    else if (checkIfTxtRecord(record))
        OpenTxtFile(record);
    else 
        std::cout << "Dung phan mem tuong thich de doc noi dung." << std::endl;
}

bool checkIfTxtRecord(Record* record)
{
    std::string filename = record->filename();
    if (filename.size() <= 8)
        return false;

    WORD dot = 0x002E;
    WORD t = 0x0074;
    WORD x = 0x0078;
    if (memcmp(&filename[filename.size() - 2], &t, 1) != 0
        && memcmp(&filename[filename.size() - 4], &x, 1) != 0
        && memcmp(&filename[filename.size() - 6], &t, 1) != 0
        && memcmp(&filename[filename.size() - 8], &dot, 1) != 0)
        return false;
    return true;
}

void OpenTxtFile(Record* record)
{
    std::cout << "Noi dung txt: " << record->dataString() << std::endl;
}

void Menu(BPB& bpb, MFT& mft)
{
    std::cout << "1. Thong tin phan vung NTFS" << std::endl;
    std::cout << "2. Truy cap thu muc goc" << std::endl;
    std::cout << "#. Thoat khoi chuong trinh" << std::endl;

    std::cout << "Lua chon: ";
    int choose;
    std::cin >> choose;
    if (choose == 1)
        BPB_Menu(bpb, mft);
    else if (choose == 2)
        Directory_Menu(bpb, mft, mft.getMFTRecord()->getParentID());
    else exit(0);

}

void BPB_Menu(BPB& bpb, MFT& mft)
{
    bpb.Print();

    std::cout << "Nhan bat ki de quay lai";
    std::string handle;
    std::cin >> handle;

    Menu(bpb, mft);
}

void Directory_Menu(BPB& bpb, MFT& mft, int currentParentID)
{
    int handle;

    mft.PrintPerDirectory(currentParentID); //in ra thu muc
    std::cout << "Nhap ID tep tin de mo, nhap -1 se ve Menu: ";
    std::cin >> handle;

    if (handle==-1)
        Menu(bpb, mft);
    else
        OpenRecord(bpb, mft, mft.lookRecordID(handle));
}
