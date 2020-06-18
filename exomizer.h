/*

Warning! This is a modified version of original sources!

To sum up:
- all include files and C sources were merged in a single file
- existing logs were removed (except error logs)
- main were removed and wrapper added


*/

#ifndef ALREADY_INCLUDED_CALLBACK
#define ALREADY_INCLUDED_CALLBACK

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

typedef int cb_cmp(const void *a, const void *b);
typedef void cb_free(void *a);
typedef void cb_fprint(FILE *f, const void *a);

#endif
#ifndef INCLUDED_INT
#define INCLUDED_INT

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

typedef signed char i8;
typedef signed short int i16;
typedef signed int i32;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;

#endif
#ifndef ALREADY_INCLUDED_PROGRESS
#define ALREADY_INCLUDED_PROGRESS

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

struct progress
{
    char *msg;
    float factor;
    int offset;
    int last;
};

void progress_init(struct progress p[1], char *msg, int start, int end);

void progress_bump(struct progress p[1], int pos);

void progress_free(struct progress p[1]);

#endif
#ifndef ALREADY_INCLUDED_CHUNKPOOL
#define ALREADY_INCLUDED_CHUNKPOOL

/*
 * Copyright (c) 2003 -2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

#define CHUNKPOOL_CHUNKS_MAX    64

struct chunkpool {
    int chunk_size;
    int chunk;
    int chunk_pos;
    int chunk_max;
    void *chunks[64];
};

void
chunkpool_init(struct chunkpool *ctx, int size);

void
chunkpool_free(struct chunkpool *ctx);

void chunkpool_free2(struct chunkpool *ctx, cb_free *f);

void *
chunkpool_malloc(struct chunkpool *ctx);

void *
chunkpool_calloc(struct chunkpool *ctx);

#endif
#ifndef ALREADY_INCLUDED_MATCH
#define ALREADY_INCLUDED_MATCH
/*
 * Copyright (c) 2002 - 2005, 2013 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

struct match {
    unsigned short int offset;
    unsigned short int len;
    struct match *next;
};

typedef struct match match[1];
typedef struct match *matchp;
typedef const struct match *const_matchp;

struct pre_calc {
    struct match_node *single;
    const struct match *cache;
};

struct match_ctx {
    struct chunkpool m_pool[1];
    struct pre_calc (*info)[1];
    unsigned short int *rle;
    unsigned short int *rle_r;
    const unsigned char *buf;
    int len;
    int max_offset;
    int max_len;
};

typedef struct match_ctx match_ctx[1];
typedef struct match_ctx *match_ctxp;

//void match_ctx_init(match_ctx ctx,          /* IN/OUT */ struct membuf *inbuf,   /* IN */ int max_len,            /* IN */ int max_offset,         /* IN */ int use_imprecise_rle); /* IN */

void match_ctx_free(match_ctx ctx);     /* IN/OUT */

/* this needs to be called with the indexes in
 * reverse order */
const_matchp matches_get(match_ctx ctx, /* IN/OUT */
                         int index);     /* IN */

void match_delete(match_ctx ctx,        /* IN/OUT */
                  matchp mp);   /* IN */

struct matchp_cache_enum {
    match_ctxp ctx;
    const_matchp next;
    match tmp1;
    match tmp2;
    int pos;
};

typedef struct matchp_cache_enum matchp_cache_enum[1];
typedef struct matchp_cache_enum *matchp_cache_enump;

void matchp_cache_get_enum(match_ctx ctx,       /* IN */
                           matchp_cache_enum mpce);     /* IN/OUT */

typedef const_matchp matchp_enum_get_next_f(void *matchp_enum); /* IN/OUT */

const_matchp matchp_cache_enum_get_next(void *matchp_cache_enum);       /* IN */

#endif
#ifndef ALREADY_INCLUDED_OUTPUT
#define ALREADY_INCLUDED_OUTPUT

/*
 * Copyright (c) 2002 - 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

struct _output_ctx {
    unsigned int bitbuf;
    int pos;
    int start;
    struct membuf *buf;
};

typedef struct _output_ctx output_ctx[1];
typedef struct _output_ctx *output_ctxp;

void output_ctx_init(output_ctx ctx, struct membuf *out);   /* IN/OUT */

unsigned int output_get_pos(output_ctx ctx);    /* IN */

void output_byte(output_ctx ctx,        /* IN/OUT */
                 unsigned char byte);   /* IN */

void output_word(output_ctx ctx,        /* IN/OUT */
                 unsigned short int word);      /* IN */

void output_bits_flush(output_ctx ctx); /* IN/OUT */

void output_bits(output_ctx ctx,        /* IN/OUT */
                 int count,     /* IN */
                 int val);      /* IN */

void output_gamma_code(output_ctx ctx,  /* IN/OUT */
                       int code);       /* IN */
#endif
#ifndef ALREADY_INCLUDED_SEARCH
#define ALREADY_INCLUDED_SEARCH

/*
 * Copyright (c) 2002 - 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

struct _search_node {
    int index;
    match match;
    unsigned int total_offset;
    float total_score;
    struct _search_node *prev;
};

typedef struct _search_node search_node[1];
typedef struct _search_node *search_nodep;
typedef const struct _search_node *const_search_nodep;

struct _encode_match_data {
    output_ctxp out;
    void *priv;
};

typedef struct _encode_match_data encode_match_data[1];
typedef struct _encode_match_data *encode_match_datap;

/* example of what may be used for priv data
 * field in the encode_match_data struct */
typedef
float encode_int_f(int val, void *priv, output_ctxp out);       /* IN */

struct _encode_match_priv {
    int lit_num;
    int seq_num;
    int rle_num;
    float lit_bits;
    float seq_bits;
    float rle_bits;

    encode_int_f *offset_f;
    encode_int_f *len_f;
    void *offset_f_priv;
    void *len_f_priv;

    output_ctxp out;
};

typedef struct _encode_match_priv encode_match_priv[1];
typedef struct _encode_match_priv *encode_match_privp;
/* end of example */

typedef
float encode_match_f(const_matchp mp, encode_match_data emd);   /* IN */

void search_node_dump(search_nodep snp);        /* IN */

void search_node_free(search_nodep snp);        /* IN/OUT */

search_nodep search_buffer(match_ctx ctx,       /* IN */
                           encode_match_f * f,  /* IN */
                           encode_match_data emd,
                           int use_literal_sequences);      /* IN */

struct _matchp_snp_enum {
    const_search_nodep startp;
    const_search_nodep currp;
};

typedef struct _matchp_snp_enum matchp_snp_enum[1];
typedef struct _matchp_snp_enum *matchp_snp_enump;

void matchp_snp_get_enum(const_search_nodep snp,        /* IN */
                         matchp_snp_enum snpe); /* IN/OUT */

const_matchp matchp_snp_enum_get_next(void *matchp_snp_enum);

#endif
#ifndef ALREADY_INCLUDED_RADIX
#define ALREADY_INCLUDED_RADIX
/*
 * Copyright (c) 2002, 2003 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

typedef struct _radix_node *radix_nodep;

struct _radix_root {
    int depth;
    radix_nodep root;
    struct chunkpool mem[1];
};

typedef struct _radix_root radix_root[1];
typedef struct _radix_root *radix_rootp;


typedef void free_callback(void *data, void *priv);

/* *f will be called even for null pointers */
void radix_tree_free(radix_root rr,     /* IN */
                     free_callback * f, /* IN */
                     void *priv);       /* IN */

void radix_tree_init(radix_root rr);    /* IN */

void radix_node_set(radix_root rr,      /* IN */
                    unsigned int index, /* IN */
                    void *data);        /* IN */

void *radix_node_get(radix_root rr,     /* IN */
                     unsigned int index);       /* IN */

#endif
#ifndef MEMBUF_IO_ALREADY_INCLUDED
#define MEMBUF_IO_ALREADY_INCLUDED

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

void read_file(const char *name, struct membuf *buf);
void write_file(const char *name, struct membuf *buf);

#endif
#ifndef ALREADY_INCLUDED_MEMBUF
#define ALREADY_INCLUDED_MEMBUF

/*
 * Copyright (c) 2002 - 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

#define STATIC_MEMBUF_INIT {0, 0, 0}

struct membuf {
    void *buf;
    int len;
    int size;
};

void membuf_init(struct membuf *sb);
void membuf_clear(struct membuf *sb);
void membuf_free(struct membuf *sb);
void membuf_new(struct membuf **sbp);
void membuf_delete(struct membuf **sbp);
/* Gets the length of data put into the membuf */
int membuf_memlen(const struct membuf *sb);
void membuf_truncate(struct membuf *sb, int len);

/* returns the new len or < 0 if failure */
int membuf_trim(struct membuf *sb, int pos);

void *membuf_memcpy(struct membuf *sb, int offset, const void *mem, int len);
void *membuf_append(struct membuf *sb, const void *mem, int len);
void *membuf_append_char(struct membuf *sb, char c);
void *membuf_insert(struct membuf *sb, int offset, const void *mem, int len);
void membuf_remove(struct membuf *sb, int offset, int len);
/* Grows the capacity if it's less than the given size */
void membuf_atleast(struct membuf *sb, int size);
/* Skrinks the capacity if it's greater than the given size */
void membuf_atmost(struct membuf *sb, int size);
/* Gets the current capacity of the membuf */
int membuf_get_size(const struct membuf *sb);
/* Gets a pointer to the internal buffer. Don't dereferece it beyond
 * its size. */
void *membuf_get(const struct membuf *sb);

#endif
#ifndef INCLUDED_PARSE
#define INCLUDED_PARSE

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

#define ATOM_TYPE_OP_ARG_NONE    0	/* uses u.op */
#define ATOM_TYPE_OP_ARG_U8      1	/* uses u.op */
#define ATOM_TYPE_OP_ARG_U16     2	/* uses u.op */
#define ATOM_TYPE_OP_ARG_I8      3	/* uses u.op */
#define ATOM_TYPE_OP_ARG_UI8     4	/* uses u.op */
#define ATOM_TYPE_EXPRS		12	/* uses u.exprs */
#define ATOM_TYPE_WORD_EXPRS	10	/* uses u.exprs */
#define ATOM_TYPE_BYTE_EXPRS	11	/* uses u.exprs */
#define ATOM_TYPE_RES		13	/* uses u.res */
#define ATOM_TYPE_BUFFER	14	/* uses u.buffer */

struct op
{
    struct expr *arg;
    u8 code;
};

struct res
{
    struct expr *length;
    struct expr *value;
};

struct buffer
{
    const char *name;
    i32 length;
    i32 skip;
};

struct atom
{
    u8 type;
    union
    {
        struct op op;
        struct vec *exprs;
        struct buffer buffer;
        struct res res;
    } u;
};

extern int push_state_skip;
extern int push_state_macro;
extern int push_state_init;
extern int num_lines;

void parse_init(void);
void parse_free(void);

void set_initial_symbol(const char *symbol, i32 value);
void initial_symbol_dump(int level, const char *symbol);

struct membuf *new_initial_named_buffer(const char *name);

int assemble(struct membuf *source, struct membuf *dest);

/* start of internal functions */

struct atom *new_op(u8 op_code, u8 op_size, struct expr *arg);
struct atom *new_op0(u8 op_code);

struct atom *new_exprs(struct expr *arg);
struct atom *exprs_add(struct atom *atom, struct expr *arg);
struct atom *exprs_to_byte_exprs(struct atom *atom);
struct atom *exprs_to_word_exprs(struct atom *atom);

struct atom *new_res(struct expr *len, struct expr *value);
struct atom *new_incbin(const char *name,
                        struct expr *skip, struct expr *len);

struct expr *new_is_defined(const char *symbol);
struct expr *new_expr_inclen(const char *name);
struct expr *new_expr_incword(const char *name,
                              struct expr *skip);

void new_symbol_expr(const char *symbol, struct expr *arg);
void new_symbol_expr_guess(const char *symbol, struct expr *arg);

/* returns NULL if found, not otherwise, expp may be NULL. */
const char *find_symref(const char *symbol,
                        struct expr **expp);

int resolve_symbol(const char *symbol, int *has_valuep, i32 *valuep);
void symbol_dump_resolved(int level, const char *symbol);

void new_label(const char *label);
void set_org(struct expr *arg);
void push_if_state(struct expr *arg);
void push_macro_state(const char *name);
void macro_append(const char *text);
void asm_error(const char *msg);
void asm_echo(const char *msg, struct atom *atom);
void asm_include(const char *msg);

void output_atoms(struct membuf *out, struct vec *mem);
void asm_src_buffer_push(struct membuf *buf);

int assembleSinglePass(struct membuf *source, struct membuf *dest);

#endif
#ifndef ALREADY_INCLUDED_OPTIMAL
#define ALREADY_INCLUDED_OPTIMAL

/*
 * Copyright (c) 2002 - 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

float optimal_encode(const_matchp mp,   /* IN */
                     encode_match_data emp);    /* IN */

void optimal_init(encode_match_data emp);        /* IN/OUT */

void optimal_free(encode_match_data emd);       /* IN */

void optimal_optimize(encode_match_data emd,    /* IN/OUT */
                      matchp_enum_get_next_f * f,       /* IN */
                      void *priv);      /* IN */

void optimal_encoding_import(encode_match_data emd,     /* IN/OUT */
                             const char *encoding);     /* IN */

const char *
optimal_encoding_export(encode_match_data emd);    /* IN */

void optimal_dump(int level, encode_match_data emp);       /* IN */

void optimal_out(output_ctx out,        /* IN/OUT */
                 encode_match_data emd);        /* IN */

#endif
#ifndef ALREADY_INCLUDED_VEC
#define ALREADY_INCLUDED_VEC

/*
 * Copyright (c) 2003 - 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */
#define STATIC_VEC_INIT(EL_SIZE) {(EL_SIZE), STATIC_MEMBUF_INIT, 1}

struct vec {
    size_t elsize;
    struct membuf buf;
    int flags;
};

struct vec_iterator {
    const struct vec *vec;
    int pos;
};

