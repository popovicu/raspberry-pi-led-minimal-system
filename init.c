#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DEV_NAME "/dev/gpiochip0"

int mount_proc() {
  int mkdir_status = mkdir("/proc", 0777);

  if (mkdir_status != 0) {
    printf("Unable to create /proc: %s\n", strerror(errno));
    return mkdir_status;
  }
  
  int status = mount("none", "/proc", "proc", 0, ""); 

  if (status != 0) {
    printf("Unable to mount proc: %s\n", strerror(errno));
    return status;
  }

  return 0;
}

int mount_devtmpfs() {
  int status = mount("none", "/dev", "devtmpfs", 0, ""); 

  if (status != 0) {
    printf("Unable to mount devtmpfs: %s\n", strerror(errno));
    return status;
  }

  return 0;
}

void print_mounts() {
  printf("============================\n");
  
  FILE *fptr;
  fptr = fopen("/proc/mounts", "r");
  char line[100];
  
  while(fgets(line, 100, fptr)) {
    printf("%s\n", line);
  }
  
  fclose(fptr);

  printf("============================\n");
}

void list_devices() {
  DIR *d;
  struct dirent *dir;
  d = opendir("/dev/");

  printf("============================\n");
  
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      printf("%s\n", dir->d_name);
    }
    
    closedir(d);
  }

  printf("============================\n");
}

int counter = 1;

int switch_gpio(int high, int gpio_fd) {
  struct gpiohandle_data data;
  memset(&data, 0, sizeof(data));
  data.values[0] = high;
  int ret = ioctl(gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

  if (ret == -1) {
    printf("Unable to set line value using ioctl : %s\n", strerror(errno));
    return ret;
  }
}

void print_gpio_line(int line, int fd) {
  struct gpioline_info line_info;
  memset(&line_info, 0, sizeof(line_info));
  line_info.line_offset = line;
  
  int ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &line_info);
  
  if (ret == -1){
    printf("Unable to get line info from offset %d: %s", line, strerror(errno));
  } else {
    printf("offset: %d, name: %s, consumer: %s\n", line, line_info.name, line_info.consumer);
  }
}

int main() {
  printf("Hello from Raspberry!\n");
  
  int proc =  mount_proc();

  if (proc) {
    return 5;
  }

  int devtmpfs = mount_devtmpfs();

  if (devtmpfs) {
    return 6;
  }

  print_mounts();
  
  printf("Listing all devices:\n");
  list_devices();
  
  int fd, ret;
  fd = open(DEV_NAME, O_RDONLY);
  
  if (fd < 0) {
    printf("Unabled to open %s: %s\n", DEV_NAME, strerror(errno));
    return 1;
  }

  struct gpiochip_info info;
  memset(&info, 0, sizeof(info));

  ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
  
  if (ret == -1) {
    printf("Unable to get chip info from ioctl: %s\n", strerror(errno));
    close(fd);
    return 2;
  }
  
  printf("Chip name: %s\n", info.name);
  printf("Chip label: %s\n", info.label);
  printf("Number of lines: %d\n", info.lines);
  
  for (int i = 0; i < info.lines; i++) {
    print_gpio_line(i, fd);
  }

  struct gpiohandle_request rq;
  memset(&rq, 0, sizeof(rq));
  rq.lineoffsets[0] = 17;
  rq.lines = 1;
  rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
  strcpy(rq.consumer_label, "blink-userspace");

  ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);

  if (ret == -1) {
    printf("Unable to line handle from ioctl : %s\n", strerror(errno));
    return 9;
  }

  for (int i = 0; i < 3; i++) {
    printf("Setting high: %d\n", i);
    switch_gpio(1, rq.fd);
    sleep(2);
    printf("Setting low: %d\n", i);
    switch_gpio(0, rq.fd);
    sleep(2);
  }
  
  fflush(stdout);

  close(rq.fd);
  close(fd);
  
  return 0;
}
