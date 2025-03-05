#include <sodium.h>
#include <stdio.h>

int main() {
    if (sodium_init() < 0) {
        printf("sodium_init() failed\n");
        return 1;
    }
    printf("sodium_init() succeeded\n");
    return 0;
}