void vec_init(struct vec *p, size_t elsize);
void vec_clear(struct vec *p, cb_free * f);
void vec_free(struct vec *p, cb_free * f);

int vec_count(const struct vec *p);
void *vec_get(const struct vec *p, int index);

/**
 * Returns a pointer to the set area or null if the index is out of
 * bounds.
 **/
void *vec_set(struct vec *p, int index, const void *in);
void *vec_insert(struct vec *p, int index, const void *in);
void vec_remove(struct vec *p, int index);

void *vec_push(struct vec *p, const void *in);

/**
 * Gets the position where the key is stored in the vector. The vector
 * needs to be sorted for this function to work. Returns the position,
 * -1 on error or a negative number that can be converted to where
 * it should have been if it had been inserted. insert_pos = -(val + 2)
 **/
int vec_find(const struct vec *p, cb_cmp * f, const void *key);

/**
 * Gets a pointer to the element that the key points to.
 * Returns a pointer that may be null if not found.
 **/
void *vec_find2(const struct vec *p, cb_cmp * f, const void *key);

/**
 * Inserts the in element in its correct position in a sorted vector.
 * returns 1 if insertion is successful, 0 if element is already
 * present or -1 on error. If out is not NULL it will be
 * dereferenced and set to the inserted or present element.
 **/
int vec_insert_uniq(struct vec *p, cb_cmp * f, const void *in, void **out);
void vec_sort(struct vec *p, cb_cmp * f);

void vec_get_iterator(const struct vec *p, struct vec_iterator *i);
void *vec_iterator_next(struct vec_iterator *i);

int vec_equals(const struct vec *a, const struct vec *b, cb_cmp *equals);
void vec_fprint(FILE *, const struct vec *a, cb_fprint *fprint);

#endif
#ifndef ALREADY_INCLUDED_MAP
#define ALREADY_INCLUDED_MAP

/*
 * Copyright (c) 2006 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

struct map_entry {
    const char *key;
    void *value;
};

#define STATIC_MAP_INIT {STATIC_VEC_INIT(sizeof(struct map_entry))}

struct map {
    struct vec vec;
};

struct map_iterator {
    struct vec_iterator vec;
};

void map_init(struct map *m);
void map_clear(struct map *m);
void map_free(struct map *m);

void *map_put(struct map *m, const char *key, void *value);
void *map_get(const struct map *m, const char *key);
int map_contains_key(const struct map *m, const char *key);
void map_put_all(struct map *m, const struct map *source);

int map_contains(const struct map *m1, const struct map *m2, cb_cmp *f);

/**
 * If f is NULL, only the keys will be compared.
 * returns -1 on error, 1 on equality and 0 otherwise,
 **/
int map_equals(const struct map *m1, const struct map *m2, cb_cmp *f);

void map_get_iterator(const struct map *p, struct map_iterator *i);
const struct map_entry *map_iterator_next(struct map_iterator *i);

#endif
#ifndef NAMED_BUFFER_INCLUDED
#define NAMED_BUFFER_INCLUDED

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

struct named_buffer {
    struct map map;
    struct chunkpool buf;
};

void named_buffer_init(struct named_buffer *nb);
void named_buffer_free(struct named_buffer *nb);
void named_buffer_clear(struct named_buffer *nb);

void named_buffer_copy(struct named_buffer *nb,
                       const struct named_buffer *source);

struct membuf *new_named_buffer(struct named_buffer *nb, const char *name);
struct membuf *get_named_buffer(struct named_buffer *nb, const char *name);

#endif
#ifndef ALREADY_INCLUDED_LOG
#define ALREADY_INCLUDED_LOG
/*
 * Copyright (c) 2002, 2003 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */


#ifndef __GNUC__
#define  __attribute__(x)  /*NOTHING*/
#endif

enum log_level {
    LOG_MIN = -99,
    LOG_FATAL = -40,
    LOG_ERROR = -30,
    LOG_WARNING = -20,
    LOG_BRIEF = -10,
    LOG_NORMAL = 0,
    LOG_VERBOSE = 10,
    LOG_TRACE = 20,
    LOG_DEBUG = 30,
    LOG_DUMP = 40,
    LOG_MAX = 99
};

typedef
void log_formatter_f(FILE * out,        /* IN */
                     enum log_level level,      /* IN */
                     const char *context,       /* IN */
                     const char *);     /* IN */

/*
 * this log output function adds nothing
 */
void raw_log_formatter(FILE * out,      /* IN */
                       enum log_level level,    /* IN */
                       const char *context,     /* IN */
                       const char *log);        /* IN */


struct log_output;

struct log_ctx;

struct log_ctx *log_new(void);

/* log_delete closes all added output streams
 * and files except for stdout and stderr
 */
void log_delete(struct log_ctx *ctx);

void log_set_level(struct log_ctx *ctx, /* IN/OUT */
                   enum log_level level);       /* IN */

void log_add_output_stream(struct log_ctx *ctx, /* IN/OUT */
                           enum log_level min,  /* IN */
                           enum log_level max,  /* IN */
                           log_formatter_f * default_f, /* IN */
                           FILE * out_stream);  /* IN */

void log_vlog(struct log_ctx *ctx,      /* IN */
              enum log_level level,     /* IN */
              const char *context,      /* IN */
              log_formatter_f * f,      /* IN */
              const char *printf_str,   /* IN */
              va_list argp);


void log_log_default(const char *printf_str,    /* IN */
                     ...)
    __attribute__((format(printf,1,2)));

/* some helper macros */

extern struct log_ctx *G_log_ctx;
extern enum log_level G_log_level;
extern enum log_level G_log_log_level;

#define LOG_SET_LEVEL(L) \
do { \
    log_set_level(G_log_ctx, (L)); \
    G_log_level = (L); \
} while(0)

#define LOG_INIT(L) \
do { \
    G_log_ctx = log_new(); \
    log_set_level(G_log_ctx, (L)); \
    G_log_level = (L); \
} while(0)

#define LOG_INIT_CONSOLE(X) \
do { \
    G_log_ctx = log_new(); \
    log_set_level(G_log_ctx, (X)); \
    G_log_level = (X); \
    log_add_output_stream(G_log_ctx, LOG_WARNING, LOG_MAX, NULL, stderr); \
    log_add_output_stream(G_log_ctx, LOG_MIN, LOG_WARNING - 1, NULL, stderr); \
} while(0)

#define LOG_FREE log_delete(G_log_ctx)

#define IS_LOGGABLE(L) (G_log_level >= (L))

#define LOG(L, M) \
do { \
    if(IS_LOGGABLE(L)) { \
        G_log_log_level = (L); \
        log_log_default M; \
    } \
} while(0)


void hex_dump(int level, unsigned char *p, int len);

#endif
#ifndef ALREADY_INCLUDED_GETFLAG
#define ALREADY_INCLUDED_GETFLAG
/*
 * Copyright (c) 2002, 2003 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

extern int flagind;
extern int flagflag;
extern const char *flagarg;

int getflag(int argc, char *argv[], const char *flags);

#endif
#ifndef ALREADY_INCLUDED_EXODEC
#define ALREADY_INCLUDED_EXODEC

/*
 * Copyright (c) 2005 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */


struct dec_table
{
    unsigned char table_bit[3];
    unsigned char table_off[3];
    unsigned char table_bi[100];
    unsigned char table_lo[100];
    unsigned char table_hi[100];
};

struct dec_ctx
{
    int inpos;
    int inend;
    unsigned char *inbuf;
    struct membuf *outbuf;
    unsigned int bitbuf;
    /* dep_table */
    struct dec_table t[1];
    int bits_read;
};

/* returns the encoding */
char *
dec_ctx_init(struct dec_ctx ctx[1],
             struct membuf *inbuf, struct membuf *outbuf);

void
dec_ctx_free(struct dec_ctx ctx[1]);

void dec_ctx_decrunch(struct dec_ctx ctx[1]);

#endif
#ifndef EXO_UTIL_ALREADY_INCLUDED
#define EXO_UTIL_ALREADY_INCLUDED

/*
 * Copyright (c) 2008 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */


/*
 * target is the basic token for the sys/call basic command
 * it may be -1 for hardcoded detection of a few targets.
 */
int find_sys(const unsigned char *buf, int target);

struct load_info
{
    int basic_txt_start; /* in */
    int basic_var_start; /* out */
    int run; /* out */
    int start; /* out */
    int end; /* out */
};

void load_located(char *filename, unsigned char mem[65536],
                  struct load_info *info);

int str_to_int(const char *str, int *value);

const char *fixup_appl(char *appl);

#endif
#ifndef EXO_HELPER_ALREADY_INCLUDED
#define EXO_HELPER_ALREADY_INCLUDED

/*
 * Copyright (c) 2005, 2013 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */


#define CRUNCH_OPTIONS_DEFAULT {NULL, 65535, 65535, 65535, 1, 0}

struct common_flags
{
    struct crunch_options *options;
    const char *outfile;
};

#define CRUNCH_FLAGS "cCe:m:M:p:o:qv"
#define BASE_FLAGS "o:qv"

void print_crunch_flags(enum log_level level, const char *default_outfile);

void print_base_flags(enum log_level level, const char *default_outfile);

typedef void print_usage_f(const char *appl, enum log_level level,
                           const char *default_outfile);

void handle_crunch_flags(int flag_char, /* IN */
                         const char *flag_arg, /* IN */
                         print_usage_f *print_usage, /* IN */
                         const char *appl, /* IN */
                         struct common_flags *options); /* OUT */

void handle_base_flags(int flag_char, /* IN */
                       const char *flag_arg, /* IN */
                       print_usage_f *print_usage, /* IN */
                       const char *appl, /* IN */
                       const char **default_outfilep); /* OUT */

struct crunch_options
{
    const char *exported_encoding;
    int max_passes;
    int max_len;
    int max_offset;
    int use_literal_sequences;
    int use_imprecise_rle;
};

struct crunch_info
{
    int literal_sequences_used;
    int needed_safety_offset;
};

void print_license(void);

void crunch_backwards(struct membuf *inbuf,
                      struct membuf *outbuf,
                      struct crunch_options *options, /* IN */
                      struct crunch_info *info); /* OUT */

void exocrunch(struct membuf *inbuf,
            struct membuf *outbuf,
            struct crunch_options *options, /* IN */
            struct crunch_info *info); /* OUT */

void exodecrunch(int level,
              struct membuf *inbuf,
              struct membuf *outbuf);

void decrunch_backwards(int level,
                        struct membuf *inbuf,
                        struct membuf *outbuf);

void reverse_buffer(char *start, int len);
#endif
#ifndef ALREADY_INCLUDED_DESFX
#define ALREADY_INCLUDED_DESFX

/*
 * Copyright (c) 2007 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

u16 decrunch_sfx(u8 mem[65536], u16 run, u16 *start, u16 *end);

#endif

/*
 * Copyright (c) 2002 - 2007 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */
#define DEFAULT_OUTFILE "a.out"
#define OUTPUT_FLAG_REVERSE 1

/*
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
*/

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif
#ifdef DJGPP
#define vsnprintf(A, B, C, D) vsprintf((A),(C),(D))
#endif

#define BAR_LENGTH 64

//extern struct membuf sfxdecr[];


#define RADIX_TREE_NODE_RADIX 11U
#define RADIX_TREE_NODE_MASK  ((1U << RADIX_TREE_NODE_RADIX) - 1U)

struct _radix_node {
    struct _radix_node *rn;
};

void radix_tree_init(radix_root rr)     /* IN */
{
    rr->depth = 0;
    rr->root = NULL;

    chunkpool_init(rr->mem, (1 << RADIX_TREE_NODE_RADIX) * sizeof(void *));
}

static
void radix_tree_free_helper(int depth, radix_nodep rnp, free_callback * f,      /* IN */
                            void *priv) /* IN */
{
    int i;
    do
    {
        if (depth == 0)
        {
            /* do something to the data pointer? */
            if (f != NULL)
            {
                f(rnp, priv);
            }
            break;
        }
        if (rnp == NULL)
        {
            /* tree not grown here */
            break;
        }

        for (i = RADIX_TREE_NODE_MASK; i >= 0; --i)
        {
            radix_tree_free_helper(depth - 1, rnp[i].rn, f, priv);
            rnp[i].rn = NULL;
        }
    }
    while (0);
}

void radix_tree_free(radix_root rr,     /* IN */
                     free_callback * f, /* IN */
                     void *priv)        /* IN */
{
    radix_tree_free_helper(rr->depth, rr->root, f, priv);
    rr->depth = 0;
    rr->root = NULL;
    chunkpool_free(rr->mem);
}

void radix_node_set(radix_rootp rrp,    /* IN */
                    unsigned int index, /* IN */
                    void *data) /* IN */
{
    radix_nodep rnp;
    radix_nodep *rnpp;
    unsigned int mask;
    int depth;

    mask = ~0U << (RADIX_TREE_NODE_RADIX * rrp->depth);
    while (index & mask)
    {
        /*LOG(LOG_DUMP, ("calloc called\n")); */
        /* not deep enough, let's deepen the tree */
	rnp = chunkpool_calloc(rrp->mem);

        rnp[0].rn = rrp->root;
        rrp->root = rnp;
        rrp->depth += 1;

        mask = ~0U << (RADIX_TREE_NODE_RADIX * rrp->depth);
    }

    /* go down */
    rnpp = &rrp->root;
    for (depth = rrp->depth - 1; depth >= 0; --depth)
    {
        unsigned int node_index;

        if (*rnpp == NULL)
        {
            /*LOG(LOG_DUMP, ("calloc called\n")); */
            /* tree is not grown in this interval */
	    *rnpp = chunkpool_calloc(rrp->mem);
        }
        node_index = ((index >> (RADIX_TREE_NODE_RADIX * depth)) &
                      RADIX_TREE_NODE_MASK);

        rnpp = &((*rnpp)[node_index].rn);
    }
    *rnpp = data;
}

