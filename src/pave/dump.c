/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: dump.c 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://cjcoats.github.io/pave/PaveManual.html
 *      FAQ:        https://cjcoats.github.io/pave/Pave.FAQ.html
 *
 ****************************************************************************
// File:        dump.c
 * Author:      Suresh Balu
 * Date:        May, 1995
 * Purpose:     To dump an image of a window to disk.
 ****************************************************************************
 *
 *  Modification history:
 *
 *  9605   SB  Implemented
 *  960523 SRT Integrated into PAVE's SCCS directories and source code;
 *         removed frame_num arg to write_frame()
 *
 ****************************************************************************/

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <X11/Xos.h>

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Xmu/WinUtil.h>
typedef unsigned long Pixel;
#include <X11/XWDFile.h>

#define FEEP_VOLUME 0

/* Include routines to do parsing */
/* #include "dsimple.h" */

/* Setable Options */

static int  format = ZPixmap;
static Bool nobdrs = False;
static Bool on_root = False;
static Bool standard_out = True;
static Bool debug = False;
static Bool use_installed = False;
static long add_pixel_value = 0;
static char * program_name = "PAVE" ;
static Display *dpy;                                 /* The current display */
static int      screen ;

extern int ( *_XErrorFunction ) ();
extern int _XDefaultError();


#define lowbit(x) ((x) & (~(x) + 1))

static void _swapshort ( bp, n )
register char *bp;
register unsigned n;
    {
    register char c;
    register char *ep = bp + n;

    while ( bp < ep )
        {
        c = *bp;
        *bp = * ( bp + 1 );
        bp++;
        *bp++ = c;
        }
    }

static void _swaplong ( bp, n )
register char *bp;
register unsigned n;
    {
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while ( bp < ep )
        {
        sp = bp + 3;
        c = *sp;
        *sp = *bp;
        *bp++ = c;
        sp = bp + 1;
        c = *sp;
        *sp = *bp;
        *bp++ = c;
        bp += 2;
        }
    }


/********** Error - Fatal xwd error.****************************/

static void Error ( char *string )
    {
    fprintf ( stderr,"\nxwd: Error => %s\n", string );
    if ( errno != 0 )
        {
        perror ( "xwd" );
        fprintf ( stderr,"\n" );
        }

    exit ( 1 );
    }

static void Fatal_Error ( msg, arg0,arg1,arg2,arg3,arg4,arg5,arg6 )
char *msg;
char *arg0, *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
    {
    fflush ( stdout );
    fflush ( stderr );
    fprintf ( stderr, "PAVE: error: " );
    fprintf ( stderr, msg, arg0, arg1, arg2, arg3, arg4, arg5, arg6 );
    fprintf ( stderr, "\n" );
    exit ( 1 );
    }


/*********** Determine the pixmap size.  ********************/

static int Image_Size ( XImage *image )
    {
    if ( image->format != ZPixmap )
        return ( image->bytes_per_line * image->height * image->depth );

    return ( image->bytes_per_line * image->height );
    }


/********** Get the XColors of all pixels in image - returns # of colors  */

static int Get_XColors ( XWindowAttributes *win_info, XColor **colors )
    {
    int i, ncolors;
    Colormap cmap = win_info->colormap;

    if ( use_installed )
        /* assume the visual will be OK ... */
        cmap = XListInstalledColormaps ( dpy, win_info->root, &i ) [0];
    if ( !cmap )
        return ( 0 );

    ncolors = win_info->visual->map_entries;
    if ( ! ( *colors = ( XColor * ) malloc ( sizeof ( XColor ) * ncolors ) ) )
        Fatal_Error ( "Out of memory!", NULL,  NULL, NULL,
                      NULL,  NULL,  NULL,  NULL );


    if ( win_info->visual->class == DirectColor ||
            win_info->visual->class == TrueColor )
        {
        Pixel red, green, blue, red1, green1, blue1;

        red = green = blue = 0;
        red1 = lowbit ( win_info->visual->red_mask );
        green1 = lowbit ( win_info->visual->green_mask );
        blue1 = lowbit ( win_info->visual->blue_mask );
        for ( i=0; i<ncolors; i++ )
            {
            ( *colors ) [i].pixel = red|green|blue;
            ( *colors ) [i].pad = 0;
            red += red1;
            if ( red > win_info->visual->red_mask )
                red = 0;
            green += green1;
            if ( green > win_info->visual->green_mask )
                green = 0;
            blue += blue1;
            if ( blue > win_info->visual->blue_mask )
                blue = 0;
            }
        }
    else
        {
        for ( i=0; i<ncolors; i++ )
            {
            ( *colors ) [i].pixel = i;
            ( *colors ) [i].pad = 0;
            }
        }

    XQueryColors ( dpy, cmap, *colors, ncolors );

    return ( ncolors );
    }


