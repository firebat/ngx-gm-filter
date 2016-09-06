/* vim:set ft=c ts=4 sw=4 et fdm=marker: */

#include "ngx_http_gm_module.h"
#include "ngx_http_gm_common.h"
#include "ngx_http_gm_auto_orient.h"
#include "ngx_http_gm_blur.h"
#include "ngx_http_gm_composite.h"
#include "ngx_http_gm_crop.h"
#include "ngx_http_gm_define.h"
#include "ngx_http_gm_filter.h"
#include "ngx_http_gm_format.h"
#include "ngx_http_gm_gravity.h"
#include "ngx_http_gm_quality.h"
#include "ngx_http_gm_resize.h"
#include "ngx_http_gm_rotate.h"
#include "ngx_http_gm_strip.h"
#include "ngx_http_gm_unsharp.h"

#define GM_IMAGE_HEADER_LENGTH 16

static ngx_int_t ngx_http_gm_image_read(ngx_http_request_t *r, ngx_chain_t *in);
static ngx_int_t ngx_http_gm_image_send(ngx_http_request_t *r, ngx_http_gm_ctx_t *ctx, ngx_chain_t *in);
static ngx_buf_t * ngx_http_gm_image_process(ngx_http_request_t *r);
static void ngx_http_gm_image_cleanup(void *data);

static ngx_uint_t ngx_http_gm_filter_value(ngx_str_t *value);

static void *ngx_http_gm_create_conf(ngx_conf_t *cf);
static char *ngx_http_gm_merge_conf(ngx_conf_t *cf, void *parent, void *child);

