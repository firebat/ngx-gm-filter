 #include "ngx_http_gm_identify.h"

ngx_buf_t *
gm_identify_image(ngx_http_request_t *r, Image *image)
{
    ngx_buf_t              *b;

    const MagickInfo       *magick_info;
    const ImageAttribute   *attribute;
    const u_char           *profile;
    const size_t            profile_length;
    register size_t         i;

    ngx_http_clean_header(r);
    r->headers_out.status = NGX_HTTP_OK;
    ngx_str_set(&r->headers_out.content_type, "application/json");
    r->headers_out.content_type_lowcase = NULL;

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NULL;
    }

    b->memory = 1;
    b->last_buf = 1;

    // FIXME, len
    b->pos = ngx_palloc(r->pool, 512);
    if (b->pos == NULL) {
        return NULL;
    }

    magick_info = GetMagickInfo(image->magick, &image->exception);
    if (magick_info == (const MagickInfo *) NULL || IsEmpty(magick_info->description)) {
        b->last = ngx_sprintf(b->pos, "{\"Format\":\"%s\"", image->magick);
    } else {
        b->last = ngx_sprintf(b->pos, "{\"Format\":\"%s (%s)\"", image->magick, magick_info->description);
    }
    b->last = ngx_sprintf(b->last, ", \"Geometry\": \"%uzx%uz\"", image->columns, image->rows);
    b->last = ngx_sprintf(b->last, ", \"Width\": %uz", image->columns);
    b->last = ngx_sprintf(b->last, ", \"Height\": %uz", image->rows);
    b->last = ngx_sprintf(b->last, ", \"Class\": \"%s\"", image->storage_class == DirectClass ? "DirectClass" : "PseudoClass");
    b->last = ngx_sprintf(b->last, ", \"Type\": \"%s\"", ImageTypeToString(GetImageType(image, &image->exception)));
    b->last = ngx_sprintf(b->last, ", \"Depth\": %uz", GetImageDepth(image, &image->exception));
    if (image->x_resolution != 0.0 && image->y_resolution != 0.0) {
        b->last = ngx_sprintf(b->last, ", \"Resolution\": \"%fx%f pixels", image->x_resolution, image->y_resolution);
        if (image->units == PixelsPerInchResolution) {
            b->last = ngx_sprintf(b->last, "/inch");
        } else if (image->units == PixelsPerCentimeterResolution) {
            b->last = ngx_sprintf(b->last, "/centimeter");
        }
        b->last = ngx_sprintf(b->last, "\"");
    }
    b->last = ngx_sprintf(b->last, ", \"Filesize\": %uz", GetBlobSize(image));
    b->last = ngx_sprintf(b->last, ", \"Interlace\": \"%s\"", InterlaceTypeToString(image->interlace == UndefinedInterlace ? NoInterlace : image->interlace));
    b->last = ngx_sprintf(b->last, ", \"Orientation\": \"%s\"", OrientationTypeToString(image->orientation));
    b->last = ngx_sprintf(b->last, ", \"Compression\": \"%s\"", CompressionTypeToString(image->compression));

    attribute = GetImageAttribute(image, (char *) NULL);
    for (; attribute != NULL; attribute=attribute->next) {
        if (ngx_strncmp("EXIF", attribute->key, 4) != 0) {
            b->last = ngx_sprintf(b->last, ", \"%s\": \"%s\"", attribute->key, attribute->value);
        }
    }
    b->last = ngx_sprintf(b->last, "}" CRLF);

    return b;
}
