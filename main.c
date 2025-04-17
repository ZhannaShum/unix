#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#define TRUNCATED_FILESIZE                                                 (4 * 1024 * 1024 + 1)
#define DEFAULT_BLOCK_SIZE                                                                  4096
#define IS_TWO_PARAMETERS                                                  ((optind + 1) < argc)
#define IS_TWO_FILES           ((file0.type == FILETYPE_FILENAME) && (file0.type == file1.type))

enum {
	FILETYPE_FILENAME,
	FILETYPE_STDIN,
	N_FILETYPE
};

struct file_arg {
	uint32_t type;
	char *filename;
	int fd;
};

static int is_page_is_null (const uint8_t *b, uint64_t sz)
{
	for (int i = 0; i < sz; i++) {
		if (b[i] != 0)
			return 0;
	}

	return 1;
}

static void print_help ()
{
	fprintf (stderr, "./app [-b block_size] [input filename] output_file_name\n");
}

int main (int argc, char **argv)
{
	int opt;

	uint32_t block_size = DEFAULT_BLOCK_SIZE;

	struct file_arg file0;
	struct file_arg file1;

	while ((opt = getopt (argc, argv, "hb:")) != -1) {
		switch (opt) {
			case 'h':
				print_help ();
				exit (EXIT_SUCCESS);
			case 'b':
				block_size = atoi (optarg);
				break;
		}
	}

	if (optind >= argc) {
		print_help ();
		exit (EXIT_FAILURE);
	}

	if (IS_TWO_PARAMETERS) {
		file0.type = FILETYPE_FILENAME;
		file0.filename = argv[optind++];
		file1.type = FILETYPE_FILENAME;
		file1.filename = argv[optind];
	} else {
		file0.type = FILETYPE_STDIN;
		file0.filename = NULL;
		file1.type = FILETYPE_FILENAME;
		file1.filename = argv[optind];
	}

	if (IS_TWO_FILES) {
		file0.fd = open (file0.filename, O_RDONLY, 0);
		file1.fd = open (file1.filename, O_WRONLY | O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);

	} else if (file0.type == FILETYPE_STDIN) {
		file0.fd = STDIN_FILENO;
		file1.fd = open (file1.filename, O_WRONLY | O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
	} else {
		print_help ();
		exit (EXIT_FAILURE);
	}

	int ret = ftruncate (file1.fd, TRUNCATED_FILESIZE);
	if (ret == -1) {
		perror ("truncate");
		return -1;
	}


	off_t cur_offset = 0L;

	uint8_t bytes[block_size];

	while (1) {
		int ret = read (file0.fd, bytes, block_size);
		if (ret <= 0)
			break;

		int retseek = lseek (file1.fd, cur_offset, SEEK_SET);
		if (retseek == -1) {
			perror ("lseek");
			exit (EXIT_FAILURE);
		}

		if (!is_page_is_null (bytes, ret)) {
			write (file1.fd, bytes, ret);
		}
		cur_offset += block_size;
	}

	close (file0.fd);
	close (file1.fd);
}
