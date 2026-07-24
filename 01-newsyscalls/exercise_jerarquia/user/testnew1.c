#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

#include "colors.h"

#define SYSCALL1	600

long int getnumchilds()
{
	return(syscall(SYSCALL1));
}

#define MAX_CHLD   5

void bye(int foo)
{
	signal(SIGTERM, SIG_IGN);
	kill(-getpid(), SIGTERM);
	printf(COLOR_BRED "Something failed!" COLOR_RESET "\n");
	exit(0);
}

int bomb()
{
	int i, n, pid;

	n = rand()%MAX_CHLD;

	for (i=0; i<n; i++) {
		assert((pid = fork())!=-1);
		if (pid == 0) pause();
	}
	
	return(n);
}

void iter()
{
	int ret;
	char s[100];

	ret = bomb();

	printf(COLOR_BYELLOW "Testing %d child processes" COLOR_RESET "\n", ret);

	sprintf(s, "pstree %d", getpid());
	system(s);

	assert (ret == getnumchilds());

	signal(SIGTERM, SIG_IGN);
	kill(-getpid(), SIGTERM);
	signal(SIGTERM, SIG_DFL);

	while(wait(NULL) > 0);
}

int main(int argc, char *argv[])
{
	signal(SIGABRT, bye);

	srand(getpid());

	assert(0 == getnumchilds());

	iter(); iter(); iter();
	iter(); iter(); iter();
	iter(); iter(); iter();

	assert(0 == getnumchilds());

	printf(COLOR_BGREEN "All tests seem to be OK!" COLOR_RESET "\n");
}
