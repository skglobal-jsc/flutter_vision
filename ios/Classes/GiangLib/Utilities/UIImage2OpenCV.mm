//
//  UIImage_UIImage2OpenCV.mm
//  OpenCV Tutorial
//
//  Created by BloodAxe on 6/25/12.
//  Copyright (c) 2012 computer-vision-talks.com. All rights reserved.
//

#import "UIImage2OpenCV.h"

@implementation UIImage (OpenCV)

- (UIImage *)fixOrientation {
    // No-op if the orientation is already correct
    if (self.imageOrientation == UIImageOrientationUp) return self;
    
    // We need to calculate the proper transformation to make the image upright.
    // We do it in 2 steps: Rotate if Left/Right/Down, and then flip if Mirrored.
    CGAffineTransform transform = CGAffineTransformIdentity;
    
    switch (self.imageOrientation) {
        case UIImageOrientationDown:
        case UIImageOrientationDownMirrored:
            transform = CGAffineTransformTranslate(transform, self.size.width, self.size.height);
            transform = CGAffineTransformRotate(transform, M_PI);
            break;
            
        case UIImageOrientationLeft:
        case UIImageOrientationLeftMirrored:
            transform = CGAffineTransformTranslate(transform, self.size.width, 0);
            transform = CGAffineTransformRotate(transform, M_PI_2);
            break;
            
        case UIImageOrientationRight:
        case UIImageOrientationRightMirrored:
            transform = CGAffineTransformTranslate(transform, 0, self.size.height);
            transform = CGAffineTransformRotate(transform, -M_PI_2);
            break;
        case UIImageOrientationUp:
        case UIImageOrientationUpMirrored:
            break;
    }
    
    switch (self.imageOrientation) {
        case UIImageOrientationUpMirrored:
        case UIImageOrientationDownMirrored:
            transform = CGAffineTransformTranslate(transform, self.size.width, 0);
            transform = CGAffineTransformScale(transform, -1, 1);
            break;
            
        case UIImageOrientationLeftMirrored:
        case UIImageOrientationRightMirrored:
            transform = CGAffineTransformTranslate(transform, self.size.height, 0);
            transform = CGAffineTransformScale(transform, -1, 1);
            break;
        case UIImageOrientationUp:
        case UIImageOrientationDown:
        case UIImageOrientationLeft:
        case UIImageOrientationRight:
            break;
    }
    
    // Now we draw the underlying CGImage into a new context, applying the transform
    // calculated above.
    CGContextRef ctx = CGBitmapContextCreate(NULL, self.size.width, self.size.height,
                                             CGImageGetBitsPerComponent(self.CGImage), 0,
                                             CGImageGetColorSpace(self.CGImage),
                                             CGImageGetBitmapInfo(self.CGImage));
    CGContextConcatCTM(ctx, transform);
    switch (self.imageOrientation) {
        case UIImageOrientationLeft:
        case UIImageOrientationLeftMirrored:
        case UIImageOrientationRight:
        case UIImageOrientationRightMirrored:
            // Grr...
            CGContextDrawImage(ctx, CGRectMake(0,0,self.size.height,self.size.width), self.CGImage);
            break;
            
        default:
            CGContextDrawImage(ctx, CGRectMake(0,0,self.size.width,self.size.height), self.CGImage);
            break;
    }
    
    // And now we just create a new UIImage from the drawing context
    CGImageRef cgimg = CGBitmapContextCreateImage(ctx);
    UIImage *img = [UIImage imageWithCGImage:cgimg];
    CGContextRelease(ctx);
    CGImageRelease(cgimg);
    return img;
}

- (UIImage*) flipHorizontal {
    CGAffineTransform transform = CGAffineTransformIdentity;
    transform = CGAffineTransformTranslate(transform, self.size.width, 0);
    transform = CGAffineTransformScale(transform, -1, 1);
    // Now we draw the underlying CGImage into a new context, applying the transform
    // calculated above.
    CGContextRef ctx = CGBitmapContextCreate(NULL, self.size.width, self.size.height,
                                             CGImageGetBitsPerComponent(self.CGImage), 0,
                                             CGImageGetColorSpace(self.CGImage),
                                             CGImageGetBitmapInfo(self.CGImage));
    CGContextConcatCTM(ctx, transform);
    CGContextDrawImage(ctx, CGRectMake(0,0,self.size.width,self.size.height), self.CGImage);
    
    // And now we just create a new UIImage from the drawing context
    CGImageRef cgimg = CGBitmapContextCreateImage(ctx);
    UIImage *img = [UIImage imageWithCGImage:cgimg];
    CGContextRelease(ctx);
    CGImageRelease(cgimg);
    return img;
}

