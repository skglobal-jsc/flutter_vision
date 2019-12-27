import 'dart:async';

import 'package:flutter/services.dart';

class FlutterVision {
  static const MethodChannel _channel = const MethodChannel('flutter_vision');

  static Future<dynamic> sortWords(String base64Image, int width, int height, Map rawInfo) {
    Map arguments = {
      'base64Image': base64Image,
      'width': width,
      'height': height
    };
    rawInfo.forEach((key, value) {
      arguments[key] = value;
    });
    return _channel.invokeMethod('vision.sortWords', arguments);
  }

  static Future<String> base64FromInfo(Map info) {
    return _channel.invokeMethod('vision.base64String', info);
  }
}
