import Flutter
import UIKit

public class SwiftFlutterVisionPlugin: NSObject, FlutterPlugin {
    
    let textDetector = TextDetector()
    
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "flutter_vision", binaryMessenger: registrar.messenger())
    let instance = SwiftFlutterVisionPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "vision.sortWords":
        guard let info: Dictionary<String, Any> = call.arguments as? Dictionary<String, Any> else {
            result(nil)
            return
        }
        sortWords(from: info, result: result)
        break
    case "vision.base64String":
        guard let info: Dictionary<String, Any> = call.arguments as? Dictionary<String, Any> else {
            result(nil)
            return
        }
        result(textDetector.bytes(toBase64String: info));
        break;
    default:
        result(FlutterMethodNotImplemented)
    }
  }
    
    private func sortWords(from info: Dictionary<String, Any>, result: @escaping FlutterResult) {
        guard let confidences: Array<Double> = info["confidences"] as? Array<Double>,
            let base64String = info["base64Image"] as? String,
            let data: Data = Data(base64Encoded: base64String, options: NSData.Base64DecodingOptions.ignoreUnknownCharacters),
            let image = UIImage(data: data),
            let boxes = info["boxes"] as? Array<Any> else {
            result(["error": "invalid arguments"])
            return
        }
        
        guard let indexes = textDetector.sortWords(image, boxes: boxes, confidences: confidences) as? Array<Array<Int>> else {
            result(["error": "parse model failed"])
            return
        }
        result(["result": indexes])
    }
}
