//import 'package:flutter/services.dart';
//import 'package:flutter_test/flutter_test.dart';
//import 'package:flutter_vision/flutter_vision.dart';
//
//void main() {
//  const MethodChannel channel = MethodChannel('flutter_vision');
//
//  setUp(() {
//    channel.setMockMethodCallHandler((MethodCall methodCall) async {
//      return '42';
//    });
//  });
//
//  tearDown(() {
//    channel.setMockMethodCallHandler(null);
//  });
//
//  test('getPlatformVersion', () async {
//    expect(await FlutterVision.platformVersion, '42');
//  });
//}