#include <string.h>

#define MIN_PID 300
#define MAX_PID 5000
static const int MIN_BLK = MIN_PID / 32;
static const int MAX_BLK = MAX_PID / 32;

static unsigned int pid_aval[MAX_PID - MIN_PID + 1];

int allocate_map(void) {
    memset(pid_aval, 0, sizeof pid_aval);
}

int allocate_pid(void) {
    for (int i = MIN_PID; i <= MAX_PID; ++i) {
        int blk = i / 32, sft = i % 32;
        if (pid_aval[blk] & (1u << sft)) {
            continue;
        }
        pid_aval[blk] |= 1u << sft;
        return i;
    }
    return -1;
}

void release_pid(int pid) {
    int blk = pid / 32, sft = pid % 32;
    pid_aval[blk] |= (unsigned)(-1) ^ (1u << sft);
}

int main() {
    allocate_map();
    int x = allocate_pid();
    release_pid(x);
    return 0;
}