void *radix_node_get(radix_root rr,     /* IN */
                     unsigned int index)        /* IN */
{
    radix_nodep rnp;
    unsigned short int depth;

    /* go down */
    rnp = rr->root;
    for (depth = rr->depth - 1; depth < 0xffff; --depth)
    {
        unsigned short int node_index;

        if (rnp == NULL)
        {
            /* tree is not grown in this interval */
            break;
        }
        node_index = ((index >> (RADIX_TREE_NODE_RADIX * depth)) &
                      RADIX_TREE_NODE_MASK);

        rnp = rnp[node_index].rn;
    }
    return rnp;
}

void search_node_free(search_nodep snp) /* IN */
{
    /* emty now since snp:s are stored in an array */
}

search_nodep search_buffer(match_ctx ctx,       /* IN */
                           encode_match_f * f,  /* IN */
                           encode_match_data emd,       /* IN */
                           int use_literal_sequences)
{
    static struct membuf backing[1] = { STATIC_MEMBUF_INIT };
    static search_node *snp_arr;
    const_matchp mp = NULL;
    search_nodep snp;
    search_nodep best_copy_snp;
    int best_copy_len;

    search_nodep best_rle_snp;

    int len = ctx->len + 1;


    membuf_atleast(backing, len * sizeof(search_node));
    snp_arr = membuf_get(backing);
    memset(snp_arr, 0, len * sizeof(search_node));

    --len;
    snp = snp_arr[len];
    snp->index = len;
    snp->match->offset = 0;
    snp->match->len = 0;
    snp->total_offset = 0;
    snp->total_score = 0;
    snp->prev = NULL;

    best_copy_snp = snp;
    best_copy_len = 0.0;

    best_rle_snp = NULL;

    /* think twice about changing this code,
     * it works the way it is. The last time
     * I examined this code I was certain it was
     * broken and broke it myself, trying to fix it. */
    while (len > 0 && (mp = matches_get(ctx, len - 1)) != NULL)
    {
        float prev_score;
        float prev_offset_sum;

        if(use_literal_sequences)
        {
            /* check if we can do even better with copy */
            snp = snp_arr[len];
            if(best_copy_snp->total_score+best_copy_len * 8.0 -
               snp->total_score > 0.0 || best_copy_len > 65535)
            {
                /* found a better copy endpoint */
                best_copy_snp = snp;
                best_copy_len = 0.0;
            } else
            {
                float copy_score = best_copy_len * 8.0 + (1.0 + 17.0 + 17.0);
                float total_copy_score = best_copy_snp->total_score +
                                         copy_score;


                if(snp->total_score > total_copy_score )
                {
                    match local_mp;
                    /* here it is good to just copy instead of crunch */


                    local_mp->len = best_copy_len;
                    local_mp->offset = 0;
                    local_mp->next = NULL;
                    snp->total_score = total_copy_score;
                    snp->total_offset = best_copy_snp->total_offset;
                    snp->prev = best_copy_snp;
                    *snp->match = *local_mp;
                }
            }
            /* end of copy optimization */
        }

        /* check if we can do rle */
        snp = snp_arr[len];
        if(best_rle_snp == NULL ||
           snp->index + 65535 < best_rle_snp->index ||
           snp->index + ctx->rle_r[snp->index] < best_rle_snp->index)
        {
            /* best_rle_snp can't be reached by rle from snp, reset it*/
            if(ctx->rle[snp->index] > 0)
            {
                best_rle_snp = snp;
            }
            else
            {
                best_rle_snp = NULL;
            }
        }
        else if(ctx->rle[snp->index] > 0 &&
                snp->index + ctx->rle_r[snp->index] >= best_rle_snp->index)
        {
            float best_rle_score;
            float total_best_rle_score;
            float snp_rle_score;
            float total_snp_rle_score;
            match rle_mp;


            /* snp and best_rle_snp is the same rle area,
             * let's see which is best */
#undef NEW_STYLE
#ifdef NEW_STYLE
            rle_mp->len = best_rle_snp->index - snp->index;
#else
            rle_mp->len = ctx->rle[best_rle_snp->index];
#endif
            rle_mp->offset = 1;
            best_rle_score = f(rle_mp, emd);
            total_best_rle_score = best_rle_snp->total_score +
                best_rle_score;

#ifdef NEW_STYLE
            snp_rle_score = 0.0;
#else
            rle_mp->len = ctx->rle[snp->index];
            rle_mp->offset = 1;
            snp_rle_score = f(rle_mp, emd);
#endif
            total_snp_rle_score = snp->total_score + snp_rle_score;

            if(total_snp_rle_score <= total_best_rle_score)
            {
                /* yes, the snp is a better rle than best_rle_snp */
                best_rle_snp = snp;
            }
        }
        if(best_rle_snp != NULL && best_rle_snp != snp)
        {
            float rle_score;
            float total_rle_score;
            /* check if rle is better */
            match local_mp;
            local_mp->len = best_rle_snp->index - snp->index;
            local_mp->offset = 1;

            rle_score = f(local_mp, emd);
            total_rle_score = best_rle_snp->total_score + rle_score;

            if(snp->total_score > total_rle_score)
            {
                /*here it is good to do rle instead of crunch */
                snp->total_score = total_rle_score;
                snp->total_offset = best_rle_snp->total_offset + 1;
                snp->prev = best_rle_snp;

                *snp->match = *local_mp;
            }
        }
        /* end of rle optimization */


        prev_score = snp_arr[len]->total_score;
        prev_offset_sum = snp_arr[len]->total_offset;
        while (mp != NULL)
        {
            matchp next;
            int end_len;

            match tmp;

            next = mp->next;
            end_len = 1;
#if 0
            if(next != NULL)
            {
                end_len = next->len + (next->offset > 0);
            }
#endif
            *tmp = *mp;
#if 1
            tmp->next = NULL;
#endif
            for(tmp->len = mp->len; tmp->len >= end_len; --(tmp->len))
            {
                float score;
                float total_score;
                unsigned int total_offset;

                score = f(tmp, emd);
                total_score = prev_score + score;
                total_offset = prev_offset_sum + tmp->offset;
                snp = snp_arr[len - tmp->len];

                if ((total_score < 100000000.0) &&
                    (snp->match->len == 0 ||
                     total_score < snp->total_score ||
                     (total_score == snp->total_score &&
#if 1
                      (tmp->offset == 0 ||
                       (snp->match->len == tmp->len &&
                        (total_offset <= snp->total_offset))))))
#endif
                {
                    snp->index = len - tmp->len;

                    *snp->match = *tmp;
                    snp->total_offset = total_offset;
                    snp->total_score = total_score;
                    snp->prev = snp_arr[len];
                }
                //LOG(LOG_DUMP, ("\n"));
            }
            mp = next;
        }

        /* slow way to get to the next node for cur */
        --len;
        ++best_copy_len;
    }
    if(len > 0 && mp == NULL)
    {
        LOG(LOG_ERROR, ("No matches at len %d.\n", len));
    }


    return snp_arr[0];
}

void matchp_snp_get_enum(const_search_nodep snp,        /* IN */
                         matchp_snp_enum snpe)  /* IN/OUT */
{
    snpe->startp = snp;
    snpe->currp = snp;
}

const_matchp matchp_snp_enum_get_next(void *matchp_snp_enum)
{
    matchp_snp_enump snpe;
    const_matchp val;

    snpe = matchp_snp_enum;

    val = NULL;
    while (snpe->currp != NULL && val == NULL)
    {
        val = snpe->currp->match;
        snpe->currp = snpe->currp->prev;
    }

    if (snpe->currp == NULL)
    {
        snpe->currp = snpe->startp;
    }
    return val;
}

void
chunkpool_init(struct chunkpool *ctx, int size)
{
    ctx->chunk_size = size;
    ctx->chunk = -1;
    ctx->chunk_max = (0x1fffff / size) * size;
    ctx->chunk_pos = ctx->chunk_max;
}

void
chunkpool_free2(struct chunkpool *ctx, cb_free *f)
{
    while(ctx->chunk >= 0)
    {
        if(f != NULL)
        {
            do
            {
                ctx->chunk_pos -= ctx->chunk_size;
                f((char*)ctx->chunks[ctx->chunk] + ctx->chunk_pos);
            }
            while(ctx->chunk_pos > 0);
            ctx->chunk_pos = ctx->chunk_max;
        }
	free(ctx->chunks[ctx->chunk]);
	ctx->chunk -= 1;
    }
    ctx->chunk_size = -1;
    ctx->chunk_max = -1;
    ctx->chunk_pos = -1;
}

void
chunkpool_free(struct chunkpool *ctx)
{
    chunkpool_free2(ctx, NULL);
}

void *
chunkpool_malloc(struct chunkpool *ctx)
{
    void *p;
    if(ctx->chunk_pos == ctx->chunk_max)
    {
	void *m;
	if(ctx->chunk == CHUNKPOOL_CHUNKS_MAX - 1)
	{
	    LOG(LOG_ERROR, ("out of chunks in file %s, line %d\n",
			    __FILE__, __LINE__));
	    LOG(LOG_BRIEF, ("chunk_size %d\n", ctx->chunk_size));
	    LOG(LOG_BRIEF, ("chunk_max %d\n", ctx->chunk_max));
	    LOG(LOG_BRIEF, ("chunk %d\n", ctx->chunk));
	    exit(-1);
	}
	m = malloc(ctx->chunk_max);
	if (m == NULL)
	{
	    LOG(LOG_ERROR, ("out of memory error in file %s, line %d\n",
			    __FILE__, __LINE__));
	    exit(-1);
	}
	ctx->chunk += 1;
	ctx->chunks[ctx->chunk] = m;
	ctx->chunk_pos = 0;
    }
    p = (char*)ctx->chunks[ctx->chunk] + ctx->chunk_pos;
    ctx->chunk_pos += ctx->chunk_size;
    return p;
}

void *
chunkpool_calloc(struct chunkpool *ctx)
{
    void *p = chunkpool_malloc(ctx);
    memset(p, 0, ctx->chunk_size);
    return p;
}

static struct crunch_options default_options[1] = { CRUNCH_OPTIONS_DEFAULT };

int do_output(match_ctx ctx,
              search_nodep snp,
              encode_match_data emd,
              encode_match_f * f,
              struct membuf *outbuf,
              int *literal_sequences_used)
{
    int pos;
    int pos_diff;
    int max_diff;
    int diff;
    int copy_used = 0;
    output_ctxp old;
    output_ctx out;

    output_ctx_init(out, outbuf);
    old = emd->out;
    emd->out = out;

    pos = output_get_pos(out);

    pos_diff = pos;
    max_diff = 0;

    output_gamma_code(out, 16);
    output_bits(out, 1, 0); /* 1 bit out */

    diff = output_get_pos(out) - pos_diff;
    if(diff > max_diff)
    {
        max_diff = diff;
    }

    while (snp != NULL)
    {
        const_matchp mp;

        mp = snp->match;
        if (mp != NULL && mp->len > 0)
        {
            if (mp->offset == 0)
            {
                if(mp->len == 1)
                {
                    /* literal */
                    output_byte(out, ctx->buf[snp->index]);
                    output_bits(out, 1, 1);
                } else
                {
                    int i;
                    for(i = 0; i < mp->len; ++i)
                    {
                        output_byte(out, ctx->buf[snp->index + i]);
                    }
                    output_bits(out, 16, mp->len);
                    output_gamma_code(out, 17);
                    output_bits(out, 1, 0);
                    copy_used = 1;
                }
            } else
            {
                f(mp, emd);
                output_bits(out, 1, 0);
            }

            pos_diff += mp->len;
            diff = output_get_pos(out) - pos_diff;
            if(diff > max_diff)
            {
                max_diff = diff;
            }
        }
        snp = snp->prev;
    }

    /* output header here */
    optimal_out(out, emd);

    output_bits_flush(out);

    emd->out = old;

    if(literal_sequences_used != NULL)
    {
        *literal_sequences_used = copy_used;
    }

    return max_diff;
}

search_nodep
do_compress(match_ctx ctx, encode_match_data emd,
            const char *exported_encoding,
            int max_passes,
            int use_literal_sequences)
{
    matchp_cache_enum mpce;
    matchp_snp_enum snpe;
    search_nodep snp;
    search_nodep best_snp;
    int pass;
    float size;
    float old_size;
    char prev_enc[100];
    const char *curr_enc;

    pass = 1;
    prev_enc[0] = '\0';

    if(exported_encoding != NULL)
    {
        optimal_encoding_import(emd, exported_encoding);
    }
    else
    {
        matchp_cache_get_enum(ctx, mpce);
        optimal_optimize(emd, matchp_cache_enum_get_next, mpce);
    }

    best_snp = NULL;
    old_size = 100000000.0;

    for (;;)
    {
        snp = search_buffer(ctx, optimal_encode, emd,
                            use_literal_sequences);
        if (snp == NULL)
        {
            LOG(LOG_ERROR, ("error: search_buffer() returned NULL\n"));
            exit(-1);
        }

        size = snp->total_score;

        if (size >= old_size)
        {
            search_node_free(snp);
            break;
        }

        if (best_snp != NULL)
        {
            search_node_free(best_snp);
        }
        best_snp = snp;
        old_size = size;
        ++pass;

        if(pass > max_passes)
        {
            break;
        }

        optimal_free(emd);
        optimal_init(emd);

        matchp_snp_get_enum(snp, snpe);
        optimal_optimize(emd, matchp_snp_enum_get_next, snpe);

        curr_enc = optimal_encoding_export(emd);
        if (strcmp(curr_enc, prev_enc) == 0)
        {
            break;
        }
        strcpy(prev_enc, curr_enc);
    }

    return best_snp;
}

void match_ctx_init(match_ctx ctx,         /* IN/OUT */
                    struct membuf *inbuf,  /* IN */
                    int max_len,           /* IN */
                    int max_offset,        /* IN */
                    int use_imprecise_rle); /* IN */
