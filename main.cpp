#include "headers.h"

using namespace std;

int main (int argc, char *argv[]) {

    TMSLSimulator *ts = new TMSLSimulator(); 

    switch (argc) {
        case 2: // appoint a lines file
            ts->readLines(argv[1]);
            printf("Finish read lines\n");
            ts->doClocks();
            printf("Finish do Clocks\n");
            ts->writeRecords();
            break;
        default:
            printf("Invalid argument number!\n");
            exit(-1);
    }
    return 0;
}
