#include "textures.h"

#include "allocorexit.h"
#include <png.h>

GLuint load_png_texture_from_path(const char *path) {
    png_image image = {.version = PNG_IMAGE_VERSION};

    /* The first argument is the file to read: */
    if (png_image_begin_read_from_file(&image, path) != 0) {
        image.format = PNG_FORMAT_RGBA;

        png_bytep buffer =  malloc_or_exit(PNG_IMAGE_SIZE(image));

        if (buffer != NULL && png_image_finish_read(&image, NULL, buffer, 0, NULL) != 0) {
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            return texture;
        } else {
            /* Calling png_image_free is optional unless the simplified API was
             * not run to completion.  In this case, if there wasn't enough
             * memory for 'buffer', we didn't complete the read, so we must
             * free the image:
             */
            if (buffer == NULL) // Just in case if malloc_or_exit is changed back to malloc
                png_image_free(&image);
            else
                free(buffer);
        }
    }
    return 0;
}
