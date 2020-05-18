#include "headers.h"

using namespace std;
using namespace std::chrono;

time_point<steady_clock> start;

long long getTime() {
    // time_point<high_resolution_clock> end;
    time_point<steady_clock> end;
    double duration;
    // end = high_resolution_clock::now();//获取当前时间
    end = steady_clock::now();
	auto dur = duration_cast<nanoseconds>(end - start);
	duration = double(dur.count()) * nanoseconds::period::num / nanoseconds::period::den * 1000000000;
    return (long long)duration;
}

int main (int argc, char *argv[]) {

    TMSLSimulator *ts = new TMSLSimulator(); 

    switch (argc) {
        case 2: // appoint a lines file
            ts->readLines(argv[1]);
            printf("Finish read lines\n");
            start = steady_clock::now();
            ts->doClocks();
            printf("Total time cost is %llf\n", (double)getTime()/1000000000);
            printf("Finish do Clocks\n");
            ts->writeRecords();
            break;
        case 3:
            ts->readLines(argv[1]);
            printf("Finish read lines\n");
            if (argv[2][0] == 'P') {
                ts->performance = true;
                start = steady_clock::now();
                ts->doClocks();
                printf("Total time cost is %llf\n", (double)getTime()/1000000000);
            } else if (argv[2][0] == 'C') {
                sscanf(&argv[2][1], "%d", &ts->t_clock);
                if (ts->t_clock < 1) {
                    printf("The searched clock is less than 1\n");
                    exit(-1);
                }
                ts->search_clock = true;
                start = steady_clock::now();
                ts->doClocks();
                printf("Total time cost is %llf\n", (double)getTime()/1000000000);
            } else {
                printf("Invalid arg 2\n");
                exit(-1);
            }
            printf("Finish do Clocks\n");
            if (!ts->performance) ts->writeRecords();
            break;
        default:
            printf("Invalid argument number!\n");
            exit(-1);
    }
    return 0;
}
