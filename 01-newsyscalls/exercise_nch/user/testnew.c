#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>

#define NTESTS 	10
#define MAXCH	20
int main() {
	int i;

	printf("✅ OK  ❌ ERROR\n");

#if 0
	int maxch = sysconf(_SC_OPEN_MAX);
	srand(getpid());
	for (i=0, i<NTESTS; i++) {
		int n = rand() % maxch;

		switch (fork()){
			case -1 : error(1, errno, "fork");

			case 0: int k = ran

		}
	}
#endif
}
