/*
 * Temporary PNG loading routine that support the alpha channel
 * while Imlib either gets fixed or we replace it with the
 * new libart_lgpl/GdkPixBuf image system.
 *
 * Most of the code comes from GdkImlib, written by the Rasterman
 * (raster@redhat.com).
 *
 * After Imlib 1.9 is wildely used, we can remove this.
 *
 * Miguel de Icaza.
 */
#include <config.h>
#include <gdk_imlib.h>
#include <malloc.h>
#include "gtk-canvas.h"
#include "gtk-canvas-load.h"

#ifdef HAVE_IMLIB_1_9
GdkImlibImage *
gtk_canvas_load_alpha (const gchar *file)
{
	g_return_val_if_fail (file != NULL, NULL);

	return gdk_imlib_load_alpha (file);
}

void
gtk_canvas_destroy_image (GdkImlibImage *image)
{
	g_return_if_fail (image != NULL);
	gdk_imlib_destroy_image (image);
}
#else
#ifdef HAVE_LIBPNG
#include <png.h>
#include <setjmp.h>

static unsigned char *
_gtk_canvas_load_alpha (FILE * f, int *w, int *h, int *t, unsigned char **alpha)
{
	png_structp         png_ptr;
	png_infop           info_ptr;
	unsigned char      *data, *ptr, **lines, *ptr2, r, g, b, a, *aptr;
	int                 i, x, y, transp, bit_depth, color_type, interlace_type;
	png_uint_32         ww, hh;
	
	/* Init PNG Reader */
	transp = 0;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return NULL;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	
	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	
	if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	png_init_io(png_ptr, f);

	/* Read Header */
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &ww, &hh, &bit_depth, &color_type, &interlace_type,
		     NULL, NULL);
	*w = ww;
	*h = hh;
	/* Setup Translators */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	data = malloc(*w ** h * 3);
	if (!data)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	if (color_type != PNG_COLOR_TYPE_GRAY){
		*alpha = malloc(*w * *h);
		if (!*alpha)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return NULL;
		}
		transp = 1;
	} else {
		*alpha = NULL;
		transp = 0;
	}
	
	lines = (unsigned char **)malloc(*h * sizeof(unsigned char *));
	
	if (lines == NULL)
	{
		free(data);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	for (i = 0; i < *h; i++)
	{
		if ((lines[i] = malloc(*w * (sizeof(unsigned char) * 4))) == NULL)
		{
			int                 n;
			
			free(data);
			free(*alpha);
			for (n = 0; n < i; n++)
				free(lines[n]);
			free(lines);
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return NULL;
		}
	}
	png_read_image(png_ptr, lines);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	ptr = data;
	aptr = *alpha;
	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		for (y = 0; y < *h; y++)
		{
			ptr2 = lines[y];
			for (x = 0; x < *w; x++)
			{
				r = *ptr2++;
				*aptr++ = *ptr2++;
				*ptr++ = r;
				*ptr++ = r;
				*ptr++ = r;
			}
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY)
	{
		for (y = 0; y < *h; y++)
		{
			ptr2 = lines[y];
			for (x = 0; x < *w; x++)
			{
				r = *ptr2++;
				*ptr++ = r;
				*ptr++ = r;
				*ptr++ = r;
			}
		}
	}
	else
	{
		for (y = 0; y < *h; y++)
		{
			ptr2 = lines[y];
			for (x = 0; x < *w; x++)
			{
				*ptr++  = *ptr2++;
				*ptr++  = *ptr2++;
				*ptr++  = *ptr2++;
				*aptr++ = *ptr2++;
			}
		}
	}
	for (i = 0; i < *h; i++)
		free(lines[i]);
	free(lines);
	*t = transp;
	return data;
}

/**
 * gtk_canvas_load_alpha:
 * @file: filename to load
 *
 * This routine loads a PNG file with full alpha transparency and
 * returns a (GdkImlibImage *).
 *
 * Note that images created by this routine are not intended to be
 * passed to GdkImlib, we just use this to remain compatible with
 * the GtkCanvasImage Canvas item.  Use this with GdkImlib at
 * your own risk.
 *
 * To release images loaded by gtk_canvas_load_alpha, use
 * gtk_canvas_destroy_image preferably although it works
 * with imlib now, in the future this might not be the case.
 */
GdkImlibImage *
gtk_canvas_load_alpha (char *file)
{
	FILE *f;
	int w, h, trans;
	GdkImlibImage *im;
	unsigned char *data, *alpha;
	
	g_return_val_if_fail (file != NULL, NULL);

	f = fopen (file, "rb");
	if (!f)
		return NULL;

	data = _gtk_canvas_load_alpha (f, &w, &h, &trans, &alpha);
	fclose (f);

	if (!data)
		return NULL;

	im = (GdkImlibImage *) malloc (sizeof (GdkImlibImage));
	if (!im){
		free (data);
		if (alpha)
			free (alpha);
		return NULL;
	}
	memset (im, 0, sizeof (GdkImlibImage));
	
        im->alpha_data = alpha;
	im->shape_color.r = -1;
	im->shape_color.g = -1;
	im->shape_color.b = -1;
	im->rgb_data = data;
	im->rgb_width = w;
	im->rgb_height = h;

	return im;
}

/**
 * gtk_canvas_destroy_image:
 * @image: A GdkImlibImage allocated by gtk_canvas_load_alpha
 *
 * Do not pass a regular GdkImlibImage to this routine, only pass
 * GdkImlibImage pointer that were created by gtk_canvas_load_alpha
 */
void
gtk_canvas_destroy_image (GdkImlibImage *image)
{
	g_return_if_fail (image != NULL);

	if (image->rgb_data)
		free (image->rgb_data);
	if (image->alpha_data)
		free (image->alpha_data);
	
	free (image);
}
#else

GdkImlibImage *
gtk_canvas_load_alpha (char *file)
{
	return NULL;
}

void
gtk_canvas_destroy_image (GdkImlibImage *image)
{
}
#endif
#endif
