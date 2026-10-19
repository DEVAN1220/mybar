#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <time.h>
#include <math.h>
#include<raylib.h>



void make_window_dock(void);
#include "socket.c"
#include "utils.c"

const int width = 1366;
const int height = 25;

const char* font_path = "/home/devan/.local/share/fonts/JetBrainsMonoNerdFontPropo-Regular.ttf";
Font font;

const char numbers[10][2] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };



/*const Color background = { .r = 0x28, .g = 0x28, .b = 0x28, .a = 0xFF }; 
const Color background_2 = { .r = 0x3c, .g = 0x38, .b = 0x36, .a = 0xAA };
const Color text_1 = { .r = 0xeb, .g = 0xdb, .b = 0xb2, .a = 0xFF };
const Color text_2 = { .r = 0x92, .g = 0x83, .b = 0x74, .a = 0xFF };*/

const Color background = color_from_hex(0x282828, 0xFF);
const Color background_2 = color_from_hex(0x3c3836, 0xAA);
const Color text_1 = color_from_hex(0xebdbb2,  0xff);
const Color text_2 = color_from_hex(0x928374, 0xff);

const Color yellow = color_from_hex(0xd79921, 0xAA);
const Color red    = color_from_hex(0xcc241d, 0xAA);


void draw_bar(int focused_workspace, bool occupied_workspaces[10]){


  Vector2 c_size = MeasureTextEx(font, numbers[0], 20, 0);


  float text_y = floorf(((float)height/2)-(c_size.y/2));

  BeginDrawing();
  int count = 0;
  float spacing = 30;
  float padding = 0;
  ClearBackground(BLACK);
  DrawRectangleGradientH(0, 0, width, height, red, yellow);
  Color text_c;
  Color outer_c;


  for(int i=1;i<=10;i++){
	if(occupied_workspaces[i-1]) {
	  text_c = text_1;
	} else {
	  text_c = text_2;
	}

	if(i == focused_workspace) {
	  outer_c = background_2;
	} else {
	  outer_c = BLANK;
	}
	Rectangle outer = {.x = i*spacing - (spacing)/2, .y = 0, .width = spacing, .height = 30};


	float x = floorf(outer.x + ((float)spacing/2)-(c_size.x/2));
	float y = text_y;
	DrawRectangleRec(outer, outer_c);
	DrawTextEx(font, numbers[i-1], (Vector2){.x = x, .y = y},
		22, 0, text_c);

	DrawLine(0, height, width, height, text_1);
  }

  char time_string[60];
  get_time_string(time_string);

  Vector2 time_size = MeasureTextEx(font, time_string, 21, 0);

  DrawTextEx(font, time_string, (Vector2){.x = width-(time_size.x+spacing), .y = text_y},
	  22, 0, text_1);

  EndDrawing();
}


int main() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow(width, height, "bar");
  SetWindowPosition(0, 0); 
  SetTargetFPS(10);




  font = LoadFontEx(font_path, 22, 0, 0);
  GenTextureMipmaps(&font.texture); 
  SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);


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

  struct timespec last_tick, now;
  clock_gettime(CLOCK_MONOTONIC, &last_tick);

  while (1) {
	clock_gettime(CLOCK_MONOTONIC, &now);

	long elapsed_ms = (now.tv_sec - last_tick.tv_sec) * 1000 + 
	  (now.tv_nsec - last_tick.tv_nsec) / 1000000;

	int timeout_ms = 1000 - (int)elapsed_ms;
	if (timeout_ms < 0) timeout_ms = 0;


	int ret = poll(fds, 1, timeout_ms);

	if (ret < 0) {
	  perror("poll error");
	  break;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	long total_elapsed_ms = (now.tv_sec - last_tick.tv_sec) * 1000 + 
	  (now.tv_nsec - last_tick.tv_nsec) / 1000000;

	if (total_elapsed_ms >= 1000) {
	  last_tick = now; // Reset timer anchor
	}

	if (ret > 0 && (fds[0].revents & POLLIN)) {
	  char chunk[256];
	  ssize_t n = read(sock_fd, chunk, sizeof(chunk) - 1);
	  if (n <= 0) break; // Socket closed

	  for (ssize_t i = 0; i < n; i++) {
		if (chunk[i] == '\n') {
		  line_buf[buf_len] = '\0';

		  parse_report(line_buf, occupied_workspaces, &focused_workspace);
		  draw_bar(focused_workspace, occupied_workspaces);

		  buf_len = 0; // Reset buffer for the next line
		} else if (buf_len < sizeof(line_buf) - 1) {
		  line_buf[buf_len++] = chunk[i];
		}
	  }
	} else {
	  draw_bar(focused_workspace, occupied_workspaces);
	}

  }


  UnloadFont(font);
  CloseWindow();
  close(sock_fd);
  return 0;
}
