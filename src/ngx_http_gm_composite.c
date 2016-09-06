#include "ngx_http_gm_composite.h"

#define COMPOSITE_IMAGE_SIZE 32

typedef struct {
    ngx_str_t  name;
    Image     *image;
} composite_image_t;

typedef struct {
    ngx_http_gm_option_t *compose;
    ngx_http_gm_option_t *geometry;
    ngx_http_gm_option_t *image_file;

    ngx_uint_t min_width;
    ngx_uint_t min_height;

    ngx_array_t *images;

    ngx_pool_t *pool;
} ngx_http_gm_composite_option_t;

static Image * find_composite_image(ngx_http_gm_composite_option_t *opt, u_char *name);
static Image * load_composite_image(ngx_http_request_t *r, ngx_http_gm_composite_option_t *opt, u_char *name);
static ngx_uint_t cache_composite_image(ngx_http_request_t *r, ngx_http_gm_composite_option_t *opt, u_char *name, Image *image);

ngx_int_t
gm_parse_composite_option(ngx_conf_t *cf, ngx_array_t *args, void **option)
{
    ngx_http_gm_composite_option_t   *opt;
    ngx_str_t                        *value;
    ngx_uint_t                        i;
    ngx_uint_t                        rc;

    dd("entering");

    opt = ngx_palloc(cf->pool, sizeof(ngx_http_gm_composite_option_t));
    if (opt == NULL) {
        return NGX_ERROR;
    }
    ngx_memzero(opt, sizeof(ngx_http_gm_composite_option_t));

    opt->pool = cf->pool;
    opt->images = ngx_array_create(cf->pool, COMPOSITE_IMAGE_SIZE, sizeof(composite_image_t));
    if (opt->images == NULL) {
        return NGX_ERROR;
    }

    value = args->elts;
    rc = NGX_OK;

    for (i=2; i < args->nelts; i+=2) {

        if (ngx_strcmp(value[i].data, "-compose") == 0) {
            rc = gm_parse_nth_option(cf, args, i, (void **) &opt->compose);
        } else if(ngx_strcmp(value[i].data, "-geometry") == 0) {
            rc = gm_parse_nth_option(cf, args, i, (void **) &opt->geometry);
        } else if (ngx_strcmp(value[i].data, "-image") == 0) {
            rc = gm_parse_nth_option(cf, args, i, (void **) &(opt->image_file));
        } else if (ngx_strcmp(value[i].data, "-min-width") == 0) {
            opt->min_width = ngx_atoi(value[i+1].data, value[i+1].len);
        } else if (ngx_strcmp(value[i].data, "-min-height") == 0) {
            opt->min_height = ngx_atoi(value[i+1].data, value[i+1].len);
        } else {
            return NGX_ERROR;
        }

        if (rc != NGX_OK) {
            return rc;
        }
    }

    *option = opt;

    return NGX_OK;
}

ngx_int_t
gm_composite_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    ngx_http_gm_composite_option_t *opt = (ngx_http_gm_composite_option_t *) option;
    CompositeOperator              compose;
    RectangleInfo                  geometry;
    char                           composite_geometry[MaxTextExtent];
    Image                         *composite_image;

    u_char                        *value;

    MagickPassFail                 rc;

    if ((*image)->columns < opt->min_width || (*image)->rows < opt->min_height) {
        return NGX_OK;
    }

    // -image
    value = gm_get_option_value(r, opt->image_file);
    if (value == NULL || ngx_strlen(value) == 0) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: composite require 'image'");
        return NGX_ERROR;
    }

    composite_image = find_composite_image(opt, value);
    if (composite_image == NULL) {
        composite_image = load_composite_image(r, opt, value);
    }
    if (composite_image == NULL) {
        return NGX_ERROR;
    }

    // -compose
    value = gm_get_option_value(r, opt->compose);
    if (value == NULL || ngx_strlen(value) == 0) {
        compose = OverCompositeOp;
    } else {
        compose = StringToCompositeOperator((char *)value);
    }

    // -geometry
    value = gm_get_option_value(r, opt->geometry);
    if (value == NULL || ngx_strlen(value) == 0 || !IsGeometry((const char*)value)) {
        value = (u_char *) "+0+0";
    }

    GetGeometry((char *) value, &geometry.x, &geometry.y, &geometry.width, &geometry.height);
    FormatString(composite_geometry, "%lux%lu%+ld%+ld", composite_image->columns, composite_image->rows, geometry.x, geometry.y);
    GetImageGeometry(*image, composite_geometry, 0, &geometry);

    rc = CompositeImage(*image, compose, composite_image, geometry.x, geometry.y);
    if (rc != MagickPass) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


static Image *
find_composite_image(ngx_http_gm_composite_option_t *opt, u_char *name)
{
    composite_image_t *elt;
    ngx_uint_t         i;

    for (i=0, elt = (composite_image_t *) opt->images->elts; i<opt->images->nelts; i++, elt++) {
        if (ngx_strncmp(elt->name.data, name, elt->name.len) == 0) {
            return elt->image;
        }
    }

    return NULL;
}


static ngx_uint_t
cache_composite_image(ngx_http_request_t *r, ngx_http_gm_composite_option_t *opt, u_char *name, Image *image)
{
    composite_image_t * elt = ngx_array_push(opt->images);
    if (elt == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: create composite image failed");
        return NGX_ERROR;
    }

    size_t len = ngx_strlen(name);
    u_char * buf = ngx_palloc(opt->pool, len);
    ngx_memcpy(buf, name, len);

    elt->name.len = len;
    elt->name.data = buf;
    elt->image = image;

    return NGX_OK;
}

static Image *
load_composite_image(ngx_http_request_t *r, ngx_http_gm_composite_option_t *opt, u_char *name)
{
    ExceptionInfo                  exception;
    ImageInfo                     *image_info;
    Image                         *image;
    ngx_uint_t                     rc;

    image_info = CloneImageInfo((ImageInfo *) NULL);
    ngx_memcpy(image_info->filename, name, ngx_strlen(name));

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "load composite image: \"%s\"", image_info->filename);

    GetExceptionInfo(&exception);

    image = ReadImage(image_info, &exception);
    if (image == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "gm filter: load composite image failed, severity: %O reason: %s, description: %s",
                      exception.severity, exception.reason, exception.description);
        goto failed;
    } 

    rc = cache_composite_image(r, opt, name, image);
    if (rc != NGX_OK) {
        // full ?
        // DestroyImage(image);
        // image = NULL;
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "gm filter: cache composite image failed, cached: %lu", opt->images->nelts);
    }

 failed:
    DestroyExceptionInfo(&exception);
    DestroyImageInfo(image_info);

    return image;
}