void crunch_backwards(struct membuf *inbuf,
                      struct membuf *outbuf,
                      struct crunch_options *options, /* IN */
                      struct crunch_info *info) /* OUT */
{
    static match_ctx ctx;
    encode_match_data emd;
    search_nodep snp;
    int outlen;
    int safety;
    int copy_used;

    if(options == NULL)
    {
        options = default_options;
    }

    outlen = membuf_memlen(outbuf);
    emd->out = NULL;
    optimal_init(emd);

    //LOG(LOG_NORMAL, (" Length of indata: %d bytes.\n", membuf_memlen(inbuf)));

    match_ctx_init(ctx, inbuf, options->max_len, options->max_offset,
                   options->use_imprecise_rle);


    emd->out = NULL;
    optimal_init(emd);

    snp = do_compress(ctx, emd, options->exported_encoding,
                      options->max_passes, options->use_literal_sequences);

    safety = do_output(ctx, snp, emd, optimal_encode, outbuf, &copy_used);
    //LOG(LOG_NORMAL, (" Length of crunched data: %d bytes.\n",membuf_memlen(outbuf) - outlen));

    optimal_free(emd);
    search_node_free(snp);
    match_ctx_free(ctx);

    if(info != NULL)
    {
        info->literal_sequences_used = copy_used;
        info->needed_safety_offset = safety;
    }
}

void reverse_buffer(char *start, int len)
{
    char *end = start + len - 1;
    char tmp;

    while (start < end)
    {
        tmp = *start;
        *start = *end;
        *end = tmp;

        ++start;
        --end;
    }
}

void exocrunch(struct membuf *inbuf,
            struct membuf *outbuf,
            struct crunch_options *options, /* IN */
            struct crunch_info *info) /* OUT */
{
    int outpos;
    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    outpos = membuf_memlen(outbuf);

    crunch_backwards(inbuf, outbuf, options, info);

    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    reverse_buffer((char*)membuf_get(outbuf) + outpos,
                   membuf_memlen(outbuf) - outpos);
}

void exodecrunch(int level,
              struct membuf *inbuf,
              struct membuf *outbuf)
{
    struct dec_ctx ctx[1];
    char *enc;
    enc = dec_ctx_init(ctx, inbuf, outbuf);

    LOG(level, (" Encoding: %s\n", enc));

    dec_ctx_decrunch(ctx);
    dec_ctx_free(ctx);
}

void decrunch_backwards(int level,
                        struct membuf *inbuf,
                        struct membuf *outbuf)
{
    int outpos;
    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    outpos = membuf_memlen(outbuf);

    exodecrunch(level, inbuf, outbuf);

    reverse_buffer(membuf_get(inbuf), membuf_memlen(inbuf));
    reverse_buffer((char*)membuf_get(outbuf) + outpos,
                   membuf_memlen(outbuf) - outpos);
}

void print_license(void)
{
}

void print_base_flags(enum log_level level, const char *default_outfile)
{
    LOG(level,
        ("  -o <outfile>  sets the outfile name, default is \"%s\"\n",
         default_outfile));
    LOG(level,
        ("  -q            quiet mode, disables display output\n"
         "  -v            displays version and the usage license\n"
         "  --            treats all following arguments as non-options\n"
         "  -?            displays this help screen\n"));
}

void print_crunch_flags(enum log_level level, const char *default_outfile)
{
    LOG(level,
        ("  -c            compatibility mode, disables the use of literal sequences\n"
         "  -C            enable imprecise rle matching, trades result for speed\n"
         "  -e <encoding> uses the given encoding for crunching\n"
         "  -m <offset>   sets the maximum sequence offset, default is 65535\n"
         "  -M <length>   sets the maximum sequence length, default is 65535\n"
         "  -p <passes>   limits the number of optimization passes, default is 65535\n"));
    print_base_flags(level, default_outfile);
}

void handle_base_flags(int flag_char, /* IN */
                       const char *flag_arg, /* IN */
                       print_usage_f *print_usage, /* IN */
                       const char *appl, /* IN */
                       const char **default_outfilep) /* IN */
{
    switch(flag_char)
    {
    case 'o':
        *default_outfilep = flag_arg;
        break;
    case 'q':
        LOG_SET_LEVEL(LOG_BRIEF);
        break;
    case 'v':
        print_license();
        exit(0);
    default:
        if (flagflag != '?')
        {
            LOG(LOG_ERROR,
                ("error, invalid option \"-%c\"", flagflag));
            if (flagarg != NULL)
            {
                LOG(LOG_ERROR, (" with argument \"%s\"", flagarg));
            }
            LOG(LOG_ERROR, ("\n"));
        }
        print_usage(appl, LOG_BRIEF, *default_outfilep);
        exit(0);
    }
}

void handle_crunch_flags(int flag_char, /* IN */
                         const char *flag_arg, /* IN */
                         print_usage_f *print_usage, /* IN */
                         const char *appl, /* IN */
                         struct common_flags *flags) /* OUT */
{
    struct crunch_options *options = flags->options;
    switch(flag_char)
    {
    case 'c':
        options->use_literal_sequences = 0;
        break;
    case 'C':
        options->use_imprecise_rle = 1;
        break;
    case 'e':
        options->exported_encoding = flag_arg;
        break;
    case 'm':
        if (str_to_int(flag_arg, &options->max_offset) != 0 ||
            options->max_offset < 0 || options->max_offset >= 65536)
        {
            LOG(LOG_ERROR,
                ("Error: invalid offset for -m option, "
                 "must be in the range of [0 - 65535]\n"));
            print_usage(appl, LOG_NORMAL, flags->outfile);
            exit(-1);
        }
        break;
    case 'M':
        if (str_to_int(flag_arg, &options->max_len) != 0 ||
            options->max_len < 0 || options->max_len >= 65536)
        {
            LOG(LOG_ERROR,
                ("Error: invalid offset for -n option, "
                 "must be in the range of [0 - 65535]\n"));
            print_usage(appl, LOG_NORMAL, flags->outfile);
            exit(-1);
        }
        break;
    case 'p':
        if (str_to_int(flag_arg, &options->max_passes) != 0 ||
            options->max_passes < 1 || options->max_passes >= 65536)
        {
            LOG(LOG_ERROR,
                ("Error: invalid value for -p option, "
                 "must be in the range of [1 - 65535]\n"));
            print_usage(appl, LOG_NORMAL, flags->outfile);
            exit(-1);
        }
        break;
    default:
        handle_base_flags(flag_char, flag_arg, print_usage,
                          appl, &flags->outfile);
    }
}

int find_sys(const unsigned char *buf, int target)
{
    int outstart = -1;
    int state = 1;
    int i = 0;
    /* skip link and line number */
    buf += 4;
    /* exit loop at line end */
    while(i < 1000 && buf[i] != '\0')
    {
        unsigned char *sys_end;
        int c = buf[i];
        switch(state)
        {
            /* look for and consume sys token */
        case 1:
            if((target == -1 &&
                (c == 0x9e /* cbm */ ||
                 c == 0x8c /* apple 2*/ ||
                 c == 0xbf /* oric 1*/)) ||
               c == target)
            {
                state = 2;
            }
            break;
            /* skip spaces and left parenthesis, if any */
        case 2:
            if(strchr(" (", c) != NULL) break;
            state = 3;
            /* convert string number to int */
        case 3:
            outstart = strtol((char*)(buf + i), (void*)&sys_end, 10);
            if((buf + i) == sys_end)
            {
                /* we got nothing */
                outstart = -1;
            }
            state = 4;
            break;
        case 4:
            break;
        }
        ++i;
    }

    return outstart;
}

static int ExoUtil_get_byte(FILE *in)
{
    int byte = fgetc(in);
    if(byte == EOF)
    {
        LOG(LOG_ERROR, ("Error: unexpected end of xex-file."));
        fclose(in);
        exit(-1);
    }
    return byte;
}

static int get_le_word(FILE *in)
{
    int word = ExoUtil_get_byte(in);
    word |= ExoUtil_get_byte(in) << 8;
    return word;
}

static int get_be_word(FILE *in)
{
    int word = ExoUtil_get_byte(in) << 8;
    word |= ExoUtil_get_byte(in);
    return word;
}

static
FILE *
open_file(char *name, int *load_addr)
{
    FILE * in;
    int is_plain = 0;
    int is_relocated = 0;
    int load = -3;

    do
    {
        char *load_str;
        char *at_str;

        in = fopen(name, "rb");
        if (in != NULL)
        {
            /* We have succeded in opening the file.
             * There's no address suffix. */
            break;
        }

        /* hmm, let's see if the user is trying to relocate it */
        load_str = strrchr(name, ',');
        at_str = strrchr(name, '@');
        if(at_str != NULL && (load_str == NULL || at_str > load_str))
        {
            is_plain = 1;
            load_str = at_str;
        }

        if (load_str == NULL)
        {
            /* nope, */
            break;
        }

        *load_str = '\0';
        ++load_str;
        is_relocated = 1;

        /* relocation was requested */
        if (str_to_int(load_str, &load) != 0)
        {
            /* we fail */
            LOG(LOG_ERROR,
                (" can't parse load address from \"%s\"\n", load_str));
            exit(-1);
        }

        in = fopen(name, "rb");

    } while (0);
    if (in == NULL)
    {
        LOG(LOG_ERROR,
            (" can't open file \"%s\" for input\n", name));
        exit(-1);
    }

    if(!is_plain)
    {
        /* read the prg load address */
        int prg_load = get_le_word(in);
        if(!is_relocated)
        {
            load = prg_load;
            /* unrelocated prg loading to $ffff is xex */
            if(prg_load == 0xffff)
            {
                /* differentiate this from relocated $ffff files so it is
                 * possible to override the xex auto-detection. */
                load = -1;
            }
            /* unrelocated prg loading to $1616 is Oric tap */
            else if(prg_load == 0x1616)
            {
                load = -2;
            }
        }
    }

    if(load_addr != NULL)
    {
        *load_addr = load;
    }
    return in;
}

static void load_xex(unsigned char mem[65536], FILE *in,
                     struct load_info *info)
{
    int run = -1;
    int jsr = -1;
    int min = 65536, max = 0;

    goto initial_state;
    for(;;)
    {
        int start, end, len;

        start = fgetc(in);
        if(start == EOF) break;
        ungetc(start, in);

        start = get_le_word(in);
        if(start == 0xffff)
        {
            /* allowed optional header */
        initial_state:
            start = get_le_word(in);
        }
        end = get_le_word(in);
        if(start > 0xffff || end > 0xffff || end < start)
        {
            LOG(LOG_ERROR, ("Error: corrupt data in xex-file."));
            fclose(in);
            exit(-1);
        }
        if(start == 0x2e2 && end == 0x2e3)
        {
            /* init vector */
            jsr = get_le_word(in);
            LOG(LOG_VERBOSE, ("Found xex initad $%04X.\n", jsr));
            continue;
        }
        if(start == 0x2e0 && end == 0x2e1)
        {
            /* run vector */
            run = get_le_word(in);
            LOG(LOG_VERBOSE, ("Found xex runad $%04X.\n", run));
            continue;
        }
        ++end;
        jsr = -1;
        if(start < min) min = start;
        if(end > max) max = end;

        len = fread(mem + start, 1, end - start, in);
        if(len != end - start)
        {
            LOG(LOG_ERROR, ("Error: unexpected end of xex-file.\n"));
            fclose(in);
            exit(-1);
        }
        LOG(LOG_VERBOSE, (" xex chunk loading from $%04X to $%04X\n",
                          start, end));
    }

    if(run == -1 && jsr != -1) run = jsr;

    info->start = min;
    info->end = max;
    info->basic_var_start = -1;
    info->run = -1;
    if(run != -1)
    {
        info->run = run;
    }
}

static void load_oric_tap(unsigned char mem[65536], FILE *in,
                          struct load_info *info)
{
    int c;
    int autostart;
    int start, end, len;

    /* read oric tap header */

    /* next byte must be 0x16 as we have already read two and must
     * have at least three */
    if(ExoUtil_get_byte(in) != 0x16)
    {
        LOG(LOG_ERROR, ("Error: fewer than three lead-in bytes ($16) "
                        "in Oric tap-file header.\n"));
        fclose(in);
        exit(-1);
    }
    /* optionally more 0x16 bytes */
    while((c = ExoUtil_get_byte(in)) == 0x16);
    /* next byte must be 0x24 */
    if(c != 0x24)
    {
        LOG(LOG_ERROR, ("Error: bad sync byte after lead-in in Oric tap-file "
                        "header, got $%02X but expected $24\n", c));
        fclose(in);
        exit(-1);
    }

    /* now we are in sync, lets be lenient */
    ExoUtil_get_byte(in); /* should be 0x0 */
    ExoUtil_get_byte(in); /* should be 0x0 */
    ExoUtil_get_byte(in); /* should be 0x0 or 0x80 */
    autostart = (ExoUtil_get_byte(in) != 0);  /* should be 0x0, 0x80 or 0xc7 */
    end = get_be_word(in) + 1; /* the header end address is inclusive */
    start = get_be_word(in);
    ExoUtil_get_byte(in); /* should be 0x0 */
    /* read optional file name */
    while(ExoUtil_get_byte(in) != 0x0);

    /* read the data */
    len = fread(mem + start, 1, end - start, in);
    if(len != end - start)
    {
        LOG(LOG_BRIEF, ("Warning: Oric tap-file contains %d byte(s) data "
                        "less than expected.\n", end - start - len));
        end = start + len;
    }
    LOG(LOG_VERBOSE, (" Oric tap-file loading from $%04X to $%04X\n",
                      start, end));

    /* fill in the fields */
    info->start = start;
    info->end = end;
    info->run = -1;
    info->basic_var_start = -1;
    if(autostart)
    {
        info->run = start;
    }
    if(info->basic_txt_start >= start &&
       info->basic_txt_start < end)
    {
        info->basic_var_start = end - 1;
    }
}

static void load_prg(unsigned char mem[65536], FILE *in,
                     struct load_info *info)
{
    int len;
    len = fread(mem + info->start, 1, 65536 - info->start, in);

