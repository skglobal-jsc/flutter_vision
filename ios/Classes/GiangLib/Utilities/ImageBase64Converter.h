//
//  ImageBase64Converter.h
//  flutter_vision
//
//  Created by Tran Ngoc Phuc on 12/5/19.
//

#import <Flutter/Flutter.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ImageBase64Converter : NSObject
- (NSString *)bytesToBase64String:(NSDictionary *)imageData;
@end

NS_ASSUME_NONNULL_END
