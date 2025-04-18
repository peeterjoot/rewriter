#include <stdio.h>
#include <stdlib.h>

void foo_exit( int rc );

int main() {
    exit( 1 );

    return 0;
}

void foo_exit( int rc ) {
    printf( "exit: %d\n", rc );
}
