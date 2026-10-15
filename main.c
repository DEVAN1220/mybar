#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <time.h>


#include "socket.c"

void parse_report(char report[1024], bool occupied_workspaces[10], int* focused_workspace){
  char monitor[10];
  char aw[10];
	sscanf(report, "WM%9[^:]:%c1:%c2:%c3:%c4:%c5:%c6:%c7:%c8:%c9:%c0:LT:TT:G",
					monitor, &aw[0], &aw[1], &aw[2], &aw[3], &aw[4], &aw[5], 
							 &aw[6], &aw[7], &aw[8], &aw[9]);

	for(int i=0;i<10;i++){
	  occupied_workspaces[i] = false;
	  if(aw[i] == 'o'){ occupied_workspaces[i] = true; }
	  else if(aw[i] == 'O'){ occupied_workspaces[i] = true; *focused_workspace = i+1;}
	  printf("%d", occupied_workspaces[i]);
	}

}

int main() {

	int sock_fd = create_socket();

    char *cmd[] = {"subscribe", "report"};
    int cmd_count = sizeof(cmd) / sizeof(cmd[0]);


    if (send_bspwm_cmd(sock_fd, cmd, cmd_count) == -1) {
        perror("Failed to send command stream");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }


	struct pollfd fds[1];
    fds[0].fd = sock_fd;
    fds[0].events = POLLIN;



	bool occupied_workspaces[10] = { 0 };
	int focused_workspace = 1;

    char line_buf[1024];
    size_t buf_len = 0;

    // Track time using a monotonic clock for accurate 1-second ticks
    struct timespec last_tick, now;
    clock_gettime(CLOCK_MONOTONIC, &last_tick);

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        
        // Calculate elapsed milliseconds since last tick
        long elapsed_ms = (now.tv_sec - last_tick.tv_sec) * 1000 + 
                          (now.tv_nsec - last_tick.tv_nsec) / 1000000;
        
        int timeout_ms = 1000 - (int)elapsed_ms;
        if (timeout_ms < 0) timeout_ms = 0;

        // Wait for socket input or until the 1-second timer expires
        int ret = poll(fds, 1, timeout_ms);

        if (ret < 0) {
            perror("poll error");
            break;
        }

        // 1. Data arrived from bspwm! Read chunk and extract lines.
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            char chunk[256];
            ssize_t n = read(sock_fd, chunk, sizeof(chunk) - 1);
            if (n <= 0) break; // Socket closed

            for (ssize_t i = 0; i < n; i++) {
                if (chunk[i] == '\n') {
                    line_buf[buf_len] = '\0';
                    
                    parse_report(line_buf, occupied_workspaces, &focused_workspace);
                    //update_bar_display(occupied_workspaces, focused_workspace);
					for(int i=0;i<10;i++){
					  printf("%d", occupied_workspaces[i]);

					}

					  printf("\n");
                    buf_len = 0; // Reset buffer for the next line
                } else if (buf_len < sizeof(line_buf) - 1) {
                    line_buf[buf_len++] = chunk[i];
                }
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        long total_elapsed_ms = (now.tv_sec - last_tick.tv_sec) * 1000 + 
                                (now.tv_nsec - last_tick.tv_nsec) / 1000000;

        if (total_elapsed_ms >= 1000) {
            //trigger_periodic_update();
            last_tick = now; // Reset timer anchor
        }
    }



	printf("%d", focused_workspace );
    close(sock_fd);
    return 0;
}
