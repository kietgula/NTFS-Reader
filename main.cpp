#include "Header.h"


int main()
{
    //prepare Menu
    BPB bpb(L"\\\\.\\E:");
    MFT mft(L"\\\\.\\E:", bpb.GetMFTOffset());

    while (true)
        Menu(bpb, mft);
    return 0;
}