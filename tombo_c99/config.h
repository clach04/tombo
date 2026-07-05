#ifndef CONFIG_H
#define CONFIG_H

#include "ini.h"

#define CFG_PATH "tombo.ini"

typedef struct {
  int win_x, win_y, win_w, win_h;
  int tree_w;
  char last_dir[260];
  int word_wrap;
  int safe_save;
  int paranoid_save;
} AppConfig;

void config_load(AppConfig *cfg, const char *path);
void config_save(const AppConfig *cfg, const char *path);

#endif