static char *ngx_http_gm_gm(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_gm_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_gm_init_worker(ngx_cycle_t *cycle);
static void ngx_http_gm_exit_worker(ngx_cycle_t *cycle);

static ngx_int_t gm_parse_option(ngx_conf_t *cf, ngx_array_t *args, void **option)
{
    return gm_parse_nth_option(cf, args, 1, option);
}

static ngx_http_gm_command_t ngx_gm_commands[] = {

    { ngx_string("auto-orient"),
      gm_auto_orient_image,
      NULL,
      NULL },

    { ngx_string("blur"),
      gm_blur_image,
      gm_parse_option, 
      NULL },

    { ngx_string("composite"),
      gm_composite_image,
      gm_parse_composite_option, 
      NULL },

    { ngx_string("crop"),
      gm_crop_image,
      gm_parse_option, 
      NULL },

    { ngx_string("define"),
      gm_define_image,
      gm_parse_option,
      NULL },

    { ngx_string("filter"),
      gm_filter_image,
      gm_parse_option,
      NULL },

    { ngx_string("format"),
      gm_format_image,
      gm_parse_option,
      NULL },

    { ngx_string("gravity"),
      gm_gravity_image,
      gm_parse_option, 
      NULL },

    { ngx_string("quality"),
      gm_quality_image,
      gm_parse_option,
      NULL },

    { ngx_string("resize"),
      gm_resize_image,
      gm_parse_option,
      NULL },

    { ngx_string("rotate"),
      gm_rotate_image,
      gm_parse_option, 
      NULL },

    { ngx_string("strip"),
      gm_strip_image,
      NULL,
      NULL },

    { ngx_string("unsharp"),
      gm_unsharp_image,
      gm_parse_option,
      NULL },

    /* ngx_gm_null_command */
    { ngx_null_string,
      NULL,
      NULL,
      NULL}
};

static ngx_command_t  ngx_http_gm_commands[] = {

    { ngx_string("gm_buffer"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_gm_conf_t, buffer_size),
      NULL },

    { ngx_string("gm_enable"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_gm_conf_t, enable),
      NULL},

    { ngx_string("gm"),
      NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_gm_gm,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
};


static ngx_http_module_t  ngx_http_gm_module_ctx = {
    NULL,                        /* preconfiguration */
    ngx_http_gm_init,            /* postconfiguration */

    NULL,                        /* create main configuration */
    NULL,                        /* init main configuration */

    NULL,                        /* create server configuration */
    NULL,                        /* merge server configuration */

    ngx_http_gm_create_conf,     /* create location configuration */
    ngx_http_gm_merge_conf       /* merge location configuration */
};


ngx_module_t  ngx_http_gm_module = {
    NGX_MODULE_V1,
    &ngx_http_gm_module_ctx,        /* module context */
    ngx_http_gm_commands,           /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                           /* init master */
    NULL,                           /* init module */
    ngx_http_gm_init_worker,        /* init process */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    ngx_http_gm_exit_worker,        /* exit process */
    NULL,                           /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


static ngx_int_t
ngx_http_gm_header_filter(ngx_http_request_t *r)
{
    off_t                          len;
    ngx_http_gm_ctx_t             *ctx;
    ngx_http_gm_conf_t            *conf;

    ctx = ngx_http_get_module_ctx(r, ngx_http_gm_module);

    if (ctx) {
        dd("gm filter: image filter bypass because of ctx exist, %.*s", (int) r->uri.len, r->uri.data);
        ngx_http_set_ctx(r, NULL, ngx_http_gm_module);
        return ngx_http_next_header_filter(r);
    }

    conf = ngx_http_get_module_loc_conf(r, ngx_http_gm_module);

    if (!conf->enable || !conf->cmds || conf->cmds->nelts == 0) {
        return ngx_http_next_header_filter(r);
    }

    if (r->headers_out.status != NGX_HTTP_OK) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "gm filter: image filter bypassed because of unmatched status "
                       "code %i (only 200 are accepted by "
                       "default)", r->headers_out.status);

        return ngx_http_next_header_filter(r);
    }


    if (r->headers_out.content_type.len
            >= sizeof("multipart/x-mixed-replace") - 1
        && ngx_strncasecmp(r->headers_out.content_type.data,
                           (u_char *) "multipart/x-mixed-replace",
                           sizeof("multipart/x-mixed-replace") - 1)
           == 0)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "gm filter: multipart/x-mixed-replace response");

        return NGX_ERROR;
    }

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_gm_ctx_t));
    dd("gm filter: image filter create, %.*s", (int) r->uri.len, r->uri.data);
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    ngx_http_set_ctx(r, ctx, ngx_http_gm_module);

    len = r->headers_out.content_length_n;

    if (len != -1 && len > (off_t) conf->buffer_size) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "gm filter: too big response: %O", len);

        return NGX_HTTP_UNSUPPORTED_MEDIA_TYPE;
    }

    if (len == -1) {
        ctx->length = conf->buffer_size;
    } else {
        ctx->length = (size_t) len;
    }

    if (r->headers_out.refresh) {
        r->headers_out.refresh->hash = 0;
    }

    r->main_filter_need_in_memory = 1;
    r->allow_ranges = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_gm_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_int_t                      rc;
    ngx_str_t                     *ct;
    ngx_chain_t                    out;
    ngx_http_gm_ctx_t             *ctx;

    char                          type[5];

    if (in == NULL) {
        return ngx_http_next_body_filter(r, in);
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_gm_module);
    if (ctx == NULL) {
        return ngx_http_next_body_filter(r, in);
    }

    switch (ctx->phase) {

    case NGX_HTTP_GM_START:

        if (in->buf->last - in->buf->pos >= GM_IMAGE_HEADER_LENGTH) {
            GetMagickFileFormat(in->buf->pos, GM_IMAGE_HEADER_LENGTH, type, sizeof(type), NULL);
        }

        /* type = '\0' */
        if (type[0] == 0) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: image test return none");
            return ngx_http_filter_finalize_request(r, &ngx_http_gm_module,
                       NGX_HTTP_UNSUPPORTED_MEDIA_TYPE);
        }

        ctx->phase = NGX_HTTP_GM_IMAGE_READ;

        /* fall through */

    case NGX_HTTP_GM_IMAGE_READ:

        rc = ngx_http_gm_image_read(r, in);
        if (rc == NGX_AGAIN) {
            return NGX_OK;
        }

        if (rc == NGX_ERROR) {
            return ngx_http_filter_finalize_request(r,
                                              &ngx_http_gm_module,
                                              NGX_HTTP_UNSUPPORTED_MEDIA_TYPE);
        }

        /* fall through */

    case NGX_HTTP_GM_IMAGE_PROCESS:

        out.buf = ngx_http_gm_image_process(r);
        if (out.buf == NULL) {
            return ngx_http_filter_finalize_request(r,
                                              &ngx_http_gm_module,
                                              NGX_HTTP_UNSUPPORTED_MEDIA_TYPE);
        }

        out.next = NULL;
        ctx->phase = NGX_HTTP_GM_IMAGE_PASS;

        return ngx_http_gm_image_send(r, ctx, &out);

    case NGX_HTTP_GM_IMAGE_PASS:

        return ngx_http_next_body_filter(r, in);

    default: /* NGX_HTTP_GM_IMAGE_DONE */

        rc = ngx_http_next_body_filter(r, NULL);

        /* NGX_ERROR resets any pending data */
        return (rc == NGX_OK) ? NGX_ERROR : rc;
    }
}


