#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_DATA 4096

const char banner[] =
"scl [options] [file]?\n"
"	Read and load shellcode from [file] or stdin\n\n"

"	-h         help\n"
"	-u [euid]  set effective user id\n"
"	-g [egid]  set effective group id\n";

// Load data from stdin
static ssize_t _load_stdin(uint8_t data[MAX_DATA]);
// Load data from a file
static ssize_t _load_file(uint8_t data[MAX_DATA], const char *path);

int main(int argc, char **argv)
{
	int opt;
	int ret = 0;

	ssize_t size;

	uid_t euid = geteuid();
	gid_t egid = getegid();

	uint8_t data[MAX_DATA];

	while ((opt = getopt(argc, argv, ":hu:g:")) != -1) {
		switch (opt) {
			case 'h':
				fputs(banner, stderr);
				return EXIT_SUCCESS;

                        case 'u':
                                euid = (uid_t)strtoul(optarg, NULL, 10);
                                break;

                        case 'g':
                                egid = (gid_t)strtoul(optarg, NULL, 10);
                                break;

			case ':':
				fprintf(
					stderr,
					"[scl] Option '%c' requires an argument\n", optopt
				);

				return EXIT_FAILURE;

			case '?':
				ret = EXIT_FAILURE;
				fprintf(stderr, "[scl] Invalid option '%c'\n", optopt);

				return EXIT_FAILURE;
		}
	}

	if (optind < argc - 1) {
		fputs(banner, stderr);
		return EXIT_FAILURE;
	}

	if (euid != getuid() && setuid(euid) < 0) {
		perror("setuid");
		return EXIT_FAILURE;
	}

	if (egid != getgid() && setgid(egid) < 0) {
		perror("setgid");
		return EXIT_FAILURE;
	}

	if ((size = (optind < argc) ? _load_file(data, argv[optind]) : _load_stdin(data)) < 0)
		return EXIT_FAILURE;

	if (! size) {
		fputs("[scl] Read 0 bytes\n", stderr);
		return EXIT_FAILURE;
	}

	if (size == MAX_DATA) {
		fprintf(
			stderr,
			"[scl] Read size equals maximum %i (input may have been truncated)\n",
			MAX_DATA
		);
	}

	((void (*)(void))data)();

	return ret;
}

static ssize_t _load_stdin(uint8_t data[MAX_DATA])
{
	ssize_t size;

	if ((size = fread(data, 1, MAX_DATA, stdin)) < 0)
		perror("fread");

	return size;
}

static ssize_t _load_file(uint8_t data[MAX_DATA], const char *path)
{
	FILE *fp;
	ssize_t size;

	if (! (fp = fopen(path, "rb"))) {
		perror("fopen");
		return 0;
	}

	if ((size = fread(data, 1, MAX_DATA, fp)) < 0) {
		perror("fread");
		return size;
	}

	fclose(fp);

	return size;
}
