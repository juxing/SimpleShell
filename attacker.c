#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    sleep(1);
    printf("pwd\n");
    
    sleep(1);
    printf("cd -\n");

    sleep(1);
    printf("cd .\n");

    sleep(1);
    printf("cd /\n");

    sleep(1);
    printf("cd ..\n");

    sleep(1);
    printf("cd /home/mzhang8\n");

    sleep(1);
    printf("cd\n");

    sleep(1);
    printf("cd -\n");

    sleep(1);
    printf("cd hello\n");

    sleep(1);
    printf("history hello\n");

    sleep(1);
    printf("!4\n");

    sleep(1);
    printf("!4000\n");

    sleep(1);
    printf("!0\n");

    sleep(1);
    printf("!-2\n");

    sleep(1);
    printf("!a3b\n");

    sleep(1);
    printf("/bin/ls -al\n");

    sleep(1);
    printf("/bin/lsa\n");

    sleep(1);
    printf("ps\n");

    sleep(1);
    printf("  ls    -al  \t\n");

    sleep(1);
    printf("ps -kajdfkljak\n");

    sleep(1);
    printf("pushd\n");

    sleep(1);
    printf("popd\n");

    sleep(1);
    printf("popd\n");

    sleep(1);
    printf("dirs\n");

    sleep(1);
    printf("exit\n");

    return 0;
}
