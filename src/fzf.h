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
  const char *data;
  size_t size;
} fzf_string_t;

typedef struct {
  fzf_alg_types typ;
  bool inv;
  char *ptr;
  fzf_string_t *text;
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

typedef struct {
  int8_t score_match;
  int8_t score_gap_start;
  int8_t score_gap_extention;
  int8_t bonus_boundary;
  int8_t bonus_non_word;
  int8_t bonus_camel_123;
  int8_t bonus_consecutive;
  int8_t bonus_first_char_multiplier;
} fzf_score_t;

extern fzf_score_t fzf_default_scoring;

fzf_result_t fzf_fuzzy_match_v1(bool case_sensitive, bool normalize,
                                fzf_score_t *scoring, const char *text,
                                const char *pattern, fzf_position_t *pos,
                                fzf_slab_t *slab);
fzf_result_t fzf_fuzzy_match_v1_str(bool case_sensitive, bool normalize,
                                    fzf_score_t *scoring, fzf_string_t *text,
                                    fzf_string_t *pattern, fzf_position_t *pos,
                                    fzf_slab_t *slab);
fzf_result_t fzf_fuzzy_match_v2(bool case_sensitive, bool normalize,
                                fzf_score_t *scoring, const char *text,
                                const char *pattern, fzf_position_t *pos,
                                fzf_slab_t *slab);
fzf_result_t fzf_fuzzy_match_v2_str(bool case_sensitive, bool normalize,
                                    fzf_score_t *scoring, fzf_string_t *text,
                                    fzf_string_t *pattern, fzf_position_t *pos,
                                    fzf_slab_t *slab);
fzf_result_t fzf_exact_match_naive(bool case_sensitive, bool normalize,
                                   fzf_score_t *scoring, const char *text,
                                   const char *pattern, fzf_position_t *pos,
                                   fzf_slab_t *slab);
fzf_result_t fzf_exact_match_naive_str(bool case_sensitive, bool normalize,
                                       fzf_score_t *scoring, fzf_string_t *text,
                                       fzf_string_t *pattern,
                                       fzf_position_t *pos, fzf_slab_t *slab);
fzf_result_t fzf_prefix_match(bool case_sensitive, bool normalize,
                              fzf_score_t *scoring, const char *text,
                              const char *pattern, fzf_position_t *pos,
                              fzf_slab_t *slab);
fzf_result_t fzf_prefix_match_str(bool case_sensitive, bool normalize,
                                  fzf_score_t *scoring, fzf_string_t *text,
                                  fzf_string_t *pattern, fzf_position_t *pos,
                                  fzf_slab_t *slab);
fzf_result_t fzf_suffix_match(bool case_sensitive, bool normalize,
                              fzf_score_t *scoring, const char *text,
                              const char *pattern, fzf_position_t *pos,
                              fzf_slab_t *slab);
fzf_result_t fzf_suffix_match_str(bool case_sensitive, bool normalize,
                                  fzf_score_t *scoring, fzf_string_t *text,
                                  fzf_string_t *pattern, fzf_position_t *pos,
                                  fzf_slab_t *slab);
fzf_result_t fzf_equal_match(bool case_sensitive, bool normalize,
                             fzf_score_t *scoring, const char *text,
                             const char *pattern, fzf_position_t *pos,
                             fzf_slab_t *slab);
fzf_result_t fzf_equal_match_str(bool case_sensitive, bool normalize,
                                 fzf_score_t *scoring, fzf_string_t *text,
                                 fzf_string_t *pattern, fzf_position_t *pos,
                                 fzf_slab_t *slab);

/* interface */
fzf_pattern_t *fzf_parse_pattern(fzf_case_types case_mode, bool normalize,
                                 char *pattern, bool fuzzy);
fzf_pattern_t *fzf_parse_pattern_str(fzf_case_types case_mode, bool normalize,
                                     fzf_string_t *pattern, bool fuzzy);
void fzf_free_pattern(fzf_pattern_t *pattern);

int32_t fzf_get_score(const char *text, fzf_pattern_t *pattern,
                      fzf_score_t *scoring, fzf_slab_t *slab);
int32_t fzf_get_score_str(fzf_string_t *text, fzf_pattern_t *pattern,
                          fzf_score_t *scoring, fzf_slab_t *slab);
void fzf_get_positions(const char *text, fzf_pattern_t *pattern,
                       fzf_score_t *scoring, fzf_position_t *positions,
                       fzf_slab_t *slab);
void fzf_get_positions_str(fzf_string_t *text, fzf_pattern_t *pattern,
                           fzf_score_t *scoring, fzf_position_t *positions,
                           fzf_slab_t *slab);
void fzf_alloc_positions(fzf_position_t *pos, size_t size);
void fzf_free_positions(fzf_position_t *pos);
size_t fzf_get_num_positions(fzf_pattern_t *pattern);

fzf_slab_t *fzf_make_slab(size_t size_16, size_t size_32);
fzf_slab_t *fzf_make_default_slab(void);
void fzf_free_slab(fzf_slab_t *slab);

#endif // _fzf_H_
