//
//  TextSorter.h
//  TextSorter
//
//  Created by ptgiang on 10/1/19.
//  Copyright © 2019 ptgiang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface TextDetector : NSObject
- (NSArray*)sortWords:(UIImage*)image
                     boxes:(NSArray*)boxes
               confidences:(NSArray*)confidences;
- (NSString*)bytesToBase64String:(NSDictionary*)metaData;
//- (UIImage*) sortDebug:(UIImage*)image boxes:(NSArray*)boxes words:(NSArray*)words confidences:(NSArray*)confidences;
//- (NSArray*) sort:(UIImage*)image boxes:(NSArray*)boxes words:(NSArray*)words confidences:(NSArray*)confidences;
@end
