package biz.skglobal.flutter_vision;

import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

public class TextDetectorJNI {
    // Used to load the 'moving_detector' library on application startup.
    static {
        System.loadLibrary("text_detector");
        if (!OpenCVLoader.initDebug()) {
            // Handle initialization error
        }
    }

    // Singleton
    private static TextDetectorJNI textDetectorJNI;
    public static TextDetectorJNI newInstance() {
        if (textDetectorJNI == null) {
            textDetectorJNI = new TextDetectorJNI();
        }
        return textDetectorJNI;
    }
    // Singleton-end

    Object[] sort(final Mat image, int[][] points, float[] confidences) {
        return sort(image.nativeObj, points, confidences);
    }

    // JNI
    public native Object[] sort(long image, int[][] points, float[] confidences);
    // JNI-end

}

