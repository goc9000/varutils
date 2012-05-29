#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#define MAX_FDS 256
#define BUF_SIZE 4096

int main(int argc, char** argv)
{
	int fds[MAX_FDS];
	int n_fds, n_poll;
	int i;
	struct pollfd poll_fds[MAX_FDS];
	char buf[BUF_SIZE];
	int count;
	
	if (argc <= 1) {
		printf(
			"eet\n"
			"Combines data from pipes and outputs to stdout\n"
			"Usage: %s <pipe1> <pipe2> ...\n"
			"(use '-' for stdin)\n",
			argv[0]
		);
		
		return 0;
	}
	
	n_fds = argc-1;
	if (n_fds > MAX_FDS) {
		fprintf(stderr, "%s", "Too many input streams !\n");
		
		return -1;
	}
	
	for (i=0; i<n_fds; i++) {
		if (!strcmp(argv[1+i], "-")) {
			fds[i] = 0;
		} else {
			fds[i] = open(argv[1+i], O_RDONLY);
		}
		
		if (fds[i] < 0) {
			fprintf(stderr, "Can't open stream %s: ", argv[1+i]);
			perror("");
			
			return -1;
		}
	}
	
	while (1) {
		n_poll = 0;
		for (i=0; i<n_fds; i++)
			if (fds[i] >= 0) {
				poll_fds[n_poll].fd = fds[i];
				poll_fds[n_poll].events = POLLIN;
				n_poll++;
			}
		
		if (n_poll == 0) {
			break;
		}
		
		if (poll(poll_fds, n_poll, -1) < 0) {
			break;
		}
		
		for (i=0; i<n_fds; i++) {
			if (poll_fds[i].revents & POLLIN) {
				count = read(poll_fds[i].fd, buf, BUF_SIZE);
				if (count <= 0) {
					fds[i] = -1;
				} else {
					write(1, buf, count);
				}
			}
		}
	}

	return 0;
}
