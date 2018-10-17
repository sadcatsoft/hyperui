#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"


#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

static char gPVRTexIdentifier[5] = "PVR!";


#ifndef MAC_BUILD
#include "PVRTTexture.h"



extern PVRTuint32 PVRTGetTextureDataSize(PVRTextureHeaderV3 sTextureHeader, PVRTint32 iMipLevel, bool bAllSurfaces, bool bAllFaces);
#endif
namespace HyperUI
{
    #ifndef MAC_BUILD
enum
{
	kPVRTextureFlagTypePVRTC_2 = 24,
	kPVRTextureFlagTypePVRTC_4
};

typedef struct _PVRTexHeader
	{
		uint32_t headerLength;
		uint32_t height;
		uint32_t width;
		uint32_t numMipmaps;
		uint32_t flags;
		uint32_t dataLength;
		uint32_t bpp;
		uint32_t bitmaskRed;
		uint32_t bitmaskGreen;
		uint32_t bitmaskBlue;
		uint32_t bitmaskAlpha;
		uint32_t pvrTag;
		uint32_t numSurfs;
	} PVRTexHeader;


bool unpackData(NSData* data, NSMutableArray *_imageData, GLenum _internalFormat, int& iWidthOut, int &iHeightOut)
{
	BOOL success = FALSE;
	PVRTexHeader *header = NULL;
    PVRTextureHeaderV3 *headerV3 = NULL;
	uint32_t pvrTag;
	uint32_t dataLength = 0, dataOffset = 0, dataSize = 0;
	uint32_t blockSize = 0, widthBlocks = 0, heightBlocks = 0;
	uint32_t width = 0, height = 0, bpp = 4;
	uint8_t *bytes = NULL;
	
	header = (PVRTexHeader *)[data bytes];
	headerV3 = (PVRTextureHeaderV3*)[data bytes];
    
	pvrTag = CFSwapInt32LittleToHost(header->pvrTag);

    bool bIsV2Header = true;
    bool bIsV3Header = true;
    
	if (gPVRTexIdentifier[0] != ((pvrTag >>  0) & 0xff) ||
		gPVRTexIdentifier[1] != ((pvrTag >>  8) & 0xff) ||
		gPVRTexIdentifier[2] != ((pvrTag >> 16) & 0xff) ||
		gPVRTexIdentifier[3] != ((pvrTag >> 24) & 0xff))
	{
        // See if we can convert it to a new v3 header:
        bIsV2Header = false;
	}
    
    if(headerV3->u32Version != PVRTEX3_IDENT)
    {
        bIsV3Header = false;
    }

    if(!bIsV3Header && !bIsV2Header)
        return FALSE;
	
	
    EPVRTPixelFormat v3Format = ePVRTPF_NumCompressedPFs;
    if(bIsV3Header)
        v3Format = (EPVRTPixelFormat)headerV3->u64PixelFormat;
    
    bool bIs2bpp = false;
    bool bIs4bpp = false;
    if(bIsV2Header)
    {
        uint32_t formatFlags, flags;
       
        flags = CFSwapInt32LittleToHost(header->flags);
        formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;
        
        bIs2bpp = formatFlags == kPVRTextureFlagTypePVRTC_2;
        bIs4bpp = formatFlags == kPVRTextureFlagTypePVRTC_4;
    }
    else 
    {
        bIs2bpp = v3Format == ePVRTPF_PVRTCI_2bpp_RGBA || v3Format == ePVRTPF_PVRTCI_2bpp_RGB;
        bIs4bpp = v3Format == ePVRTPF_PVRTCI_4bpp_RGBA || v3Format == ePVRTPF_PVRTCI_4bpp_RGB;
    }
    
	if (bIs2bpp || bIs4bpp)
	{
		[_imageData removeAllObjects];
		
		//if (formatFlags == kPVRTextureFlagTypePVRTC_4)
		if(bIs4bpp)
            _internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		else if(bIs2bpp)
        //else if (formatFlags == kPVRTextureFlagTypePVRTC_2)
			_internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		
		iWidthOut = width = CFSwapInt32LittleToHost(bIsV2Header ? header->width : headerV3->u32Width);
		iHeightOut = height = CFSwapInt32LittleToHost(bIsV2Header ? header->height : headerV3->u32Height);
		/*
		 if (CFSwapInt32LittleToHost(header->bitmaskAlpha))
		 _hasAlpha = TRUE;
		 else
		 _hasAlpha = FALSE;
		 */
		dataLength = CFSwapInt32LittleToHost(bIsV2Header ? header->dataLength : PVRTGetTextureDataSize(*headerV3, 0, true, true));
		
		bytes = ((uint8_t *)[data bytes]) + sizeof(PVRTexHeader);
		
		// Calculate the data size for each texture level and respect the minimum number of blocks
		while (dataOffset < dataLength)
		{
			//if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			if(bIs4bpp)
            {
				blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
				widthBlocks = width / 4;
				heightBlocks = height / 4;
				bpp = 4;
			}
			else
			{
				blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
				widthBlocks = width / 8;
				heightBlocks = height / 4;
				bpp = 2;
			}
			
			// Clamp to minimum number of blocks
			if (widthBlocks < 2)
				widthBlocks = 2;
			if (heightBlocks < 2)
				heightBlocks = 2;
			
			dataSize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
			
			[_imageData addObject:[NSData dataWithBytes:bytes+dataOffset length:dataSize]];
			
			dataOffset += dataSize;
			
			width = MAX(width >> 1, 1);
			height = MAX(height >> 1, 1);
		}
		
		success = TRUE;
	}
	
	return success;	
}

GLuint TextureAnimSequence::initCompressedGLTexture(const char* pcsName, GLuint spriteTexture, bool bResampleNearest)
{
	NSString *path = [[NSBundle mainBundle] pathForResource:[NSString stringWithCString:pcsName] ofType:nil];
	NSData *data = [NSData dataWithContentsOfFile:path];
	
	NSMutableArray* _imageData = [[NSMutableArray alloc] initWithCapacity:10];
	myCompressedImageData = _imageData;
	
	// GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
	//	GLenum _internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
	GLenum _internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
	if(!unpackData(data, _imageData, _internalFormat, myWidth, myHeight))
	{
		_ASSERT(0);
		return spriteTexture;
	}
	
	
	gSetDiffuseTexture<TX_MAN_RETURN_TYPE>(spriteTexture);
	//glBindTexture(GL_TEXTURE_2D, spriteTexture);
	
	// Assume only one level, no mip-mapping
	NSData *data2 = [_imageData objectAtIndex:0];
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, myWidth, myHeight, 0, [data2 length], [data2 bytes]);
	
