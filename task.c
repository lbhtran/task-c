#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_ACTIVE_TASKS 3
#define MAX_LENGTH 121
#define FILE_NAME "tasks.txt"

typedef struct {
  char desc[MAX_LENGTH];
  bool archived;
} Task;

Task *tasks = NULL;
int task_count = 0;

char *get_task_file_path() {
  const char *home = getenv("HOME"); // gets your home directory
  if (!home)
    home = "."; // fallback

  static char path[512];
  snprintf(path, sizeof(path), "%s/.taskc", home);

  // create ~/.taskc if it doesn’t exist
  mkdir(path, 0755);

  // append /tasks.txt
  strncat(path, "/" FILE_NAME, sizeof(path) - strlen(path) - 1);

  return path;
}

int count_active_tasks() {

  int active_count = 0;
  for (int i = 0; i < task_count; i++) {
    if (!tasks[i].archived) {
      active_count++;
    }
  }
  return active_count;
}

int resize_tasks(int new_size) {
  Task *new_tasks = realloc(tasks, new_size * sizeof(Task));
  if (!new_tasks && new_size > 0) {
    printf("Error: could not allocate memory.\n");
    return 0; // fail
  }
  tasks = new_tasks;
  return 1; // success
}

// Save tasks to file
void save_tasks() {
  const char *file_path = get_task_file_path();
  FILE *file = fopen(file_path, "w");
  if (!file) {
    printf("Error saving tasks!\n");
    return;
  }
  for (int i = 0; i < task_count; i++) {
    fprintf(file, "%d|%s\n", tasks[i].archived, tasks[i].desc);
  }
  fclose(file);
}

// Load tasks from file
void load_tasks() {
  const char *file_path = get_task_file_path();
  FILE *file = fopen(file_path, "r");
  if (!file)
    return; // no file yet, skip

  task_count = 0;
  free(tasks);
  tasks = NULL;

  char line[MAX_LENGTH + 10];

  char fmt[32];
  snprintf(fmt, sizeof(fmt), "%%d|%%%d[^\n]", MAX_LENGTH - 1);

  while (fgets(line, sizeof(line), file)) {
    int archived;
    char text[MAX_LENGTH];
    if (sscanf(line, fmt, &archived, text) == 2) {
      if (resize_tasks(task_count + 1))
        break;

      strncpy(tasks[task_count].desc, text, MAX_LENGTH);
      tasks[task_count].desc[MAX_LENGTH - 1] = '\0';
      tasks[task_count].archived = archived;
      task_count++;
    }
  }
  fclose(file);
}

void add_task() {
  if (count_active_tasks() >= MAX_ACTIVE_TASKS) {
    printf("Task list is full! (max %d)\n", MAX_ACTIVE_TASKS);
    return;
  }
  printf("Enter task (max %d chars): ", MAX_LENGTH);
  getchar(); // consume leftover newline

  char buffer[1024];
  fgets(buffer, sizeof(buffer), stdin);
  buffer[strcspn(buffer, "\n")] = '\0';

  if (strlen(buffer) > MAX_LENGTH) {
    printf("Warning: Task is too long (max %d chars). Task not added.\n",
           MAX_LENGTH);
    return;
  }

  if (!resize_tasks(task_count + 1))
    return;

  strncpy(tasks[task_count].desc, buffer, MAX_LENGTH);
  tasks[task_count].desc[MAX_LENGTH - 1] = '\0';
  tasks[task_count].archived = false;
  task_count++;

  save_tasks();
  printf("Task added!\n");
}

void list_tasks() {
  printf("\n--- Active Tasks ---\n");
  for (int i = 0; i < task_count; i++) {
    if (!tasks[i].archived) {
      printf("[%d] %s\n", i, tasks[i].desc);
    }
  }
  printf("-------------------\n");
}

void list_archived_tasks() {
  printf("\n--- Archived Tasks ---\n");
  for (int i = 0; i < task_count; i++) {
    if (tasks[i].archived) {
      printf("[%d] %s\n", i, tasks[i].desc);
    }
  }
  printf("---------------------\n");
}

void archive_task() {
  list_tasks();
  if (count_active_tasks() == 0) {
    printf("No tasks to archive!\n");
    return;
  }
  int id;
  printf("Enter task ID to archive: ");
  scanf("%d", &id);
  if (id < 0 || id >= task_count) {
    printf("Invalid task ID!\n");
    return;
  }
  if (tasks[id].archived) {
    printf("Task already archived.\n");
  } else {
    tasks[id].archived = true;
    save_tasks();
    printf("Task archived!\n");
  }
}

int main() {
  load_tasks();
  int choice;
  while (1) {
    printf("\nTaskC REPL:\n");
    printf("1. Add Task\n");
    printf("2. List Active Tasks\n");
    printf("3. Archive Task\n");
    printf("4. List Archived Tasks \n");
    printf("5. Exit\n");
    printf("Choose an option: ");

    scanf("%d", &choice);

    switch (choice) {
    case 1:
      add_task();
      break;
    case 2:
      list_tasks();
      break;
    case 3:
      archive_task();
      break;
    case 4:
      list_archived_tasks();
      break;
    case 5:
      printf("Goodbye!\n");
      return 0;
    default:
      printf("Invalid choice!\n");
    }
  }
}
