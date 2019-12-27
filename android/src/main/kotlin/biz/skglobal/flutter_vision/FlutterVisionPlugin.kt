package biz.skglobal.flutter_vision

import android.util.Base64
import android.util.Log
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.plugin.common.PluginRegistry.Registrar
import com.google.gson.Gson
import org.json.JSONObject
import org.opencv.core.CvType
import org.opencv.core.Mat


class FlutterVisionPlugin : MethodCallHandler {
    companion object {
        @JvmStatic
        fun registerWith(registrar: Registrar) {
            val channel = MethodChannel(registrar.messenger(), "flutter_vision")
            channel.setMethodCallHandler(FlutterVisionPlugin())
        }
    }

    constructor() {
        TextDetectorJNI.newInstance()
    }

    override fun onMethodCall(call: MethodCall, result: Result): Unit =
            if (call.method == "getPlatformVersion") {
                result.success("Android ${android.os.Build.VERSION.RELEASE}")
            } else if (call.method == "vision.sortWords") {
                try {
                    var data: HashMap<String, Any> = call.arguments()
                    var pointsArr: Array<IntArray?> = arrayOfNulls((data["boxes"] as List<Any>).size)
                    var i = 0
                    (data["boxes"] as List<Any>).forEach { item ->
                        if (item is List<*>) {
                            var row = IntArray(item.size * 2)
                            var j = 0
                            item.forEach { item2 ->
                                if (item2 is HashMap<*, *>) {
                                    row[j++] = if (item2.containsKey("x")) item2["x"] as Int else 0
                                    row[j++] = if (item2.containsKey("y")) item2["y"] as Int else 0
                                }
                            }
                            pointsArr[i++] = row
                        }
                    }

                    var mat = convertBased64toMat(data["base64Image"] as String, data["width"] as Int, data["height"] as Int)
                    var arrayIndex = TextDetectorJNI.newInstance().sort(mat, pointsArr, (data["confidences"] as List<Float>).toFloatArray())
                    var indexsData = if (arrayIndex == null) emptyArray() else arrayIndex as Array<Any>
                    result.success(hashMapOf(
                            "result" to indexsData.toList()
                    ))
                } catch (e: Exception) {
                    e.printStackTrace()
                    result.success(hashMapOf(
                            "error" to e.toString()
                    ))
                }
            } else {
                result.notImplemented()
            }

    fun print(msg: Any?) {
        Log.e("VISION", msg?.toString())
    }

    private fun convertBased64toMat(base64Str: String, width: Int, height: Int) : Mat {
        val data = Base64.decode(base64Str, Base64.DEFAULT)
        val mat = Mat(height, width, CvType.CV_8UC1)
        mat.put(0,0,data)
        return mat
    }
}