    info->end = info->start + len;
    info->basic_var_start = -1;
    info->run = -1;
    if(info->basic_txt_start >= info->start &&
       info->basic_txt_start < info->end)
    {
        info->basic_var_start = info->end;
    }
}

void load_located(char *filename, unsigned char mem[65536],
                  struct load_info *info)
{
    int load;
    FILE *in;

    in = open_file(filename, &load);
    if(load == -1)
    {
        /* file is an xex file */
        load_xex(mem, in, info);
    }
    else if(load == -2)
    {
        /* file is an oric tap file */
        load_oric_tap(mem, in, info);
    }
    else
    {
        /* file is a located plain file or a prg file */
        info->start = load;
        load_prg(mem, in, info);
    }
    fclose(in);

    LOG(LOG_NORMAL,
        (" filename: \"%s\", loading from $%04X to $%04X\n",
         filename, info->start, info->end));
}

/* returns 0 if ok, 1 otherwise */
int str_to_int(const char *str, int *value)
{
    int status = 0;
    do {
        char *str_end;
        long lval;

        /* base 0 is auto detect */
        int base = 0;

        if (*str == '\0')
        {
            /* no string to parse */
            status = 1;
            break;
        }

        if (*str == '$')
        {
            /* a $ prefix specifies base 16 */
            ++str;
            base = 16;
        }

        lval = strtol(str, &str_end, base);

        if(*str_end != '\0')
        {
            /* there is garbage in the string */
            status = 1;
            break;
        }

        if(value != NULL)
        {
            /* all is well, set the out parameter */
            *value = (int)lval;
        }
    } while(0);

    return status;
}

const char *fixup_appl(char *appl)
{
    char *applp;

    /* strip pathprefix from appl */
    applp = strrchr(appl, '\\');
    if (applp != NULL)
    {
        appl = applp + 1;
    }
    applp = strrchr(appl, '/');
    if (applp != NULL)
    {
        appl = applp + 1;
    }
    /* strip possible exe suffix */
    applp = appl + strlen(appl) - 4;
    if(strcmp(applp, ".exe") == 0 || strcmp(applp, ".EXE") == 0)
    {
        *applp = '\0';
    }
    return appl;
}

char *get(struct membuf *buf)
{
    return membuf_get(buf);
}

int get_byte(struct dec_ctx *ctx)
{
    int c;
    if(ctx->inpos == ctx->inend)
    {
        LOG(LOG_ERROR, ("unexpected end of input data\n"));
        exit(-1);
    }
    c = ctx->inbuf[ctx->inpos++];

    return c;
}

int
get_bits(struct dec_ctx *ctx, int count)
{
    int val;

    val = 0;

    /*printf("get_bits: count = %d", count);*/
    while(count-- > 0) {
        if((ctx->bitbuf & 0x1FF) == 1) {
            ctx->bitbuf = get_byte(ctx) | 0x100;
        }
        val <<= 1;
        val |= ctx->bitbuf & 0x1;
        ctx->bitbuf >>= 1;
        /*printf("bit read %d\n", val &1);*/
        ctx->bits_read++;
    }
    /*printf(" val = %d\n", val);*/
    return val;
}

int
get_gamma_code(struct dec_ctx *ctx)
{
    int gamma_code;
    /* get bitnum index */
    gamma_code = 0;
    while(get_bits(ctx, 1) == 0)
    {
        ++gamma_code;
    }
    return gamma_code;
}

int
get_cooked_code_phase2(struct dec_ctx *ctx, int index)
{
    int base;
    struct dec_table *tp;
    tp = ctx->t;

    base = tp->table_lo[index] | (tp->table_hi[index] << 8);
    return base + get_bits(ctx, tp->table_bi[index]);
}

static
void
table_init(struct dec_ctx *ctx, struct dec_table *tp) /* IN/OUT */
{
    int i;
    unsigned int a = 0;
    unsigned int b = 0;

    tp->table_bit[0] = 2;
    tp->table_bit[1] = 4;
    tp->table_bit[2] = 4;

    tp->table_off[0] = 48;
    tp->table_off[1] = 32;
    tp->table_off[2] = 16;

    for(i = 0; i < 52; ++i)
    {
        if(i & 0xF)
        {
            a += 1 << b;
        } else
        {
            a = 1;
        }

        tp->table_lo[i] = a & 0xFF;
        tp->table_hi[i] = a >> 8;

        b = get_bits(ctx, 4);

        tp->table_bi[i] = b;

    }
}

char *
table_dump(struct dec_table *tp)
{
    int i, j;
    static char buf[100];
    char *p = buf;

    for(i = 0; i < 16; ++i)
    {
        p += sprintf(p, "%X", tp->table_bi[i]);
    }
    for(j = 0; j < 3; ++j)
    {
        int start;
        int end;
        p += sprintf(p, ",");
        start = tp->table_off[j];
        end = start + (1 << tp->table_bit[j]);
        for(i = start; i < end; ++i)
        {
            p += sprintf(p, "%X", tp->table_bi[i]);
        }
    }
    return buf;
}

char *
dec_ctx_init(struct dec_ctx *ctx, struct membuf *inbuf, struct membuf *outbuf)
{
    char *encoding;
    ctx->bits_read = 0;

    ctx->inbuf = membuf_get(inbuf);
    ctx->inend = membuf_memlen(inbuf);
    ctx->inpos = 0;

    ctx->outbuf = outbuf;

    /* init bitbuf */
    ctx->bitbuf = get_byte(ctx);

    /* init tables */
    table_init(ctx, ctx->t);
    encoding = table_dump(ctx->t);
    return encoding;
}

void dec_ctx_free(struct dec_ctx *ctx)
{
}

void dec_ctx_decrunch(struct dec_ctx ctx[1])
{
    int bits;
    int val;
    int i;
    int len;
    int offset;
    int src = 0;

    for(;;)
    {
        int literal = 0;
        bits = ctx->bits_read;
        if(get_bits(ctx, 1))
        {
            /* literal */
            len = 1;

            literal = 1;
            goto literal;
        }

        val = get_gamma_code(ctx);
        if(val == 16)
        {
            /* done */
            break;
        }
        if(val == 17)
        {
            len = get_bits(ctx, 16);
            literal = 1;

            goto literal;
        }

        len = get_cooked_code_phase2(ctx, val);

        i = (len > 3 ? 3 : len) - 1;

        val = ctx->t->table_off[i] + get_bits(ctx, ctx->t->table_bit[i]);
        offset = get_cooked_code_phase2(ctx, val);

        src = membuf_memlen(ctx->outbuf) - offset;

    literal:
        do {
            if(literal)
            {
                val = get_byte(ctx);
            }
            else
            {
                val = get(ctx->outbuf)[src++];
            }
            membuf_append_char(ctx->outbuf, val);
        } while (--len > 0);

    }
}

int flagind = 1;
int flagflag = '?';
const char *flagarg = NULL;

static void reverse(char **buf, int pos1, int pos2)
{
    char **buf1;
    char **buf2;
    char *tmp;

    buf1 = buf + pos1;
    buf2 = buf + pos2 - 1;

    while (buf1 < buf2)
    {
        tmp = *buf1;
        *buf1 = *buf2;
        *buf2 = tmp;

        ++buf1;
        --buf2;
    }
}


int getflag(int argc, char **argv, const char *flags)
{
    int argstart, flagstart, c;
    const char *flagp;

    c = -1;
    flagarg = NULL;
    argstart = flagind;
    flagstart = argc;

    /* skip over non-flags */
    while (flagind < argc && argv[flagind][0] != '-')
    {
        ++flagind;
    }
    if (flagind == argc)
    {
        /* no more args */
        flagind = argstart;
        return c;
    }
    /* we have an arg to work with */
    do
    {
        flagstart = flagind;
        if (argv[flagind][1] == '-' && argv[flagind][2] == '\0')
        {
            /* stop parsing at '--' flag */
            break;
        }
        c = flagflag = argv[flagind][1];
        if (c == ':' || c == '\0')
        {
            /* this is an illegal flag */
            c = '?';
            break;
        }
        /* flag with arg */
        if (argv[flagind][2] != '\0')
        {
            /* flag-arg in same argv[] */
            flagarg = argv[flagind] + 2;
        }
        flagp = strchr(flags, c);
        if (flagp == NULL)
        {
            /* this is an unknown flag */
            c = '?';
            break;
        }
        if (flagarg != NULL || flagp[1] != ':')
        {
            if (flagarg != NULL && flagp[1] != ':')
            {
                /* error, a simple flag with an argument */
                c = '?';
            }
            break;
        }

        /* flag-arg is in the next argv[] */
        if (flagind + 1 == argc)
        {
            /* auahh, no flag-arg */
            flagstart = argstart;
            c = '?';
            break;
        }
        flagarg = argv[++flagind];
    } while (0);
    /* skip to next arg */
    ++flagind;

    if (flagstart < flagind && argstart < flagstart)
    {
        /* we have found some args
         * we have also skipped over some non-args
         * shuffle the non-flag arg to the end of argv */
        reverse(argv, argstart, flagstart);
        reverse(argv, flagstart, flagind);
        reverse(argv, argstart, flagind);
    }
    flagind = argstart + flagind - flagstart;

    return c;
}

struct log_output {
    enum log_level min;
    enum log_level max;
    FILE *stream;
    log_formatter_f *f;
};

struct log_ctx {
    enum log_level level;
    int out_len;
    struct log_output *out;
    int buf_len;
    char *buf;
};

struct log_ctx *G_log_ctx = NULL;
enum log_level G_log_level = LOG_MIN;
enum log_level G_log_log_level = 0;

struct log_ctx *log_new(void)
{
    struct log_ctx *ctx;

    ctx = malloc(sizeof(*ctx));
    if (ctx == NULL)
    {
        fprintf(stderr,
                "fatal error, can't allocate memory for log context\n");
        exit(1);
    }
    ctx->level = LOG_NORMAL;
    ctx->out_len = 0;
    ctx->out = NULL;
    ctx->buf_len = 0;
    ctx->buf = NULL;

    return ctx;
}

/* log_delete closes all added output streams
 * and files except for stdout and stderr
 */
void log_delete(struct log_ctx *ctx)
{
    int i;

    for (i = 0; i < ctx->out_len; ++i)
    {
        FILE *file = ctx->out[i].stream;
        if (file != stderr && file != stdout)
        {
            fclose(file);
        }
    }
    free(ctx->out);
    free(ctx->buf);
    free(ctx);
}

void log_set_level(struct log_ctx *ctx, /* IN/OUT */
                   enum log_level level)        /* IN */
{
    ctx->level = level;
}

void log_add_output_stream(struct log_ctx *ctx, /* IN/OUT */
                           enum log_level min,  /* IN */
                           enum log_level max,  /* IN */
                           log_formatter_f * default_f, /* IN */
                           FILE * out_stream)   /* IN */
{
    struct log_output *out;

    ctx->out_len += 1;
    ctx->out = realloc(ctx->out, ctx->out_len * sizeof(*(ctx->out)));
    if (ctx->out == NULL)
    {
        fprintf(stderr,
                "fatal error, can't allocate memory for log output\n");
        exit(1);
    }
    out = &(ctx->out[ctx->out_len - 1]);
    out->min = min;
    out->max = max;
    out->stream = out_stream;
    out->f = default_f;
}

void raw_log_formatter(FILE * out,      /* IN */
                       enum log_level level,    /* IN */
                       const char *context,     /* IN */
                       const char *log) /* IN */
{
    fprintf(out, "%s", log);
    fflush(out);
}

void log_vlog(struct log_ctx *ctx,      /* IN */
              enum log_level level,     /* IN */
              const char *context,      /* IN */
              log_formatter_f * f,      /* IN */
              const char *printf_str,   /* IN */
              va_list argp)
{
    int len;
    int i;

    if (ctx->level < level)
    {
        /* don't log this */
        return;
    }

    len = 0;
    do
    {
        if (len >= ctx->buf_len)
        {
            ctx->buf_len = len + 1024;
            ctx->buf = realloc(ctx->buf, ctx->buf_len);
            if (ctx->buf == NULL)
            {
                fprintf(stderr,
                        "fatal error, can't allocate memory for log log\n");
                exit(1);
            }
        }
        len = vsnprintf(ctx->buf, ctx->buf_len, printf_str, argp);
    }

    while (len >= ctx->buf_len);

    for (i = 0; i < ctx->out_len; ++i)
    {
        struct log_output *o = &ctx->out[i];
        log_formatter_f *of = f;

        if (level >= o->min && level <= o->max)
        {
            /* generate log for this output */
            if (of == NULL)
            {
                of = o->f;
            }
            if (of != NULL)
            {
                of(o->stream, level, context, ctx->buf);
            } else
            {
                fprintf(o->stream, "%s\n", ctx->buf);
                fflush(o->stream);
            }
        }
    }
}

void log_log_default(const char *printf_str,    /* IN */
                     ...)
{
    va_list argp;
    va_start(argp, printf_str);
    log_vlog(G_log_ctx, G_log_log_level,
             NULL, raw_log_formatter, printf_str, argp);
}

void log_log(struct log_ctx *ctx,       /* IN */
             enum log_level level,      /* IN */
             const char *context,       /* IN */
             log_formatter_f * f,       /* IN */
             const char *printf_str,    /* IN */
             ...)
{
    va_list argp;
    va_start(argp, printf_str);
    log_vlog(ctx, level, context, f, printf_str, argp);
}

void hex_dump(int level, unsigned char *p, int len)
{
#if 0
    int i;
    int j;
    for(i = 0; i < len;)
    {
        LOG(level, ("%02x", p[i]));
        ++i;
        if(i == len || (i & 15) == 0)
        {
            LOG(level, ("\t\""));
            for(j = (i - 1) & ~15; j < i; ++j)
            {
                unsigned char c = p[j];
                if(!isprint(c))
                {
                    c = '.';
                }
                LOG(level, ("%c", c));
            }
            LOG(level, ("\"\n"));
        }
        else
        {
            LOG(level, (","));
        }
    }
#endif
}