static ngx_int_t
ngx_http_gm_image_send(ngx_http_request_t *r, ngx_http_gm_ctx_t *ctx,
    ngx_chain_t *in)
{
    ngx_int_t  rc;

    rc = ngx_http_next_header_filter(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return NGX_ERROR;
    }

    rc = ngx_http_next_body_filter(r, in);

    if (ctx->phase == NGX_HTTP_GM_IMAGE_DONE) {
        /* NGX_ERROR resets any pending data */
        return (rc == NGX_OK) ? NGX_ERROR : rc;
    }

    return rc;
}


static ngx_int_t
ngx_http_gm_image_read(ngx_http_request_t *r, ngx_chain_t *in)
{
    u_char                       *p;
    size_t                        size, rest;
    ngx_buf_t                    *b;
    ngx_chain_t                  *cl;
    ngx_http_gm_ctx_t  *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_gm_module);

    if (ctx->image_blob == NULL) {
        ctx->image_blob = ngx_palloc(r->pool, ctx->length);
        if (ctx->image_blob == NULL) {
            return NGX_ERROR;
        }

        ctx->last = ctx->image_blob;
    }

    p = ctx->last;

    for (cl = in; cl; cl = cl->next) {

        b = cl->buf;
        size = b->last - b->pos;

        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "gm filter: image buf size %uz", size);

        rest = ctx->image_blob + ctx->length - p;
        size = (rest < size) ? rest : size;

        p = ngx_cpymem(p, b->pos, size);
        b->pos += size;

        if (b->last_buf) {
            ctx->last = p;
            return NGX_OK;
        }
    }

    ctx->last = p;
    r->connection->buffered |= NGX_HTTP_IMAGE_BUFFERED;

    return NGX_AGAIN;
}


static ngx_buf_t *
ngx_http_gm_image_process(ngx_http_request_t *r)
{
    ngx_http_gm_ctx_t   *ctx;
    ngx_buf_t           *b;
    ngx_int_t            rc;
    ngx_http_gm_conf_t  *gmcf;

    u_char         *image_blob;
    size_t          image_size;

    ImageInfo      *image_info;
    Image          *image;
    ExceptionInfo   exception;

    ngx_uint_t      i;
    ngx_http_gm_command_t *gm_cmd;
    ngx_http_gm_command_info_t *cmd_info;
    u_char         *out_blob;
    ngx_uint_t      out_len;

    ngx_pool_cleanup_t            *cln;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: entering gm image run commands");
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: url %V", &r->uri);

    r->connection->buffered &= ~NGX_HTTP_IMAGE_BUFFERED;
    ctx = ngx_http_get_module_ctx(r, ngx_http_gm_module);

    GetExceptionInfo(&exception);
    image_info = CloneImageInfo((ImageInfo *) NULL);

    image_blob = ctx->image_blob;
    image_size = ctx->last - image_blob;

    /* blob to image */
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: blob to image");
    image = BlobToImage(image_info, image_blob, image_size, &exception);
    if (image == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "gm filter: blob to image failed, severity: %O reason: %s, description: %s",
                      exception.severity, exception.reason, exception.description);
        goto failed1;
    }


    /* run commands */
    rc = NGX_OK;
    gmcf = ngx_http_get_module_loc_conf(r, ngx_http_gm_module);

    if (gmcf->cmds != NULL) {

        for (i = 0, cmd_info = gmcf->cmds->elts; i < gmcf->cmds->nelts; ++i, cmd_info++) {

            gm_cmd = cmd_info->command;
            ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: run command: \"%V\"", &gm_cmd->name);

            if (gm_cmd->handler != NULL) {

                rc = gm_cmd->handler(r, cmd_info->option, &image, image_info);
                if (rc != NGX_OK) {
                    goto failed2;
                }
            }

            if (gm_cmd->out_handler != NULL) {
                b = gm_cmd->out_handler(r, image);
                goto out;
            }
        }
    }

    /* image to blob */
    out_blob = ImageToBlob(image_info, image,  &out_len, &exception);
    if (out_blob == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "gm filter: image to blob failed, severity: %O reason: %s, description: %s",
                      exception.severity, exception.reason, exception.description);
        goto failed2;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: blob to buf");
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: alloc buf_t failed");
        goto failed3;
    }

    b->pos = out_blob;
    b->last = out_blob + out_len;
    b->memory = 1;
    b->last_buf = 1;

    /* register cleanup */
    cln = ngx_pool_cleanup_add(r->pool, 0);
    if (cln == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: register cleanup failed");
        goto failed3;
    }

    cln->handler = ngx_http_gm_image_cleanup;
    cln->data = out_blob;

