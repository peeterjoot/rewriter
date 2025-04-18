#include <stdio.h>

void foo_exit( int rc ) {
    printf( "exit: %d\n", rc );
}

int main() {
    foo_exit( 1 );

    return 0;
}
