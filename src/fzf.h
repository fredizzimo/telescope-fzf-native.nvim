#ifndef _FZF_H_
#define _FZF_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
  int16_t *data;
  size_t size;
  size_t cap;
  bool allocated;
} fzf_i16_t;

typedef struct {
  int32_t *data;
  size_t size;
  size_t cap;
  bool allocated;
} fzf_i32_t;

typedef struct {
  uint32_t *data;
  size_t size;
  size_t cap;
} fzf_position_t;

typedef struct {
  int32_t start;
  int32_t end;
  int32_t score;

  fzf_position_t *pos;
} fzf_result_t;

typedef struct {
  fzf_i16_t I16;
  fzf_i32_t I32;
} fzf_slab_t;

typedef enum {
  term_fuzzy = 0,
  term_exact,
  term_prefix,
  term_suffix,
  term_equal
} fzf_alg_types;

typedef enum { case_smart = 0, case_ignore, case_respect } fzf_case_types;

typedef struct {
  fzf_alg_types typ;
  bool inv;
  char *ptr;
  void *text;
  bool case_sensitive;
} fzf_term_t;

typedef struct {
  fzf_term_t *ptr;
  size_t size;
  size_t cap;
} fzf_term_set_t;

typedef struct {
  fzf_term_set_t **ptr;
  size_t size;
  size_t cap;
  bool only_inv;
} fzf_pattern_t;

fzf_result_t fzf_fuzzy_match_v1(bool case_sensitive, bool normalize,
                                const char *text, const char *pattern,
                                bool with_pos, fzf_slab_t *slab);
fzf_result_t fzf_fuzzy_match_v2(bool case_sensitive, bool normalize,
                                const char *text, const char *pattern,
                                bool with_pos, fzf_slab_t *slab);
fzf_result_t fzf_exact_match_naive(bool case_sensitive, bool normalize,
                                   const char *text, const char *pattern,
                                   bool with_pos, fzf_slab_t *slab);
fzf_result_t fzf_prefix_match(bool case_sensitive, bool normalize,
                              const char *text, const char *pattern,
                              bool with_pos, fzf_slab_t *slab);
fzf_result_t fzf_suffix_match(bool case_sensitive, bool normalize,
                              const char *text, const char *pattern,
                              bool with_pos, fzf_slab_t *slab);
fzf_result_t fzf_equal_match(bool case_sensitive, bool normalize,
                             const char *text, const char *pattern,
                             bool with_pos, fzf_slab_t *slab);

/* interface */
fzf_pattern_t *fzf_parse_pattern(fzf_case_types case_mode, bool normalize,
                                 char *pattern, bool fuzzy);
void fzf_free_pattern(fzf_pattern_t *pattern);

int32_t fzf_get_score(const char *text, fzf_pattern_t *pattern,
                      fzf_slab_t *slab);
fzf_position_t *fzf_get_positions(const char *text, fzf_pattern_t *pattern,
                                  fzf_slab_t *slab);
void fzf_free_positions(fzf_position_t *pos);

fzf_slab_t *fzf_make_slab(size_t size_16, size_t size_32);
fzf_slab_t *fzf_make_default_slab(void);
void fzf_free_slab(fzf_slab_t *slab);

// ENTRYMANAGER THINGS
typedef struct fzf_node_s fzf_node_t;
struct fzf_node_s {
  fzf_node_t *next;
  fzf_node_t *prev;
  void *item;
};

typedef struct {
  fzf_node_t *head;
  fzf_node_t *tail;
  fzf_node_t *_tracked_node;
  size_t len;
  size_t track_at;
} fzf_linked_list_t;

fzf_linked_list_t *fzf_list_create(size_t track_at);
void fzf_list_free(fzf_linked_list_t *list);

fzf_node_t *fzf_list_append(fzf_linked_list_t *list, void *item);
void fzf_list_prepend(fzf_linked_list_t *list, void *item);

void fzf_list_place_after(fzf_linked_list_t *list, size_t index,
                          fzf_node_t *node, void *item);
void fzf_list_place_before(fzf_linked_list_t *list, size_t index,
                           fzf_node_t *node, void *item);

#endif // _fzf_H_
