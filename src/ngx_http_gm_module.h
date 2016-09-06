#ifndef NGX_HTTP_GM_FILTER_MODULE_H
#define NGX_HTTP_GM_FILTER_MODULE_H

#include "ddebug.h"

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <magick/api.h>

#define NGX_HTTP_GM_START           0
#define NGX_HTTP_GM_IMAGE_READ      1
#define NGX_HTTP_GM_IMAGE_PROCESS   2
#define NGX_HTTP_GM_IMAGE_PASS      3
#define NGX_HTTP_GM_IMAGE_DONE      4

#define NGX_HTTP_IMAGE_BUFFERED     0x08

typedef ngx_int_t (*ngx_http_gm_command_pt)(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info);
typedef ngx_int_t (*ngx_http_gm_parse_pt)(ngx_conf_t *cf, ngx_array_t *args, void **option);
typedef ngx_buf_t *(*ngx_http_gm_out_pt)(ngx_http_request_t *r, Image *image);

typedef struct {
    ngx_str_t                   name;
    ngx_http_gm_command_pt      handler;
    ngx_http_gm_parse_pt        option_parse_handler;
    ngx_http_gm_out_pt          out_handler;
} ngx_http_gm_command_t;

typedef struct {
    ngx_http_gm_command_t       *command;
    void                        *option;
} ngx_http_gm_command_info_t;

typedef struct {
    ngx_array_t                 *cmds;          /* ngx_http_gm_command_info_t */
    size_t                       buffer_size;
    ngx_flag_t                   enable;
} ngx_http_gm_conf_t;

typedef struct {
    u_char                      *image_blob;    /* image blob for read */
    u_char                      *last;
    size_t                       length;        /* image buffer alloc size */
    ngx_uint_t                   phase;
} ngx_http_gm_ctx_t;

extern ngx_module_t ngx_http_gm_module;

#endif /* NGX_HTTP_GM_FILTER_MODULE_H */
