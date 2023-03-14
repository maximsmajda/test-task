#include "folders_sync.h"

int main(int argc, char **argv)
{
	int interval;

	if (argc != 5)
	{
		printf("Invalid number of arguments, please enter source directory path, replica directory path, interval and log file path.\n");
		return 1;
	}

	interval = atoi(argv[3]);
	if (interval < 1)
	{
		printf("Invalid interval time. Please enter interval time in seconds.\n");
		return 1;
	}

	while (1)
	{
		if (sync_folders(argv[1], argv[2], argv[4]) != 0)
			return 1;
		sleep(interval);
	}
	return 0;
}

int sync_folders(char *src_path, char *rep_path, char *log_path)
{
	FILE *log_file;
	DIR *src_dir, *rep_dir;
	struct dirent *src_dirent, *rep_dirent;
	struct stat buffer_src_stat, buffer_rep_stat;
	char full_rep_path[PATH_MAX];
	char full_src_path[PATH_MAX];

	log_file = fopen(log_path, "a");
	if (log_file == NULL)
	{
		printf("Error: Failed to open log file.\n");
		return 1;
	}

	src_dir = opendir(src_path);
	if (src_dir == NULL)
	{
		fclose(log_file);
		print_message(log_file, "Error: Failed to open source directory", src_path);
		return 1;
	}

	rep_dir = opendir(rep_path);
	if (rep_dir == NULL)
	{
		// create replica folder if does not exist
		if (mkdir(rep_path, 0777) != 0)
		{
			print_message(log_file, "Error: Failed to create replica directory", rep_path);
			closedir(src_dir);
			return 1;
		}
		rep_dir = opendir(rep_path);
		if (rep_dir == NULL)
		{
			print_message(log_file, "Error: Failed to open replica directory", rep_path);
			closedir(src_dir);
			return 1;
		}
	}

	// copy or update files from source to replica
	while ((src_dirent = readdir(src_dir)) != NULL)
	{
		if (strcmp(".", src_dirent->d_name) == 0 || strcmp("..", src_dirent->d_name) == 0)
			continue;

		make_path(full_rep_path, rep_path, src_dirent->d_name);
		make_path(full_src_path, src_path, src_dirent->d_name);

		// if path is diretory, create in replica if does not exist and recurively browse subdirectory
		if (is_dir(full_src_path))
		{
			if (dir_not_exist(full_rep_path))
			{
				if (mkdir(full_rep_path, 0777) != 0)
				{
					print_message(log_file, "Error: Failed to create directory", full_rep_path);
					close_all(src_dir, rep_dir, log_file);
					return 1;
				}
				print_message(log_file, "Directory created", full_rep_path);
			}
			if (sync_folders(full_src_path, full_rep_path, log_path) != 0)
			{
				close_all(src_dir, rep_dir, log_file);
				return 1;
			}
		}
		// if path is not directory check if exists in replica folder
		else
		{
			// if exists compare time of modifing file
			if (stat(full_rep_path, &buffer_rep_stat) == 0)
			{
				stat(full_src_path, &buffer_src_stat);
				if (buffer_rep_stat.st_mtime < buffer_src_stat.st_mtime)
				{
					remove(full_rep_path);
					if (file_copy(full_src_path, full_rep_path, log_file) != 0)
					{
						close_all(src_dir, rep_dir, log_file);
						return 1;
					}
					print_message(log_file, "File updated in replica folder", full_rep_path);
				}
			}
			// else create file in replica folder
			else
			{
				// creation & copying
				if (file_copy(full_src_path, full_rep_path, log_file) != 0)
				{
					close_all(src_dir, rep_dir, log_file);
					return 1;
				}
				print_message(log_file, "File created in replica folder", full_rep_path);
			}
		}
	}

	// check replica folder for files or subdirectories that are not in source folder
	while ((rep_dirent = readdir(rep_dir)) != NULL)
	{
		if (strcmp(".", rep_dirent->d_name) == 0 || strcmp("..", rep_dirent->d_name) == 0)
			continue;

		make_path(full_rep_path, rep_path, rep_dirent->d_name);
		make_path(full_src_path, src_path, rep_dirent->d_name);

		// if path is directory
		if (is_dir(full_rep_path) && dir_not_exist(full_src_path))
		{
			if (delete_dir(full_rep_path, log_file) != 0)
			{
				close_all(src_dir, rep_dir, log_file);
				return 1;
			}
		}

		// if it is not directory
		else
		{
			if (access(full_src_path, F_OK) != 0)
			{
				if (remove(full_rep_path) != 0)
				{
					print_message(log_file, "Error: Failed to delete file from replica folder", full_rep_path);
					close_all(src_dir, rep_dir, log_file);
					return 1;
				}
				print_message(log_file, "File deleted from replica folder", full_rep_path);
			}
		}
	}

	close_all(src_dir, rep_dir, log_file);
	return 0;
}
