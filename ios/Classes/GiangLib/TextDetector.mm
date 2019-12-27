//
//  MovingTracker.m
//  TextSorter
//
//  Created by ptgiang on 10/1/19.
//  Copyright © 2019 ptgiang. All rights reserved.
//

#import "TextDetector.h"
#import "UIImage2OpenCV.h"
#import "TextSorter.hpp"
#import "ImageBase64Converter.h"

@interface TextDetector()
{
    ImageBase64Converter *converter;
}

@end

@implementation TextDetector
- (id) init
{
    self = [super init];
    if (self) {
        converter = [[ImageBase64Converter alloc] init];
    }
    return self;
}

#pragma mark - API Function
- (NSArray*)sortWords:(UIImage*)image
                     boxes:(NSArray*)boxes
               confidences:(NSArray*)confidences {
    Mat mat = [image toMat];
    vector<Word> _boxes;
    for (int i=0;i < boxes.count;i ++) {
       NSArray* vertices = boxes[i];
       vector<Point2i> _vertices;
       for (int j = 0; j < vertices.count; j++) {
           int x = [(NSNumber*)[vertices[j] objectForKey:@"x"] intValue];
           int y = [(NSNumber*)[vertices[j] objectForKey:@"y"] intValue];
           _vertices.push_back(Point2i(x, y));
       }
       Word word(_vertices, i, [confidences[i] floatValue]);
       _boxes.push_back(word);
    }
    vector<vector<int>> segments = TextSorter::sort(mat, _boxes);
    NSMutableArray* retSegments = [[NSMutableArray alloc] init];
    int segmentCount = (int)segments.size();
    for (int i=0;i < segmentCount;i ++) {
        NSMutableArray *eachSegment = [NSMutableArray new];
        int wordCount = (int)segments[i].size();
        for (int j = 0; j < wordCount; j++) {
            int wordId = segments[i][j];
            [eachSegment addObject:[NSNumber numberWithInt:wordId]];
        }
        [retSegments addObject:eachSegment];
    }
    return retSegments;
}

- (NSString*)bytesToBase64String:(NSDictionary*)metaData {
    return [converter bytesToBase64String:metaData];
}
//- (UIImage*) sortDebug:(UIImage*)image boxes:(NSArray*)boxes words:(NSArray*)words confidences:(NSArray*)confidences
//{
//    Mat mat = [image toMat];
//    vector<Word> _boxes;
//    for (int i=0;i < boxes.count;i ++) {
//        NSArray* vertices = boxes[i];
//        vector<Point2i> _vertices;
//        for (int j=0;j < vertices.count;j ++) {
//            CGPoint point = [vertices[j] CGPointValue];
//            _vertices.push_back(Point2i(point.x, point.y));
//        }
//        Word word(_vertices, i, [confidences[i] floatValue]);
//        _boxes.push_back(word);
//    }
//    vector<vector<int>> result = TextSorter::sort(mat, _boxes);
//    return [UIImage imageWithMat:mat];
//}
//
//- (NSArray*) sort:(UIImage*)image boxes:(NSArray*)boxes words:(NSArray*)words confidences:(NSArray*)confidences
//{
//    Mat mat = [image toMat];
//    vector<Word> _boxes;
//    for (int i=0;i < boxes.count;i ++) {
//        NSArray* vertices = boxes[i];
//        vector<Point2i> _vertices;
//        for (int j=0;j < vertices.count;j ++) {
//            CGPoint point = [vertices[j] CGPointValue];
//            _vertices.push_back(Point2i(point.x, point.y));
//        }
//        Word word(_vertices, i, [confidences[i] floatValue]);
//        _boxes.push_back(word);
//    }
//    vector<vector<int>> segments = TextSorter::sort(mat, _boxes);
//    NSMutableArray* retSegments = [[NSMutableArray alloc] init];
//    int segmentCount = (int)segments.size();
//    for (int i=0;i < segmentCount;i ++) {
//        NSString* content = @"";
//        int wordCount = (int)segments[i].size();
//        for (int j=0;j < wordCount;j ++) {
//            int wordId = segments[i][j];
//            content = [content stringByAppendingString:words[wordId]];
//        }
//        [retSegments addObject:content];
//    }
//    return retSegments;
//}
    
@end
