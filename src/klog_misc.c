#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/syscall.h>
#include <sys/syslog.h>

void klog_driver(const char *driver_name)
{
	int length;
	int klog_size;
	char *klog_buff;
	size_t curr_len;
	const char *curr_line, *next_line;

	klog_size = klogctl(10, NULL, 0);
	klog_buff = calloc(1, klog_size);
	if (!klog_buff) {
		perror("failed to allocate memory for klog buffer");
		goto out;
	}

	length = klogctl(3, klog_buff, klog_size);
	if (length == -1) {
		perror("klog");
		goto out;
	}

	curr_line = klog_buff;
	while (curr_line) {
		next_line = strchr(curr_line, '\n');
		curr_len = next_line ? (size_t)(next_line - curr_line) :
				       strlen(curr_line);
		char *tmp = malloc(curr_len + 1);
		if (tmp) {
			memcpy(tmp, curr_line, curr_len);
			tmp[curr_len] = '\0';

			if (strstr(tmp, driver_name))
				printf("%s\n", tmp);

			free(tmp);
		}
		curr_line = next_line ? (next_line + 1) : NULL;
	}

out:
	free(klog_buff);
}
