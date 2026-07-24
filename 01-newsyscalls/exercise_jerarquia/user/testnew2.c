#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "colors.h"

#define MAX_LEVELS 3
#define MAX_CHLD   3

#define SYSCALL2	601

long int getnumdesdendents(int level)
{
	return(syscall(SYSCALL2, level));
}

int fd[2];

void bye(int foo)
{
	signal(SIGTERM, SIG_IGN);
	kill(-getpid(), SIGTERM);
	printf(COLOR_BRED "Something failed!" COLOR_RESET "\n");
	exit(0);
}

void bomb(int lev)
{
	if (lev <= MAX_LEVELS) {
		int i, n, pid;

		srand(getpid());
		n = 1 + rand()%MAX_CHLD;

		for (i=0; i<n; i++) {
			assert((pid = fork())!=-1);
			if (pid == 0) bomb(lev+1);
			assert(write(fd[1], &lev, sizeof(lev)) == sizeof(lev));
		}
	}

	if (lev > 0) {
		assert(close(fd[1]) == 0);
		pause();
	}
}

void iter()
{
	int i, lev, n;

	assert(pipe(fd) == 0);

	bomb(0);

	int ndesc[MAX_LEVELS+1];
	memset(ndesc, 0, sizeof(ndesc));

	assert(close(fd[1])==0);
	while((n=read(fd[0], &lev, sizeof(lev)))==sizeof(lev)) 
		ndesc[lev]++;
	assert(n==0);
	assert(close(fd[0])==0);

	printf(COLOR_BYELLOW "Testing a random tree with ");
	for (i=0; i<=MAX_LEVELS; i++)
		printf("%d ", ndesc[i]);
	printf("processes at each level" COLOR_RESET "\n");

	char s[100];
	sprintf(s, "pstree %d", getpid());
	system(s);

	for (i=0; i<=MAX_LEVELS; i++) {
//		printf("%d:%ld\n", i, getnumdesdendents(i));
		assert (ndesc[i] == getnumdesdendents(i));
	}
	assert (0 == getnumdesdendents(MAX_LEVELS+1));

	signal(SIGTERM, SIG_IGN);
	kill(-getpid(), SIGTERM);
	signal(SIGTERM, SIG_DFL);
	while(wait(NULL) > 0);
}

int main(int argc, char *argv[])
{
	signal(SIGABRT, bye);

	assert((getnumdesdendents(-3) == -1) && (errno == EINVAL)); 
	assert(getnumdesdendents(0) == 0);
	assert(getnumdesdendents(1) == 0);
	assert(getnumdesdendents(2) == 0);
	assert(getnumdesdendents(3) == 0);
	assert(getnumdesdendents(4+rand()%20) == 0);

	iter(); iter();

	assert(getnumdesdendents(0) == 0);
	assert(getnumdesdendents(1) == 0);
	assert(getnumdesdendents(2) == 0);
	assert(getnumdesdendents(3) == 0);
	assert(getnumdesdendents(4+rand()%20) == 0);

	printf(COLOR_BGREEN "All tests seem to be OK!" COLOR_RESET "\n");
}