	// Note: change for uncompressed textures, as well
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
	
	// TEST THIS IN BOTH
	[_imageData release];
	myCompressedImageData = NULL;
	
	return spriteTexture;
	// NSLog(@"Levels: %d\n", [_imageData count]);
}

#endif
/********************************************************************************************/
GLuint TextureAnimSequence::initGLTexture(const char* pcsName, GLuint spriteTexture, bool bResampleNearest)
{
	//GLuint spriteTexture = 0;
	CGImageRef spriteImage = 0;
	CGContextRef spriteContext;
	GLubyte *spriteData;
	size_t	width, height;	
	
#ifdef MAC_BUILD
	NSURL					*url = nil;
	CGImageSourceRef		src;
	
//	NSString *path = [[NSBundle mainBundle] pathForResource:[NSString stringWithCString:pcsName] ofType:nil];
//	url = [NSURL fileURLWithPath:path];
	url = [NSURL fileURLWithPath:[NSString stringWithCString:pcsName]];
    src = CGImageSourceCreateWithURL((CFURLRef)url, NULL);
	if(src)
	{
		spriteImage = CGImageSourceCreateImageAtIndex(src, 0, NULL);
		CFRelease(src);	
	}

#else
	// Creates a Core Graphics image from an image file
	spriteImage = [UIImage imageNamed:[NSString stringWithCString:pcsName]].CGImage;
#endif
	
	// Get the width and height of the image
	width = CGImageGetWidth(spriteImage);
	height = CGImageGetHeight(spriteImage);
	// Texture dimensions must be a power of 2. If you write an application that allows users to supply an image,
	// you'll want to add code that checks the dimensions and takes appropriate action if they are not a power of 2.
	
	myWidth = width;
	myHeight = height;
	
	if(spriteImage) 
	{
		// Allocated memory needed for the bitmap context
		//		spriteData = (GLubyte *) malloc(width * height * 4);
		spriteData = (GLubyte *) calloc(width * height, 4);
		// Uses the bitmatp creation function provided by the Core Graphics framework. 
		spriteContext = CGBitmapContextCreate(spriteData, width, height, 8, width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);
		// After you create the context, you can draw the sprite image to the context.
		CGContextDrawImage(spriteContext, CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height), spriteImage);
		// You don't need the context at this point, so you need to release it to avoid memory leaks.
		CGContextRelease(spriteContext);
		
		// Use OpenGL ES to generate a name for the texture.
		//glGenTextures(1, &spriteTexture);
		// Bind the texture name. 
		//glBindTexture(GL_TEXTURE_2D, spriteTexture);
        GraphicsUtils::setDiffuseTexture(spriteTexture);
		// Speidfy a 2D texture image, provideing the a pointer to the image data in memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spriteData);
		
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
		
		// Release the image data
		free(spriteData);
		
		/*
		 glDeleteTextures(1, &spriteTexture);
		 
		 
		 static int iMemCount = 0;
		 int iOwnMem = width * height * 4;
		 iMemCount += iOwnMem;
		 NSLog(@"Loaded: %s Own Mem: %d Total Mem: %d\n", pcsName, iOwnMem/1024, iMemCount/1024);
		 */
#ifdef MAC_BUILD
		CGImageRelease(spriteImage);
#endif
		
	}
	else
	{
	//	_ASSERT(0);
	}
	
	return spriteTexture;
}
}