struct match_node {
    int index;
    struct match_node *next;
};

static
const_matchp matches_calc(match_ctx ctx,        /* IN/OUT */
                          int index);    /* IN */

matchp match_new(match_ctx ctx, /* IN/OUT */
                 matchp *mpp,
                 int len,
                 int offset)
{
    matchp m = chunkpool_malloc(ctx->m_pool);

    if(len == 0)
    {
        LOG(LOG_ERROR, ("tried to allocate len0 match.\n"));
        *(volatile int*)0;
    }
    if(len > 65535)
    {
        len = 65535;
    }

    m->len = len;
    m->offset = offset;

    /* insert new node in list */
    m->next = *mpp;
    *mpp = m;

    return m;
}


void match_ctx_init(match_ctx ctx,         /* IN/OUT */
                    struct membuf *inbuf,  /* IN */
                    int max_len,           /* IN */
                    int max_offset,        /* IN */
                    int use_imprecise_rle) /* IN */
{
    struct match_node *np;

    int buf_len = membuf_memlen(inbuf);
    const unsigned char *buf = membuf_get(inbuf);

    int c, i;
    int val;

    ctx->info = calloc(buf_len + 1, sizeof(*ctx->info));
    ctx->rle = calloc(buf_len + 1, sizeof(*ctx->rle));
    ctx->rle_r = calloc(buf_len + 1, sizeof(*ctx->rle_r));

    chunkpool_init(ctx->m_pool, sizeof(match));

    ctx->max_offset = max_offset;
    ctx->max_len = max_len;

    ctx->buf = buf;
    ctx->len = buf_len;

    val = buf[0];
    for (i = 1; i < buf_len; ++i)
    {
        if (buf[i] == val)
        {
            int len = ctx->rle[i - 1] + 1;
            if(len > 65535)
            {
                len = 0;
            }
            ctx->rle[i] = len;
        } else
        {
            ctx->rle[i] = 0;
        }
        val = buf[i];
    }

    for (i = buf_len - 2; i >= 0; --i)
    {
        if (ctx->rle[i] < ctx->rle[i + 1])
        {
            ctx->rle_r[i] = ctx->rle_r[i + 1] + 1;
        } else
        {
            ctx->rle_r[i] = 0;
        }
    }

    /* add extra nodes to rle sequences */
    for(c = 0; c < 256; ++c)
    {
        static char rle_map[65536];
        struct match_node *prev_np;
        unsigned short int rle_len;

        /* for each possible rle char */
        memset(rle_map, 0, sizeof(rle_map));
        prev_np = NULL;
        for (i = 0; i < buf_len; ++i)
        {
            /* must be the correct char */
            if(buf[i] != c)
            {
                continue;
            }

            rle_len = ctx->rle[i];
            if(!rle_map[rle_len] && ctx->rle_r[i] > 16)
            {
                /* no previous lengths and not our primary length*/
                continue;
            }

            if (use_imprecise_rle &&
                ctx->rle_r[i] != 0 && ctx->rle[i] != 0)
            {
                continue;
            }

            np = chunkpool_malloc(ctx->m_pool);
            np->index = i;
            np->next = NULL;
            rle_map[rle_len] = 1;

            LOG(LOG_DUMP, ("0) c = %d, added np idx %d -> %d\n", c, i, 0));

            /* if we have a previous entry, let's chain it together */
            if(prev_np != NULL)
            {
                LOG(LOG_DUMP, ("1) c = %d, pointed np idx %d -> %d\n",
                                c, prev_np->index, i));
                prev_np->next = np;
            }

            ctx->info[i]->single = np;
            prev_np = np;
        }

        memset(rle_map, 0, sizeof(rle_map));
        prev_np = NULL;
        for (i = buf_len - 1; i >= 0; --i)
        {
            /* must be the correct char */
            if(buf[i] != c)
            {
                continue;
            }

            rle_len = ctx->rle_r[i];
            np = ctx->info[i]->single;
            if(np == NULL)
            {
                if(rle_map[rle_len] && prev_np != NULL && rle_len > 0)
                {
                    np = chunkpool_malloc(ctx->m_pool);
                    np->index = i;
                    np->next = prev_np;
                    ctx->info[i]->single = np;

                    LOG(LOG_DEBUG, ("2) c = %d, added np idx %d -> %d\n",
                                    c, i, prev_np->index));
                }
            }
            else
            {
                prev_np = np;
            }

            if(ctx->rle_r[i] > 0)
            {
                continue;
            }
            rle_len = ctx->rle[i] + 1;
            rle_map[rle_len] = 1;
        }
    }


    for (i = buf_len - 1; i >= 0; --i)
    {
        const_matchp matches;

        /* let's populate the cache */
        matches = matches_calc(ctx, i);

        /* add to cache */
        ctx->info[i]->cache = matches;

    }

}

void match_ctx_free(match_ctx ctx)      /* IN/OUT */
{
    chunkpool_free(ctx->m_pool);
    free(ctx->info);
    free(ctx->rle);
    free(ctx->rle_r);
}

void dump_matches(int level, matchp mp)
{
    if (mp == NULL)
    {
        LOG(level, (" (NULL)\n"));
    } else
    {
        if(mp->offset > 0)
        {
            LOG(level, (" offset %d, len %d\n", mp->offset, mp->len));
        }
        if (mp->next != NULL)
        {
            dump_matches(level, mp->next);
        }
    }
}

const_matchp matches_get(match_ctx ctx, /* IN/OUT */
                         int index)      /* IN */
{
    return ctx->info[index]->cache;
}

/* this needs to be called with the indexes in
 * reverse order */
const_matchp matches_calc(match_ctx ctx,        /* IN/OUT */
                          int index)     /* IN */
{
    const unsigned char *buf;

    matchp matches;
    matchp mp;
    struct match_node *np;

    buf = ctx->buf;
    matches = NULL;

    LOG(LOG_DUMP, ("index %d, char '%c', rle %d, rle_r %d\n",
                   index, buf[index], ctx->rle[index],
                   ctx->rle_r[index]));

    /* proces the literal match and add it to matches */
    mp = match_new(ctx, &matches, 1, 0);

    /* get possible match */
    np = ctx->info[index]->single;
    if(np != NULL)
    {
        np = np->next;
    }
    for (; np != NULL; np = np->next)
    {
        int mp_len;
        int len;
        int pos;
        int offset;

        /* limit according to max offset */
        if(np->index > index + ctx->max_offset)
        {
            break;
        }

        LOG(LOG_DUMP, ("find lengths for index %d to index %d\n",
                        index, np->index));

        /* get match len */
        mp_len = mp->offset > 0 ? mp->len : 0;
        LOG(LOG_DUMP, ("0) comparing with current best [%d] off %d len %d\n",
                        index, mp->offset, mp_len));

        offset = np->index - index;
        len = mp_len;
        pos = index + 1 - len;
        /* Compare the first <previous len> bytes backwards. We can
         * skip some comparisons by increasing by the rle count. We
         * don't need to compare the first byte, hence > 1 instead of
         * > 0 */
        while(len > 1 && buf[pos] == buf[pos + offset])
        {
#if 1
            int offset1 = ctx->rle_r[pos];
            int offset2 = ctx->rle_r[pos + offset];
            int offset = offset1 < offset2 ? offset1 : offset2;

            LOG(LOG_DUMP, ("1) compared sucesssfully [%d] %d %d\n",
                            index, pos, pos + offset));

            len -= 1 + offset;
            pos += 1 + offset;
#else
            --len;
            ++pos;
#endif
        }
        if(len > 1)
        {
            /* sequence length too short, skip this match */
            continue;
        }

        if(offset < 17)
        {
            /* allocate match struct and add it to matches */
            mp = match_new(ctx, &matches, 1, offset);
        }

        /* Here we know that the current match is atleast as long as
         * the previuos one. let's compare further. */
        len = mp_len;
        pos = index - len;
        while(len <= ctx->max_len &&
              pos >= 0 && buf[pos] == buf[pos + offset])
        {
            LOG(LOG_DUMP, ("2) compared sucesssfully [%d] %d %d\n",
                            index, pos, pos + offset));
            ++len;
            --pos;
        }
        if(len > mp_len)
        {
            /* allocate match struct and add it to matches */
            mp = match_new(ctx, &matches, index - pos, offset);
        }
        if (len > ctx->max_len)
        {
            break;
        }
        if(pos < 0)
        {
            /* we have reached the eof, no better matches can be found */
            break;
        }
    }
    LOG(LOG_DEBUG, ("adding matches for index %d to cache\n", index));
    dump_matches(LOG_DEBUG, matches);

    return matches;
}

static
int
matchp_keep_this(const_matchp mp)
{
    int val = 1;
    /* if we want to ignore this matchp then return true else false */
    if(mp->len == 1)
    {
        if(mp->offset > 32)
        {
            val = 0;
        }
    }
    return val;
}

static
void
matchp_cache_peek(struct match_ctx *ctx, int pos,
                  const_matchp *litpp, const_matchp *seqpp,
                  matchp lit_tmp, matchp val_tmp)
{
    const_matchp litp, seqp, val;

    seqp = NULL;
    litp = NULL;
    if(pos >= 0)
    {
        val = matches_get(ctx, pos);
        litp = val;
        while(litp->offset != 0)
        {
            litp = litp->next;
        }

        /* inject extra rle match */
        if(ctx->rle_r[pos] > 0)
        {
            val_tmp->offset = 1;
            val_tmp->len = ctx->rle[pos] + 1;
            val_tmp->next = (matchp)val;
            val = val_tmp;
            LOG(LOG_DEBUG, ("injecting rle val(%d,%d)\n",
                            val->len, val->offset));
        }

        while(val != NULL)
        {
            if(val->offset != 0)
            {
                if(matchp_keep_this(val))
                {
                    if(seqp == NULL || val->len > seqp->len ||
                       (val->len == seqp->len && val->offset < seqp->offset))
                    {
                        seqp = val;
                    }
                }
                if(litp->offset == 0 || litp->offset > val->offset)
                {
                    LOG(LOG_DEBUG, ("val(%d,%d)", val->len, val->offset));
                    if(lit_tmp != NULL)
                    {
                        int diff;
                        match tmp2;
                        *tmp2 = *val;
                        tmp2->len = 1;
                        diff = ctx->rle[pos + val->offset];
                        if(tmp2->offset > diff)
                        {
                            tmp2->offset -= diff;
                        }
                        else
                        {
                            tmp2->offset = 1;
                        }
                        LOG(LOG_DEBUG, ("=> litp(%d,%d)",
                                        tmp2->len, tmp2->offset));
                        if(matchp_keep_this(tmp2))
                        {
                            LOG(LOG_DEBUG, (", keeping"));
                            *lit_tmp = *tmp2;
                            litp = lit_tmp;
                        }
                    }
                    LOG(LOG_DEBUG, ("\n"));
                }
            }
            val = val->next;
        }
    }
#if 0
    LOG(LOG_NORMAL, ("[%05d]: ", pos));
    if(litp == NULL)
        LOG(LOG_NORMAL, ("litp(NULL)"));
    else
        LOG(LOG_NORMAL, ("litp(%d,%d)", litp->len, litp->offset));

    if(seqp == NULL)
        LOG(LOG_NORMAL, ("seqp(NULL)"));
    else
        LOG(LOG_NORMAL, ("seqp(%d,%d)", seqp->len, seqp->offset));

    LOG(LOG_NORMAL, ("\n"));
#endif

    if(litpp != NULL) *litpp = litp;
    if(seqpp != NULL) *seqpp = seqp;
}

void matchp_cache_get_enum(match_ctx ctx,       /* IN */
                           matchp_cache_enum mpce)      /* IN/OUT */
{
    mpce->ctx = ctx;
    mpce->pos = ctx->len - 1;
    /*mpce->next = NULL;*/ /* two iterations */
    mpce->next = (void*)mpce; /* just one */
}

const_matchp matchp_cache_enum_get_next(void *matchp_cache_enum)
{
    const_matchp val, lit, seq;
    matchp_cache_enump mpce;

    mpce = matchp_cache_enum;

 restart:
    matchp_cache_peek(mpce->ctx, mpce->pos, &lit, &seq,
                      mpce->tmp1, mpce->tmp2);

    val = lit;
    if(lit == NULL)
    {
        /* the end, reset enum and return NULL */
        mpce->pos = mpce->ctx->len - 1;
        if(mpce->next == NULL)
        {
            mpce->next = (void*)mpce;
            goto restart;
        }
        else
        {
            mpce->next = NULL;
        }
    }
    else
    {
        if(seq != NULL)
        {
            match t1;
            match t2;
            const_matchp next;
            matchp_cache_peek(mpce->ctx, mpce->pos - 1, NULL, &next, t1 ,t2);
            if(next == NULL ||
               (next->len + (mpce->next != NULL && next->len < 3) <= seq->len))
            {
                /* nope, next is not better, use this sequence */
                val = seq;
            }
        }
    }
    if(val != NULL)
    {
        LOG(LOG_DEBUG, ("Using len %05d, offset, %05d\n", val->len, val->offset));
        mpce->pos -= val->len;
    }
    return val;
}



void read_file(const char *name, struct membuf *buf)
{
    char block[1024];
    FILE *in;
    int len;

    in = fopen(name, "rb");
    if(in == NULL)
    {
        LOG(LOG_ERROR, ("Can't open file \"%s\" for input.\n", name));
        exit(-1);
    }
    do
    {
        len = fread(block, 1, 1024, in);
        membuf_append(buf, block, len);
    }
    while(len == 1024);
    LOG(LOG_DEBUG, ("read %d bytes from file\n", len));
    fclose(in);
}

