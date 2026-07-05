#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define CFG_PATH "tombo.ini"

static void defaults(AppConfig *cfg) {
  cfg->win_x = 100;
  cfg->win_y = 100;
  cfg->win_w = 800;
  cfg->win_h = 600;
  cfg->tree_w = 200;
  cfg->last_dir[0] = '\0';
  cfg->word_wrap = 0;
  cfg->safe_save = 1;
  cfg->paranoid_save = 0;
  cfg->password_timeout = 0;
}

void config_load(AppConfig *cfg, const char *path) {
  ini_t *ini;
  const char *v;
  defaults(cfg);
  ini = ini_load(path);
  if (!ini) return;
  v = ini_get(ini, "window", "x"); if (v) cfg->win_x = atoi(v);
  v = ini_get(ini, "window", "y"); if (v) cfg->win_y = atoi(v);
  v = ini_get(ini, "window", "w"); if (v) cfg->win_w = atoi(v);
  v = ini_get(ini, "window", "h"); if (v) cfg->win_h = atoi(v);
  v = ini_get(ini, "window", "tree_w"); if (v) cfg->tree_w = atoi(v);
  v = ini_get(ini, "general", "last_dir");
  if (v) { strncpy(cfg->last_dir, v, sizeof(cfg->last_dir) - 1); cfg->last_dir[sizeof(cfg->last_dir) - 1] = '\0'; }
  v = ini_get(ini, "view", "word_wrap"); if (v) cfg->word_wrap = atoi(v);
  v = ini_get(ini, "general", "safe_save"); if (v) cfg->safe_save = atoi(v);
  v = ini_get(ini, "general", "paranoid_save"); if (v) cfg->paranoid_save = atoi(v);
  v = ini_get(ini, "general", "password_timeout"); if (v) cfg->password_timeout = atoi(v);
  ini_free(ini);
}

void config_save(const AppConfig *cfg, const char *path) {
  FILE *f = fopen(path, "w");
  if (!f) return;
  fprintf(f, "[window]\nx=%d\ny=%d\nw=%d\nh=%d\ntree_w=%d\n", cfg->win_x, cfg->win_y, cfg->win_w, cfg->win_h, cfg->tree_w);
  fprintf(f, "[general]\nlast_dir=%s\nsafe_save=%d\nparanoid_save=%d\npassword_timeout=%d\n", cfg->last_dir, cfg->safe_save, cfg->paranoid_save, cfg->password_timeout);
  fprintf(f, "[view]\nword_wrap=%d\n", cfg->word_wrap);
  fclose(f);
}
