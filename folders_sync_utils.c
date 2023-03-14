#include "folders_sync.h"

void make_path(char *full_path, char *src_path, char *file_name)
{
	bzero(full_path, PATH_MAX);
	strcpy(full_path, src_path);
	full_path = strcat(strcat(full_path, "/"), file_name);
}

int is_dir(char *path)
{
	struct stat buffer_stat;
	stat(path, &buffer_stat);
	return S_ISDIR(buffer_stat.st_mode);
}

int dir_not_exist(char *path)
{
	DIR *dir = opendir(path);
	if (dir)
	{
		closedir(dir);
		return 0;
	}
	else if (ENOENT == errno)
		return 1;
	return 0;
}

int file_copy(char *src_path, char *rep_path, FILE *log_file)
{
	int src_fd, rep_fd, n, err;
	unsigned char buffer[COPY_BUFFER_SIZE];

	src_fd = open(src_path, O_RDONLY);
	rep_fd = open(rep_path, O_WRONLY | O_CREAT, 0666);

	while (1)
	{
		err = read(src_fd, buffer, COPY_BUFFER_SIZE);
		if (err == -1)
		{
			print_message(log_file, "Error reading file", src_path);
			return 1;
		}
		n = err;

		if (n == 0)
			break;

		err = write(rep_fd, buffer, n);
		if (err == -1)
		{
			print_message(log_file, "Error: Failed writing to file", rep_path);
			return 1;
		}
	}

	close(src_fd);
	close(rep_fd);
	return 0;
}

int delete_dir(char *path, FILE *log_file)
{
	DIR *d;
	struct dirent *d_dirent;
	char full_path[PATH_MAX];

	d = opendir(path);
	if (d == NULL)
	{
		print_message(log_file, "Error: Failed to remove directory", path);
		return 1;
	}

	while ((d_dirent = readdir(d)) != NULL)
	{
		make_path(full_path, path, d_dirent->d_name);
		if (is_dir(full_path))
		{
			if (strcmp(".", d_dirent->d_name) == 0 || strcmp("..", d_dirent->d_name) == 0)
				continue;

			if (delete_dir(full_path, log_file) != 0)
			{
				closedir(d);
				return 1;
			}
		}
		else
		{
			if (remove(full_path) != 0)
			{
				print_message(log_file, "Error: Failed to delete file", full_path);
				closedir(d);
				return 1;
			}

			print_message(log_file, "File deleted", full_path);
		}
	}
	closedir(d);

	if (rmdir(path) != 0)
	{
		print_message(log_file, "Error: Failed to remove director", path);
		return 1;
	}
	print_message(log_file, "Directory deleted", path);

	return 0;
}

char *get_time_str()
{

	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	static char current_time[20];
	strftime(current_time, sizeof(current_time), "%Y-%m-%d %H:%M:%S", timeinfo);

	return current_time;
}

void print_message(FILE *log_file, char *message, char *path)
{
	char *now = get_time_str();
	printf("%s %s: %s\n", now, message, path);
	fprintf(log_file, "%s %s: %s\n", now, message, path);
}

void close_all(DIR *src_dir, DIR *rep_dir, FILE *log_file)
{
	fclose(log_file);
	closedir(src_dir);
	closedir(rep_dir);
}