void write_file(const char *name, struct membuf *buf)
{
    FILE *out;
    out = fopen(name, "wb");
    if(out == NULL)
    {
        LOG(LOG_ERROR, ("Can't open file \"%s\" for output.\n", name));
        exit(-1);
    }
    fwrite(membuf_get(buf), 1, membuf_memlen(buf), out);
    fclose(out);
}
void membuf_init(struct membuf *sb)
{
    sb->buf = NULL;
    sb->len = 0;
    sb->size = 0;
}
void membuf_clear(struct membuf *sb)
{
    sb->len = 0;
}
void membuf_free(struct membuf *sb)
{
    if (sb->buf != NULL)
    {
        free(sb->buf);
        sb->buf = NULL;
    }
    sb->len = 0;
    sb->size = 0;
}

void membuf_new(struct membuf **sbp)
{
    struct membuf *sb;

    sb = malloc(sizeof(struct membuf));
    if (sb == NULL)
    {
        fprintf(stderr, "error, can't allocate memory\n");
        exit(1);
    }
    sb->buf = NULL;
    sb->len = 0;
    sb->size = 0;

    *sbp = sb;
}

void membuf_delete(struct membuf **sbp)
{
    struct membuf *sb;

    sb = *sbp;
    membuf_free(sb);
    free(sb);
    sb = NULL;
    *sbp = sb;
}

int membuf_memlen(const struct membuf *sb)
{
    return sb->len;
}

void membuf_truncate(struct membuf *sb, int len)
{
    sb->len = len;
}

int membuf_trim(struct membuf *sb, int pos)
{
    if(pos < 0 || pos > sb->len)
    {
        return -1;
    }
    if(pos == 0)
    {
        return sb->len;
    }
    if(pos != sb->len)
    {
        memmove(sb->buf, (char*)sb->buf + pos, sb->len - pos);
    }
    sb->len -= pos;
    return sb->len;
}

void *membuf_memcpy(struct membuf *sb, int offset, const void *mem, int len)
{
    char *buf;
    membuf_atleast(sb, offset + len);
    buf = (char*)sb->buf + offset;
    memcpy(buf, mem, len);
    return buf;
}

void *membuf_append(struct membuf *sb, const void *mem, int len)
{
    int newlen;
    void *p;
    newlen = sb->len + len;
    membuf_atleast(sb, newlen);
    p = (char *) sb->buf + sb->len;
    if(mem == NULL)
    {
        memset(p, 0, len);
    }
    else
    {
        memcpy(p, mem, len);
    }
    sb->len = newlen;
    return p;
}

void *membuf_append_char(struct membuf *sb, char c)
{
    int newlen;
    char *p;
    newlen = sb->len + 1;
    membuf_atleast(sb, newlen);
    p = (char *) sb->buf + sb->len;
    *p = c;
    sb->len = newlen;
    return p;
}

void *membuf_insert(struct membuf *sb, int offset, const void *mem, int len)
{
    int newlen;
    void *from;
    void *to;
    newlen = sb->len + len;
    membuf_atleast(sb, newlen);
    from = (char *)sb->buf + offset;
    to = (char *)from + len;
    memmove(to, from, sb->len - offset);
    if(mem == NULL)
    {
        memset(from, 0, len);
    }
    else
    {
        memcpy(from, mem, len);
    }
    sb->len = newlen;
    return from;
}

void membuf_remove(struct membuf *sb, int offset, int len)
{
    void *from;
    void *to;
    to = (char *)sb->buf + offset;
    from = (char *)to + len;
    sb->len -= len;
    memmove(to, from, sb->len - offset);

}

void membuf_atleast(struct membuf *sb, int len)
{
    int size;

    size = sb->size;
    if (size == 0)
        size = 1;
    while (size < len)
    {
        size <<= 1;
    }
    if (size > sb->size)
    {
        sb->buf = realloc(sb->buf, size);
        if (sb->buf == NULL)
        {
            fprintf(stderr, "error, can't reallocate memory\n");
            exit(1);
        }
        sb->size = size;
    }
}

void membuf_atmost(struct membuf *sb, int len)
{
    int size;

    size = sb->size;
    while (size > len)
    {
        size >>= 1;
    }
    if (size < sb->size)
    {
        sb->buf = realloc(sb->buf, size);
        if (sb->buf == NULL)
        {
            fprintf(stderr, "error, can't reallocate memory\n");
            exit(1);
        }
        sb->size = size;
        sb->len = size;
    }
}

int membuf_get_size(const struct membuf *sb)
{
    return sb->size;
}
void *membuf_get(const struct membuf *sb)
{
    return sb->buf;
}



















struct _interval_node {
    int start;
    int score;
    struct _interval_node *next;
    signed char prefix;
    signed char bits;
    signed char depth;
    signed char flags;
};

typedef struct _interval_node interval_node[1];
typedef struct _interval_node *interval_nodep;

static
void
interval_node_init(interval_nodep inp, int start, int depth, int flags)
{
    inp->start = start;
    inp->flags = flags;
    inp->depth = depth;
    inp->bits = 0;
    inp->prefix = flags >= 0 ? flags : depth + 1;
    inp->score = -1;
    inp->next = NULL;
}

static
interval_nodep interval_node_clone(interval_nodep inp)
{
    interval_nodep inp2 = NULL;

    if(inp != NULL)
    {
	inp2 = malloc(sizeof(interval_node));
	if (inp2 == NULL)
	{
	    LOG(LOG_ERROR, ("out of memory error in file %s, line %d\n",
			    __FILE__, __LINE__));
	    exit(0);
	}
	/* copy contents */
	*inp2 = *inp;
	inp2->next = interval_node_clone(inp->next);
    }

    return inp2;
}

static
void interval_node_delete(interval_nodep inp)
{
    interval_nodep inp2;
    while (inp != NULL)
    {
        inp2 = inp;
        inp = inp->next;
        free(inp2);
    }
}

static
void interval_node_dump(int level, interval_nodep inp)
{
    int end;

    end = 0;
    while (inp != NULL)
    {
        end = inp->start + (1 << inp->bits);
        LOG(level, ("%X", inp->bits));
        inp = inp->next;
    }
    LOG(level, ("[eol@%d]\n", end));
}

float optimal_encode_int(int arg, void *priv, output_ctxp out)
{
    interval_nodep inp;
    int end;

    float val;

    inp = (interval_nodep) priv;
    val = 100000000.0;
    end = 0;
    while (inp != NULL)
    {
        end = inp->start + (1 << inp->bits);
        if (arg >= inp->start && arg < end)
        {
            break;
        }
        inp = inp->next;
    }
    if (inp != NULL)
    {
        val = (float) (inp->prefix + inp->bits);
    } else
    {
        val += (float) (arg - end);
    }
    LOG(LOG_DUMP, ("encoding %d to %0.1f bits\n", arg, val));

    if (out != NULL)
    {
        output_bits(out, inp->bits, arg - inp->start);
        if (inp->flags < 0)
        {
            LOG(LOG_DUMP, ("gamma prefix code = %d\n", inp->depth));
            output_gamma_code(out, inp->depth);
        } else
        {
            LOG(LOG_DUMP, ("flat prefix %d bits\n", inp->depth));
            output_bits(out, inp->prefix, inp->depth);
        }
    }

    return val;
}

float optimal_encode(const_matchp mp, encode_match_data emd)
{
    interval_nodep *offset;
    float bits;
    encode_match_privp data;

    data = emd->priv;
    offset = data->offset_f_priv;

    bits = 0.0;
    if (mp->offset == 0)
    {
        bits += 9.0f * mp->len;
        data->lit_num += mp->len;
        data->lit_bits += bits;
    } else
    {
        bits += 1.0;
        switch (mp->len)
        {
        case 0:
            LOG(LOG_ERROR, ("bad len\n"));
            exit(1);
            break;
        case 1:
            bits += data->offset_f(mp->offset, offset[0], emd->out);
            break;
        case 2:
            bits += data->offset_f(mp->offset, offset[1], emd->out);
            break;
        default:
            bits += data->offset_f(mp->offset, offset[7], emd->out);
            break;
        }
        bits += data->len_f(mp->len, data->len_f_priv, emd->out);
        if (bits > (9.0 * mp->len))
        {
            /* lets make literals out of it */
            data->lit_num += 1;
            data->lit_bits += bits;
        } else
        {
            if (mp->offset == 1)
            {
                data->rle_num += 1;
                data->rle_bits += bits;
            } else
            {
                data->seq_num += 1;
                data->seq_bits += bits;
            }
        }
    }
    return bits;
}

struct _optimize_arg {
    radix_root cache;
    int *stats;
    int *stats2;
    int max_depth;
    int flags;
    struct chunkpool in_pool[1];
};

#define CACHE_KEY(START, DEPTH, MAXDEPTH) ((int)((START)*(MAXDEPTH)|DEPTH))

typedef struct _optimize_arg optimize_arg[1];
typedef struct _optimize_arg optimize_argp;

static interval_nodep
optimize1(optimize_arg arg, int start, int depth, int init)
{
    interval_node inp;
    interval_nodep best_inp;
    int key;
    int end, i;
    int start_count, end_count;

    LOG(LOG_DUMP, ("IN start %d, depth %d\n", start, depth));

    do
    {
        best_inp = NULL;
        if (arg->stats[start] == 0)
        {
            break;
        }
        key = CACHE_KEY(start, depth, arg->max_depth);
        best_inp = radix_node_get(arg->cache, key);
        if (best_inp != NULL)
        {
            break;
        }

        interval_node_init(inp, start, depth, arg->flags);

        for (i = 0; i < 16; ++i)
        {
            inp->next = NULL;
            inp->bits = i;
            end = start + (1 << i);

            start_count = end_count = 0;
            if (start < 65536)
            {
                start_count = arg->stats[start];
                if (end < 65536)
                {
                    end_count = arg->stats[end];
                }
            }

            inp->score = (start_count - end_count) *
                (inp->prefix + inp->bits);

            /* one index below */
            LOG(LOG_DUMP, ("interval score: [%d«%d[%d\n",
                           start, i, inp->score));
            if (end_count > 0)
            {
                int penalty;
                /* we're not done, now choose between using
                 * more bits, go deeper or skip the rest */
                if (depth + 1 < arg->max_depth)
                {
                    /* we can go deeper, let's try that */
                    inp->next = optimize1(arg, end, depth + 1, i);
                }
                /* get the penalty for skipping */
                penalty = 100000000;
                if (arg->stats2 != NULL)
                {
                    penalty = arg->stats2[end];
                }
                if (inp->next != NULL && inp->next->score < penalty)
                {
                    penalty = inp->next->score;
                }
                inp->score += penalty;
            }
            if (best_inp == NULL || inp->score < best_inp->score)
            {
                /* it's the new best in town, use it */
                if (best_inp == NULL)
                {
                    /* allocate if null */
                    best_inp = chunkpool_malloc(arg->in_pool);
                }
                *best_inp = *inp;
            }
        }
        if (best_inp != NULL)
        {
            radix_node_set(arg->cache, key, best_inp);
        }
    }
    while (0);

    if(IS_LOGGABLE(LOG_DUMP))
    {
        LOG(LOG_DUMP, ("OUT depth %d: ", depth));
        interval_node_dump(LOG_DUMP, best_inp);
    }
    return best_inp;
}

static interval_nodep
exo_optimize(int stats[65536], int stats2[65536], int max_depth, int flags)
{
    optimize_arg arg;

    interval_nodep inp;

    arg->stats = stats;
    arg->stats2 = stats2;

    arg->max_depth = max_depth;
    arg->flags = flags;

    chunkpool_init(arg->in_pool, sizeof(interval_node));

    radix_tree_init(arg->cache);

    inp = optimize1(arg, 1, 0, 0);

    /* use normal malloc for the winner */
    inp = interval_node_clone(inp);

    /* cleanup */
    radix_tree_free(arg->cache, NULL, NULL);
    chunkpool_free(arg->in_pool);

    return inp;
}

static const char *export_helper(interval_nodep np, int depth)
{
    static char buf[20];
    char *p = buf;
    while(np != NULL)
    {
        p += sprintf(p, "%X", np->bits);
        np = np->next;
        --depth;
    }
    while(depth-- > 0)
    {
        p += sprintf(p, "0");
    }
    return buf;
}

const char *optimal_encoding_export(encode_match_data emd)
{
    interval_nodep *offsets;
    static char buf[100];
    char *p = buf;
    encode_match_privp data;
    data = emd->priv;
    offsets = (interval_nodep*)data->offset_f_priv;
    p += sprintf(p, "%s", export_helper((interval_nodep)data->len_f_priv, 16));
    p += sprintf(p, ",%s", export_helper(offsets[0], 4));
    p += sprintf(p, ",%s", export_helper(offsets[1], 16));
    p += sprintf(p, ",%s", export_helper(offsets[7], 16));
    return buf;
}

static void import_helper(interval_nodep *npp,
                          const char **encodingp,
                          int flags)
{
    int c;
    int start = 1;
    int depth = 0;
    const char *encoding;

    encoding = *encodingp;
    while((c = *(encoding++)) != '\0')
    {
        char buf[2] = {0, 0};
        char *dummy;
        int bits;
        interval_nodep np;

        if(c == ',')
        {
            break;
        }

        buf[0] = c;
        bits = strtol(buf, &dummy, 16);

        LOG(LOG_DUMP, ("got bits %d\n", bits));

        np = malloc(sizeof(interval_node));
        interval_node_init(np, start, depth, flags);
        np->bits = bits;

        ++depth;
        start += 1 << bits;

        *npp = np;
        npp = &(np->next);
    }
    *encodingp = encoding;
}

void optimal_encoding_import(encode_match_data emd,
                             const char *encoding)
{
    encode_match_privp data;
    interval_nodep *npp, *offsets;

    LOG(LOG_DEBUG, ("importing encoding: %s\n", encoding));

    optimal_free(emd);
    optimal_init(emd);

    data = emd->priv;
    offsets = (interval_nodep*)data->offset_f_priv;

    /* lengths */
    npp = (void*)&data->len_f_priv;
    import_helper(npp, &encoding, -1);

    /* offsets, len = 1 */
    npp = &offsets[0];
    import_helper(npp, &encoding, 2);

    /* offsets, len = 2 */
    npp = &offsets[1];
    import_helper(npp, &encoding, 4);

    /* offsets, len >= 3 */
    npp = &offsets[7];
    import_helper(npp, &encoding, 4);

    LOG(LOG_DEBUG, ("imported encoding: "));
    optimal_dump(LOG_DEBUG, emd);
}

