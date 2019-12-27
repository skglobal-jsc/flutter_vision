#import "FlutterVisionPlugin.h"
#import <flutter_vision/flutter_vision-Swift.h>

@implementation FlutterVisionPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftFlutterVisionPlugin registerWithRegistrar:registrar];
}
@end
