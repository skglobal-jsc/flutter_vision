//
//  UIImage_UIImage2OpenCV.h
//  OpenCV Tutorial
//
//  Created by BloodAxe on 6/25/12.
//  Copyright (c) 2012 computer-vision-talks.com. All rights reserved.
//

// This interface extension allows convert UIImage to Mat representation and
// vice versa using full data copy in both directions.
#import <UIKit/UIKit.h>
#import <opencv2/imgproc/imgproc.hpp>
using namespace cv;

@interface UIImage (OpenCV)
// fix orientation to make orientation and its data get match
- (UIImage*) fixOrientation;
- (UIImage*) flipHorizontal;
- (UIImage*) flipVertical;

// function for transforming between UIImage and openve-Mat
-(Mat) toMat;
-(Mat) toMatAlpha;
+(UIImage*) imageWithMat:(const Mat&)image;

@end
