#include "headers.h"

using namespace std;

int main (int argc, char *argv[]) {

    TMSLSimulator *ts = new TMSLSimulator(); 

    switch (argc) {
        case 2: // appoint a lines file
            ts->readLines(argv[1]);
            ts->doClocks();


        default:
            printf("Invalid argument number!\n");
            exit(-1);
    }
    return 0;
}
