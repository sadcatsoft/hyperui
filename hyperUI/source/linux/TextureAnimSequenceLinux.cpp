#include "stdafx.h"
#include "png.h"

#include "lodepng.h"

GLuint TextureAnimSequence::initGLTexture(const char* pcsName, GLuint spriteTexture, bool bResampleNearest)
{
	if(!pcsName)
		return 0;

	GLenum errCode;
//printf("Texture Load: %s\n",pcsName);
#if 1
	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	//decode
	string strTempName(pcsName);
	unsigned error = lodepng::decode(image, width, height, strTempName, LCT_RGBA, 8);

	//if there's an error, display it
	//if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	if(error)
	{
		Logger::log("TextureAnimSequence::initGLTexture: Could not load %s with error %s.", pcsName, lodepng_error_text(error));
		return 0;
	}

	myWidth = width;
	myHeight = height;

	unsigned char *imagePtr = &image[0];

    // Premultiply the alpha
	int x,y;
	int iOffset;
	FLOAT_TYPE fAlpha;

	for(x = 0; x < width; x++)
	{
        for(y = 0; y < height; y++)
        {
            iOffset = y*width*4 + x*4;
            fAlpha = ((FLOAT_TYPE)imagePtr[iOffset + 3])/255.0;
            imagePtr[iOffset + 0] = (unsigned char)(((FLOAT_TYPE)imagePtr[iOffset + 0])*fAlpha);
            imagePtr[iOffset + 1] = (unsigned char)(((FLOAT_TYPE)imagePtr[iOffset + 1])*fAlpha);
            imagePtr[iOffset + 2] = (unsigned char)(((FLOAT_TYPE)imagePtr[iOffset + 2])*fAlpha);
        }
	}

	glBindTexture(GL_TEXTURE_2D, spriteTexture);
	errCode = glGetError();
	if(errCode != GL_NO_ERROR)
		Logger::log("TextureAnimSequence::initGLTexture error@1 %d", errCode);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) imagePtr);
	errCode = glGetError();
	if(errCode != GL_NO_ERROR)
		Logger::log("TextureAnimSequence::initGLTexture error@2 %d", errCode);
	if(bResampleNearest)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	errCode = glGetError();
	if(errCode != GL_NO_ERROR)
		Logger::log("TextureAnimSequence::initGLTexture error@3 %d", errCode);

//printf("Success: %dx%d\n", myWidth, myHeight);
	return spriteTexture;
#endif


#if 0
//printf("Texture Load: %s\n",pcsName);
	png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    int color_type, interlace_type;
    FILE *fp;

    if ((fp = fopen(pcsName, "rb")) == NULL)
        return false;

    /* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    /* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    /* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        /* If we get here, we had a
         * problem reading the file */
        return false;
    }

    /* Set up the output control if
     * you are using standard C streams */
    png_init_io(png_ptr, fp);

    /* If we have already
     * read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

    /*
     * If you have enough memory to read
     * in the entire image at once, and
     * you need to specify only
     * transforms that can be controlled
     * with one of the PNG_TRANSFORM_*
     * bits (this presently excludes
     * dithering, filling, setting
     * background, and doing gamma
     * adjustment), then you can read the
     * entire image (including pixels)
     * into the info structure with this
     * call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
     png_set_alpha_mode(png_ptr, PNG_ALPHA_BROKEN, PNG_DEFAULT_sRGB);
//    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_INVERT_ALPHA, NULL);


    png_uint_32 width, height;
    int bit_depth;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                 &interlace_type, NULL, NULL);

    int channels = png_get_channels(png_ptr, info_ptr);

    myWidth = width;
    myHeight = height;

    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    unsigned char*image_data = (unsigned char*) malloc(row_bytes * myHeight);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < myHeight; i++)
    {
        // note that png is ordered top to
        // bottom, but OpenGL expect it bottom to top
        // so the order or swapped
       //memcpy((void*)(image_data+(row_bytes * (myHeight-1-i))), row_pointers[i], row_bytes);
memcpy((void*)(image_data+(row_bytes *i)), row_pointers[i], row_bytes);

    }

	//Now generate the OpenGL texture object
	//GLuint texture;
	//glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
	if(bResampleNearest)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}


    /* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    /* Close the file */
    fclose(fp);

	free(image_data);

//printf("Success: %dx%d\n", myWidth, myHeight);
    /* That's it */
	return spriteTexture;
    //return 0;
#endif
}