out:
    /* content length */
    r->headers_out.content_length_n = b->last - b->pos;
    if (r->headers_out.content_length) {
        r->headers_out.content_length->hash = 0;
    }
    r->headers_out.content_length = NULL;

    /* destory input blob */
    ngx_pfree(r->pool, ctx->image_blob);

    DestroyImage(image);
    DestroyImageInfo(image_info);
    DestroyExceptionInfo(&exception);

    return b;

failed3:
    /* clean out blob */
    MagickFree(out_blob);

failed2:
    DestroyImage(image);

failed1:
    DestroyImageInfo(image_info);
    DestroyExceptionInfo(&exception);

    return NULL;
}

static void
ngx_http_gm_image_cleanup(void *out_blob)
{
    dd("cleanup iamge out_blob");
    MagickFree(out_blob);
}


static void *
ngx_http_gm_create_conf(ngx_conf_t *cf)
{
    ngx_http_gm_conf_t  *gmcf;

    gmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_gm_conf_t));
    if (gmcf == NULL) {
        return NULL;
    }

    gmcf->buffer_size = NGX_CONF_UNSET_SIZE;
    gmcf->enable = NGX_CONF_UNSET;

    return gmcf;
}


static char *
ngx_http_gm_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_gm_conf_t *prev = parent;
    ngx_http_gm_conf_t *conf = child;

    if (conf->cmds == NULL && prev->cmds != NULL) {
        conf->cmds = prev->cmds;
    }

    ngx_conf_merge_size_value(conf->buffer_size, prev->buffer_size, 4 * 1024 * 1024);
    ngx_conf_merge_value(conf->enable, prev->enable, 1);
    
    return NGX_CONF_OK;
}

static char *
ngx_http_gm_gm(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_gm_conf_t                *gmcf = conf;

    ngx_str_t                         *name;
    ngx_http_gm_command_t             *gmcmd;
    ngx_http_gm_command_info_t        *info;

    dd("entering");

    if (cf->args->nelts < 2) {
        return NGX_CONF_ERROR;
    }

    /* gm $name $arg ... */
    name = cf->args->elts;
    name += 1;

    if (gmcf->cmds == NULL) {
        gmcf->cmds = ngx_array_create(cf->pool, 4, sizeof(ngx_http_gm_command_info_t));
        if (gmcf->cmds == NULL) {
            goto failed;
        }
    }

    for(gmcmd=ngx_gm_commands; gmcmd->name.len; gmcmd++) {

        if (ngx_strcmp(name->data, gmcmd->name.data) != 0) {
            continue;
        }

        info = ngx_array_push(gmcf->cmds);
        if (info == NULL) {
            goto alloc_failed;
        }
        info->command = gmcmd;

        if (gmcmd->option_parse_handler) {
            if (gmcmd->option_parse_handler(cf, cf->args, &info->option) != NGX_OK) {
                goto failed;
            }
        }

        dd("parse config okay");
        return NGX_CONF_OK;
    }

failed:
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "gm filter: invalid parameter for command, \"%V\"", name);
    return NGX_CONF_ERROR;

alloc_failed:
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "gm filter: alloc failed \"%V\"", name);
    return NGX_CONF_ERROR;
}


static ngx_int_t
ngx_http_gm_init_worker(ngx_cycle_t *cycle)
{
    InitializeMagick("logs");

    return NGX_OK;
}


static void
ngx_http_gm_exit_worker(ngx_cycle_t *cycle)
{
    DestroyMagick();
}


static ngx_int_t
ngx_http_gm_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_gm_header_filter;

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_gm_body_filter;

    return NGX_OK;
}