int write_frame ( Display *display0, Window target_win, char *file_name )
    {

    FILE *out_file;
    int returnval;

    dpy = display0;
    screen = DefaultScreen ( dpy );

    if ( ( out_file= fopen ( file_name,"w" ) ) == NULL )
        {
        fprintf ( stderr,"Unable to open %s\n",file_name );
        return 1;
        }

    /*
     * Dump it!
     */
    returnval = Window_Dump ( target_win, out_file );

    if ( fclose ( out_file ) )
        {
        perror ( "xwd" );
        }

    return returnval;
    }


/****************************************************************************
 * Window_Dump: dump a window to an XWD file which must already be open for
 *              writing.
 */

int  Window_Dump ( Window window, FILE *out )
    {
    unsigned long swaptest = 1;
    XColor *colors;
    unsigned buffer_size;
    int win_name_size;
    int header_size;
    int ncolors, i;
    char *win_name;
    Bool got_win_name;
    XWindowAttributes win_info;
    XImage *image;
    int absx, absy, x, y;
    unsigned width, height;
    int dwidth, dheight;
    int bw;
    Window dummywin;
    XWDFileHeader header;
    XWDColor xwdcolor;
    /*
     * Get the parameters of the window being dumped.
     */
    /* if (debug) outl("xwd: Getting target window information.\n"); */
    if ( !XGetWindowAttributes ( dpy, window, &win_info ) )
        Fatal_Error ( "Can't get target window attributes.", NULL,  NULL, NULL,
                      NULL,  NULL,  NULL,  NULL );

    /* handle any frame window */
    if ( !XTranslateCoordinates ( dpy, window, RootWindow ( dpy, screen ), 0, 0,
                                  &absx, &absy, &dummywin ) )
        {
        fprintf ( stderr,
                  "PAVE:  unable to translate window coordinates (%d,%d)\n",
                  absx, absy );
        return 1; /* exit (1); SRT */
        }
    win_info.x = absx;
    win_info.y = absy;
    width = win_info.width;
    height = win_info.height;
    bw = 0;

    if ( !nobdrs )
        {
        absx -= win_info.border_width;
        absy -= win_info.border_width;
        bw = win_info.border_width;
        width += ( 2 * bw );
        height += ( 2 * bw );
        }
    dwidth  = DisplayWidth ( dpy, screen );
    dheight = DisplayHeight( dpy, screen );


    /* clip to window */
    if ( absx < 0 ) width += absx, absx = 0;
    if ( absy < 0 ) height += absy, absy = 0;
    if ( absx + width > dwidth ) width = dwidth - absx;
    if ( absy + height > dheight ) height = dheight - absy;

    XFetchName ( dpy, window, &win_name );
    if ( !win_name || !win_name[0] )
        {
        win_name = "xwdump";
        got_win_name = False;
        }
    else
        {
        got_win_name = True;
        }

    /* sizeof(char) is included for the null string terminator. */
    win_name_size = strlen ( win_name ) + sizeof ( char );

    /*
     * Snarf the pixmap with XGetImage.
     */

    x = absx - win_info.x;
    y = absy - win_info.y;
    if ( on_root )
        image = XGetImage ( dpy, RootWindow ( dpy, screen ), absx, absy, width, height, AllPlanes, format );
    else
        image = XGetImage ( dpy, window, x, y, width, height, AllPlanes, format );
    if ( !image )
        {
        fprintf ( stderr, "%s:  unable to get image at %dx%d+%d+%d\n",
                  program_name, width, height, x, y );
        return 1; /* exit (1); SRT */
        }

    if ( add_pixel_value != 0 ) XAddPixel ( image, add_pixel_value );

    /*
     * Determine the pixmap size.
     */
    buffer_size = Image_Size ( image );

    /* if (debug) outl("xwd: Getting Colors.\n"); */

    ncolors = Get_XColors ( &win_info, &colors );

    /*
     * Calculate header size.
     */
    /* if (debug) outl("xwd: Calculating header size.\n"); */
    header_size = SIZEOF ( XWDheader ) + win_name_size;

    /*
     * Write out header information.
     */
    /* if (debug) outl("xwd: Constructing and dumping file header.\n");
    */
    header.header_size = ( CARD32 ) header_size;
    header.file_version = ( CARD32 ) XWD_FILE_VERSION;
    header.pixmap_format = ( CARD32 ) format;
    header.pixmap_depth = ( CARD32 ) image->depth;
    header.pixmap_width = ( CARD32 ) image->width;
    header.pixmap_height = ( CARD32 ) image->height;
    header.xoffset = ( CARD32 ) image->xoffset;
    header.byte_order = ( CARD32 ) image->byte_order;
    header.bitmap_unit = ( CARD32 ) image->bitmap_unit;
    header.bitmap_bit_order = ( CARD32 ) image->bitmap_bit_order;
    header.bitmap_pad = ( CARD32 ) image->bitmap_pad;
    header.bits_per_pixel = ( CARD32 ) image->bits_per_pixel;
    header.bytes_per_line = ( CARD32 ) image->bytes_per_line;
    header.visual_class = ( CARD32 ) win_info.visual->class;
    header.red_mask = ( CARD32 ) win_info.visual->red_mask;
    header.green_mask = ( CARD32 ) win_info.visual->green_mask;
    header.blue_mask = ( CARD32 ) win_info.visual->blue_mask;
    header.bits_per_rgb = ( CARD32 ) win_info.visual->bits_per_rgb;
    header.colormap_entries = ( CARD32 ) win_info.visual->map_entries;
    header.ncolors = ncolors;
    header.window_width = ( CARD32 ) win_info.width;
    header.window_height = ( CARD32 ) win_info.height;
    header.window_x = absx;
    header.window_y = absy;
    header.window_bdrwidth = ( CARD32 ) win_info.border_width;

    if ( * ( char * ) &swaptest )
        {
        _swaplong ( ( char * ) &header, sizeof ( header ) );
        for ( i = 0; i < ncolors; i++ )
            {
            _swaplong ( ( char * ) &colors[i].pixel, sizeof ( long ) );
            _swapshort ( ( char * ) &colors[i].red, 3 * sizeof ( short ) );
            }
        }

    if ( fwrite ( ( char * ) &header, SIZEOF ( XWDheader ), 1, out ) != 1 ||
            fwrite ( win_name, win_name_size, 1, out ) != 1 )
        {
        perror ( "xwd" );
        return 1; /* exit (1); SRT */
        }

    /*
     * Write out the color maps, if any
     */

    /* if (debug) outl("xwd: Dumping %d colors.\n", ncolors); */
    for ( i = 0; i < ncolors; i++ )
        {
        xwdcolor.pixel = colors[i].pixel;
        xwdcolor.red = colors[i].red;
        xwdcolor.green = colors[i].green;
        xwdcolor.blue = colors[i].blue;
        xwdcolor.flags = colors[i].flags;
        if ( fwrite ( ( char * ) &xwdcolor, SIZEOF ( XWDColor ), 1, out ) != 1 )
            {
            perror ( "xwd" );
            return 1; /* exit (1); SRT */
            }
        }

    /*
     * Write out the buffer.
     */
    /* if (debug) outl("xwd: Dumping pixmap.  bufsize=%d\n",buffer_size);
    */

    /*
     *    This copying of the bit stream (data) to a file is to be replaced
     *  by an Xlib call which hasn't been written yet.  It is not clear
     *  what other functions of xwd will be taken over by this (as yet)
     *  non-existant X function.
     */
    if ( fwrite ( image->data, ( int ) buffer_size, 1, out ) != 1 )
        {
        perror ( "xwd" );
        return 1; /* exit (1); SRT */
        }

    /*
     * free the color buffer.
     */

    /* if(debug && ncolors > 0) outl("xwd: Freeing colors.\n"); */
    if ( ncolors > 0 ) free ( colors );

    /*
     * Free window name string.
     */
    /* if (debug) outl("xwd: Freeing window name string.\n"); */
    if ( got_win_name ) XFree ( win_name );

    /*
     * Free image
     */
    XDestroyImage ( image );

    return 0; /* SRT */
    }