- (UIImage*) flipVertical {
    CGAffineTransform transform = CGAffineTransformIdentity;
    transform = CGAffineTransformTranslate(transform, self.size.height, 0);
    transform = CGAffineTransformScale(transform, 1, -1);
    // Now we draw the underlying CGImage into a new context, applying the transform
    // calculated above.
    CGContextRef ctx = CGBitmapContextCreate(NULL, self.size.width, self.size.height,
                                             CGImageGetBitsPerComponent(self.CGImage), 0,
                                             CGImageGetColorSpace(self.CGImage),
                                             CGImageGetBitmapInfo(self.CGImage));
    CGContextConcatCTM(ctx, transform);
    CGContextDrawImage(ctx, CGRectMake(0,0,self.size.width,self.size.height), self.CGImage);
    
    // And now we just create a new UIImage from the drawing context
    CGImageRef cgimg = CGBitmapContextCreateImage(ctx);
    UIImage *img = [UIImage imageWithCGImage:cgimg];
    CGContextRelease(ctx);
    CGImageRelease(cgimg);
    return img;
}

-(Mat) toMat
{
    UIImage* image = [self fixOrientation];
    CGImageRef imageRef = image.CGImage;
    
    const int srcWidth        = (int)CGImageGetWidth(imageRef);
    const int srcHeight       = (int)CGImageGetHeight(imageRef);
    //const int stride          = CGImageGetBytesPerRow(imageRef);
    //const int bitPerPixel     = CGImageGetBitsPerPixel(imageRef);
    //const int bitPerComponent = CGImageGetBitsPerComponent(imageRef);
    //const int numPixels       = bitPerPixel / bitPerComponent;
    
    // CGDataProviderRef dataProvider = CGImageGetDataProvider(imageRef);
    // CFDataRef rawData = CGDataProviderCopyData(dataProvider);
    
    //unsigned char * dataPtr = const_cast<unsigned char*>(CFDataGetBytePtr(rawData));
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    Mat rgbaContainer(srcHeight, srcWidth, CV_8UC4);
    CGContextRef context = CGBitmapContextCreate(rgbaContainer.data,
                                                 srcWidth,
                                                 srcHeight,
                                                 8,
                                                 4 * srcWidth,
                                                 colorSpace,
                                                 kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Big);
    
    CGContextDrawImage(context, CGRectMake(0, 0, srcWidth, srcHeight), imageRef);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    // CFRelease(rawData);
    
    Mat t;
    cvtColor(rgbaContainer, t, COLOR_RGBA2BGRA);
    
    //Vec4b a = rgbaContainer.at<Vec4b>(0,0);
    //Vec4b b = t.at<Vec4b>(0,0);
    //std::cout << std::hex << (int)a[0] << " "<< (int)a[1] << " " << (int)a[2] << " "  << (int)a[3] << std::endl;
    //std::cout << std::hex << (int)b[0] << " "<< (int)b[1] << " " << (int)b[2] << " "  << (int)b[3] << std::endl;
    
    return t;
}

-(Mat) toMatAlpha
{
    UIImage* image = [self fixOrientation];
    CGImageRef imageRef = image.CGImage;
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(imageRef);
    CGFloat cols = (int)CGImageGetWidth(imageRef);
    CGFloat rows = (int)CGImageGetHeight(imageRef);
    
    Mat cvMat(rows, cols, CV_8UC4); // 8 bits per component, 4 channels (color channels + alpha)
    
    CGContextRef contextRef = CGBitmapContextCreate(cvMat.data,                 // Pointer to  data
                                                    cols,                       // Width of bitmap
                                                    rows,                       // Height of bitmap
                                                    8,                          // Bits per component
                                                    cvMat.step[0],              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    (kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst)); // Bitmap info flags
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), imageRef);
    CGContextRelease(contextRef);
    
    return cvMat;
}

+(UIImage*) imageWithMat:(const Mat&)image
{
    Mat rgbaView;
    
    if (image.channels() == 3)
    {
        cvtColor(image, rgbaView, COLOR_BGR2RGBA);
    }
    else if (image.channels() == 4)
    {
        cvtColor(image, rgbaView, COLOR_BGRA2RGBA);
    }
    else if (image.channels() == 1)
    {
        cvtColor(image, rgbaView, COLOR_GRAY2RGBA);
    }
    
    NSData *data = [NSData dataWithBytes:rgbaView.data length:rgbaView.elemSize() * rgbaView.total()];
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    
    CGBitmapInfo bmInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
    
    // Creating CGImage from Mat
    CGImageRef imageRef = CGImageCreate(rgbaView.cols,                              //width
                                        rgbaView.rows,                              //height
                                        8,                                          //bits per component
                                        8 * rgbaView.elemSize(),                    //bits per pixel
                                        rgbaView.step.p[0],                         //bytesPerRow
                                        colorSpace,                                 //colorspace
                                        bmInfo,// bitmap info
                                        provider,                                   //CGDataProviderRef
                                        NULL,                                       //decode
                                        false,                                      //should interpolate
                                        kCGRenderingIntentDefault                   //intent
                                        );
    
    // Getting UIImage from CGImage
    UIImage *finalImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return finalImage;
}

@end