void optimal_init(encode_match_data emd)        /* IN/OUT */
{
    encode_match_privp data;
    interval_nodep *inpp;

    emd->priv = malloc(sizeof(encode_match_priv));
    data = emd->priv;

    memset(data, 0, sizeof(encode_match_priv));

    data->offset_f = optimal_encode_int;
    data->len_f = optimal_encode_int;
    inpp = malloc(sizeof(interval_nodep[8]));
    inpp[0] = NULL;
    inpp[1] = NULL;
    inpp[2] = NULL;
    inpp[3] = NULL;
    inpp[4] = NULL;
    inpp[5] = NULL;
    inpp[6] = NULL;
    inpp[7] = NULL;
    data->offset_f_priv = inpp;
    data->len_f_priv = NULL;
}

void optimal_free(encode_match_data emd)        /* IN */
{
    encode_match_privp data;
    interval_nodep *inpp;
    interval_nodep inp;

    data = emd->priv;

    inpp = data->offset_f_priv;
    if (inpp != NULL)
    {
        interval_node_delete(inpp[0]);
        interval_node_delete(inpp[1]);
        interval_node_delete(inpp[2]);
        interval_node_delete(inpp[3]);
        interval_node_delete(inpp[4]);
        interval_node_delete(inpp[5]);
        interval_node_delete(inpp[6]);
        interval_node_delete(inpp[7]);
    }
    free(inpp);

    inp = data->len_f_priv;
    interval_node_delete(inp);

    data->offset_f_priv = NULL;
    data->len_f_priv = NULL;
}

void freq_stats_dump(int level, int arr[65536])
{
    int i;
    for (i = 0; i < 32; ++i)
    {
        LOG(level, ("%d, ", arr[i] - arr[i + 1]));
    }
    LOG(level, ("\n"));
}

void freq_stats_dump_raw(int level, int arr[65536])
{
    int i;
    for (i = 0; i < 32; ++i)
    {
        LOG(level, ("%d, ", arr[i]));
    }
    LOG(level, ("\n"));
}

void optimal_optimize(encode_match_data emd,    /* IN/OUT */
                      matchp_enum_get_next_f * f,       /* IN */
                      void *matchp_enum)        /* IN */
{
    encode_match_privp data;
    const_matchp mp;
    interval_nodep *offset;
    static int offset_arr[8][65536];
    static int offset_parr[8][65536];
    static int len_arr[65536];
    int treshold;

    int i, j;
    void *priv1;

    data = emd->priv;

    memset(offset_arr, 0, sizeof(offset_arr));
    memset(offset_parr, 0, sizeof(offset_parr));
    memset(len_arr, 0, sizeof(len_arr));

    offset = data->offset_f_priv;

    /* first the lens */
    priv1 = matchp_enum;
#if 0
    while ((mp = f(priv1)) != NULL)
    {
        LOG(LOG_DEBUG, ("%p len %d offset %d\n", mp, mp->len, mp->offset));
    }
    if(mp->len < 0)
    {
        LOG(LOG_ERROR, ("the horror, negative len!\n"));
    }
#endif
    while ((mp = f(priv1)) != NULL && mp->len > 0)
    {
        if (mp->offset > 0)
        {
            len_arr[mp->len] += 1;
            if(len_arr[mp->len] < 0)
            {
                LOG(LOG_ERROR, ("len counter wrapped!\n"));
            }
        }
    }

    for (i = 65534; i >= 0; --i)
    {
        len_arr[i] += len_arr[i + 1];
        if(len_arr[i] < 0)
        {
            LOG(LOG_ERROR, ("len counter wrapped!\n"));
        }
    }

    data->len_f_priv = exo_optimize(len_arr, NULL, 16, -1);

    /* then the offsets */
    priv1 = matchp_enum;
    while ((mp = f(priv1)) != NULL && mp->len > 0)
    {
        if (mp->offset > 0)
        {
            treshold = mp->len * 9;
            treshold -= 1 + (int) optimal_encode_int(mp->len,
                                                     data->len_f_priv,
                                                     NULL);
            switch (mp->len)
            {
            case 0:
                LOG(LOG_ERROR, ("bad len\n"));
                exit(0);
                break;
            case 1:
                offset_parr[0][mp->offset] += treshold;
                offset_arr[0][mp->offset] += 1;
                if(offset_arr[0][mp->offset] < 0)
                {
                    LOG(LOG_ERROR, ("offset0 counter wrapped!\n"));
                }
                break;
            case 2:
                offset_parr[1][mp->offset] += treshold;
                offset_arr[1][mp->offset] += 1;
                if(offset_arr[1][mp->offset] < 0)
                {
                    LOG(LOG_ERROR, ("offset1 counter wrapped!\n"));
                }
                break;
            default:
                offset_parr[7][mp->offset] += treshold;
                offset_arr[7][mp->offset] += 1;
                if(offset_arr[7][mp->offset] < 0)
                {
                    LOG(LOG_ERROR, ("offset7 counter wrapped!\n"));
                }
                break;
            }
        }
    }

    for (i = 65534; i >= 0; --i)
    {
        for (j = 0; j < 8; ++j)
        {
            offset_arr[j][i] += offset_arr[j][i + 1];
            offset_parr[j][i] += offset_parr[j][i + 1];
        }
    }

    offset[0] = exo_optimize(offset_arr[0], offset_parr[0], 1 << 2, 2);
    offset[1] = exo_optimize(offset_arr[1], offset_parr[1], 1 << 4, 4);
    offset[2] = exo_optimize(offset_arr[2], offset_parr[2], 1 << 4, 4);
    offset[3] = exo_optimize(offset_arr[3], offset_parr[3], 1 << 4, 4);
    offset[4] = exo_optimize(offset_arr[4], offset_parr[4], 1 << 4, 4);
    offset[5] = exo_optimize(offset_arr[5], offset_parr[5], 1 << 4, 4);
    offset[6] = exo_optimize(offset_arr[6], offset_parr[6], 1 << 4, 4);
    offset[7] = exo_optimize(offset_arr[7], offset_parr[7], 1 << 4, 4);

    if(IS_LOGGABLE(LOG_DEBUG))
    {
        optimal_dump(LOG_DEBUG, emd);
    }
}

void optimal_dump(int level, encode_match_data emd)
{
    encode_match_privp data;
    interval_nodep *offset;
    interval_nodep len;

    data = emd->priv;

    offset = data->offset_f_priv;
    len = data->len_f_priv;

    LOG(level, ("lens:             "));
    interval_node_dump(level, len);

    LOG(level, ("offsets (len =1): "));
    interval_node_dump(level, offset[0]);

    LOG(level, ("offsets (len =2): "));
    interval_node_dump(level, offset[1]);

    LOG(level, ("offsets (len =8): "));
    interval_node_dump(level, offset[7]);
}

static
void interval_out(output_ctx out, interval_nodep inp1, int size)
{
    unsigned char buffer[256];
    unsigned char count;
    interval_nodep inp;

    count = 0;

    memset(buffer, 0, sizeof(buffer));
    inp = inp1;
    while (inp != NULL)
    {
        ++count;
        LOG(LOG_DUMP, ("bits %d, lo %d, hi %d\n",
                       inp->bits, inp->start & 0xFF, inp->start >> 8));
        buffer[sizeof(buffer) - count] = inp->bits;
        inp = inp->next;
    }

    while (size > 0)
    {
        int b;
        b = buffer[sizeof(buffer) - size];
        LOG(LOG_DUMP, ("outputting nibble %d\n", b));
        output_bits(out, 4, b);
        size--;
    }
}

void optimal_out(output_ctx out,        /* IN/OUT */
                 encode_match_data emd) /* IN */
{
    encode_match_privp data;
    interval_nodep *offset;
    interval_nodep len;

    data = emd->priv;

    offset = data->offset_f_priv;
    len = data->len_f_priv;

    interval_out(out, offset[0], 4);
    interval_out(out, offset[1], 16);
    interval_out(out, offset[7], 16);
    interval_out(out, len, 16);
}

void output_ctx_init(output_ctx ctx, struct membuf *out)    /* IN/OUT */
{
    ctx->bitbuf = 1;
    ctx->pos = membuf_memlen(out);
    ctx->buf = out;
}

unsigned int output_get_pos(output_ctx ctx)     /* IN */
{
    return ctx->pos;
}

void output_byte(output_ctx ctx,        /* IN/OUT */
                 unsigned char byte)    /* IN */
{
    /*LOG(LOG_DUMP, ("output_byte: $%02X\n", byte)); */
    if(ctx->pos < membuf_memlen(ctx->buf))
    {
        char *p;
        p = membuf_get(ctx->buf);
        p[ctx->pos] = byte;
    }
    else
    {
        while(ctx->pos > membuf_memlen(ctx->buf))
        {
            membuf_append_char(ctx->buf, '\0');
        }
        membuf_append_char(ctx->buf, byte);
    }
    ++(ctx->pos);
}

void output_word(output_ctx ctx,        /* IN/OUT */
                 unsigned short int word)       /* IN */
{
    output_byte(ctx, (unsigned char) (word & 0xff));
    output_byte(ctx, (unsigned char) (word >> 8));
}


void output_bits_flush(output_ctx ctx)  /* IN/OUT */
{
    /* flush the bitbuf including
     * the extra 1 bit acting as eob flag */
    output_byte(ctx, (unsigned char) (ctx->bitbuf & 0xFF));
    if (ctx->bitbuf & 0x100)
    {
        output_byte(ctx, 1);
    }
    LOG(LOG_DUMP, ("bitstream flushed 0x%02X\n", ctx->bitbuf & 0xFF));

    /* reset it */
    ctx->bitbuf = 1;
}

void bits_dump(int count, int val)
{
    static char buf[1024];
    char *pek;
    pek = buf;
    if (count > 0)
    {
        pek += sprintf(pek, "0x%04X, % 2d: ", val, count);
    }
    while (count-- > 0)
    {
        *(pek++) = val & (1 << count) ? '1' : '0';
    }
    *(pek++) = '\0';
    LOG(LOG_NORMAL, ("%s\n", buf));
}

static void output_bits_int(output_ctx ctx,        /* IN/OUT */
                            int count,     /* IN */
                            int val)       /* IN */
{
    /* this makes the bits appear in reversed
     * big endian order in the output stream */
    while (count-- > 0)
    {
        ctx->bitbuf <<= 1;
        ctx->bitbuf |= val & 0x1;
        val >>= 1;
        if (ctx->bitbuf & 0x100)
        {
            /* full byte, flush it */
            output_byte(ctx, (unsigned char) (ctx->bitbuf & 0xFF));
            LOG(LOG_DUMP,
               ("bitstream byte 0x%02X\n", ctx->bitbuf & 0xFF));
            ctx->bitbuf = 1;
        }
    }
}

void output_bits(output_ctx ctx,        /* IN/OUT */
                 int count,     /* IN */
                 int val)       /* IN */
{
    LOG(LOG_DUMP, ("output bits: count = %d, val = %d\n", count, val));
    output_bits_int(ctx, count, val);
}

void output_gamma_code(output_ctx ctx,  /* IN/OUT */
                       int code)        /* IN */
{
    LOG(LOG_DUMP, ("output gamma: code = %d\n", code));
    output_bits_int(ctx, 1, 1);
    while (code-- > 0)
    {
        output_bits_int(ctx, 1, 0);
    }
}

static
void print_usage(const char *appl, enum log_level level,
                 const char *default_out_name)
{
    LOG(level, ("usage: %s [option]... infile\n", appl));
    LOG(level,
        ("  -b            crunch/decrunch backwards\n"
         "  -r            write outfile in reverse order\n"
         "  -d            decrunch (instead of crunch)\n"));
    print_crunch_flags(level, default_out_name);
}

#define DEFAULT_OUTFILE "a.out"

unsigned char *Exomizer_crunch(unsigned char *input_data, int input_len, int *retlen)
{
	int argc=1;
	char **argv=NULL;
    char flags_arr[32];
    int backwards_mode = 0;
    int reverse_mode = 0;
    int c, infilec;
    char **infilev;
	/* output buffer */
	unsigned char *output_data;

    static struct crunch_options options[1] = { CRUNCH_OPTIONS_DEFAULT };
    struct common_flags flags[1] = { {options, DEFAULT_OUTFILE} };

    struct membuf inbuf[1];
    struct membuf outbuf[1];

    const char *appl;;


	argv=malloc(sizeof(char *));
	argv[0]=strdup("mem_exomizer");
    /* init args */
    appl = fixup_appl(argv[0]);

    sprintf(flags_arr, "bdr%s", CRUNCH_FLAGS);
    while ((c = getflag(argc, argv, flags_arr)) != -1)
    {
        switch (c)
        {
        case 'b':
            backwards_mode = 1;
            break;
        case 'r':
            reverse_mode = 1;
            break;
        default:
            handle_crunch_flags(c, flagarg, print_usage, appl, flags);
        }
    }

    infilev = argv + flagind;
    infilec = argc - flagind;


printf("crunching with exomizer (the art of patience...)\n");

/* only memory */

    membuf_init(inbuf);
    membuf_init(outbuf);

	/* rustine */
	membuf_append(inbuf, input_data, input_len);
	
	
    {
        struct crunch_info info[1];
        if(backwards_mode)
        {
            crunch_backwards(inbuf, outbuf, options, info);
        }
        else
        {
            exocrunch(inbuf, outbuf, options, info);
        }
    }

    if(reverse_mode)
    {
        reverse_buffer(membuf_get(outbuf), membuf_memlen(outbuf));
    }
	
	output_data=MemMalloc(membuf_memlen(outbuf));
	memcpy(output_data,membuf_get(outbuf),membuf_memlen(outbuf));
	*retlen=membuf_memlen(outbuf);
	
    membuf_free(outbuf);
    membuf_free(inbuf);

    return output_data;
}
