
#define color_from_hex(hex, alpha) (Color) {\
  .r = (unsigned char)((hex >> 16) & 0xFF),\
  .g = (unsigned char)((hex >> 8) & 0xFF),\
  .b = (unsigned char)((hex >> 0) & 0xFF),\
  .a = (unsigned char)(alpha)\
}


void get_time_string(char time_string[60]){
  struct timespec ts;
    if (timespec_get(&ts, TIME_UTC)) {
    }
    struct tm *tm_info = localtime(&ts.tv_sec);
    strftime(time_string, 60, "%F | %T", tm_info); 
}

void parse_report(char report[1024], bool occupied_workspaces[10], int* focused_workspace){
  char monitor[10];
  char aw[10];
  char layout = 0;
  char node_state = 0;
  char global_state[5] = { 0 };
  int parsed_count = sscanf(report, 
	  "WM%9[^:]:%c%*c:%c%*c:%c%*c:%c%*c:%c%*c:%c%*c:%c%*c:%c%*c:%c%*c:%c%*c:L%c:T%c:G%4s",
	  monitor, &aw[0], &aw[1], &aw[2], &aw[3], &aw[4],
	  &aw[5], &aw[6], &aw[7], &aw[8], &aw[9],
	  &layout, &node_state, global_state);

	for(int i=0;i<10;i++){
	  occupied_workspaces[i] = false;

	  switch(aw[i]){
		case 'o':
		  occupied_workspaces[i] = true;
		  break;
		case 'O':
		  occupied_workspaces[i] = true; *focused_workspace = i+1;
		  break;
		case 'F':
		  *focused_workspace = i+1;
		  break;

		default:
		  break;
	  }
	}

}
