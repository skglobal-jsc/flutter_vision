
import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_vision/flutter_vision.dart';

class VisionModel {
  List<String> texts;
  String lang;

  VisionModel({@required this.texts, @required this.lang});
}

class VisionService {
  final String _key;
  VisionService({@required String key}) : _key = key;

  Future<VisionModel> detectDocumentInImage(String image64, int width, int height) async {
    Response<Map> response = await Dio().post(
        "https://vision.googleapis.com/v1/images:annotate",
        queryParameters: {
          'key': _key
        },
        data: {
          "requests": [
            {
              "image": {"content": image64},
              "features": [
                {"type": "DOCUMENT_TEXT_DETECTION"}
              ]
            }
          ]
        });

    // Validate response
    if (response == null
        || response.data.containsKey('responses') == false
        || (response.data['responses'] as List).length == 0) {
      return null;
    }

    dynamic nativeQueryArguments =
        await compute<Map, dynamic>(_parseToGiangLibModel, response.data);
    // Or throw?
    if (nativeQueryArguments == null) return null;

    List<dynamic> indexes;
    dynamic nativeResponse =
        await FlutterVision.sortWords(image64, width, height, nativeQueryArguments);
    if (nativeResponse is Map && nativeResponse.containsKey('result')) {
      indexes = nativeResponse['result'];
    } else {
      // Show error here or skip it
//      print(nativeResponse['error']);
      return null;
    }
    // Send this to Mr.Giang lib
    List<String> segments = await compute<Map, List<String>>(
        _arrangeWordsToSegments,
        {'indexes': indexes, 'words': nativeQueryArguments['words']});
    print('best lang: ${nativeQueryArguments['bestLang']}');
    return VisionModel(texts: segments, lang: nativeQueryArguments['bestLang']);
  }
}

String _findBestLanguge(List detectedLanguages) {
  // [{"languageCode":"en","confidence":0.51},{"languageCode":"ja","confidence":0.3},{"languageCode":"fi","confidence":0.06},{"languageCode":"gd","confidence":0.03}]
  List langs = detectedLanguages.map((dl) => dl['languageCode']).toList();
  print('langs: $langs');
  if (langs.contains('ja')) {
    return 'ja-JP';
  }
  if (langs.contains('en')) {
    return 'en-US';
  }
  if (langs.contains('vi')) {
    return 'vi-VN';
  }
  return 'en-US';
}

/// Return value:
/// {
///   'words': [],
///   'boxes': [
///     [{'x': 1, 'y': 2}, {'x': 1, 'y': 2}, {'x': 1, 'y': 2}, {'x': 1, 'y': 2}]
///   ],
///   'confidences': []
/// }
/// assert item count equal each other
Future<Map> _parseToGiangLibModel(dynamic response) {
  List blocks = [];
  String bestLang = 'ja-JP';
  String defaultLang = 'en';
  if (response['responses'][0].containsKey('fullTextAnnotation') &&
      response['responses'][0]['fullTextAnnotation'].containsKey('pages')) {
    var pages = response['responses'][0]['fullTextAnnotation']['pages'];
    if (pages is List && pages.length > 0 && pages[0].containsKey('blocks')) {
      blocks = pages[0]['blocks'];
      if (pages[0].containsKey('property')) {
        defaultLang =
            pages[0]['property']['detectedLanguages'][0]['languageCode'];
        bestLang = _findBestLanguge(pages[0]['property']['detectedLanguages']);
      }
    }
  }
  if (blocks.length == 0) {
    // Fail
    return null;
  }

  List boxes = [];
  List<String> texts = [];
  List<double> confidences = [];

  for (int i = 0; i < blocks.length; i++) {
    List paragraphs = blocks[i]['paragraphs'];
    for (int j = 0; j < paragraphs.length; j++) {
      List words = paragraphs[j]['words'];
      for (int k = 0; k < words.length; k++) {
        if (!words[k].containsKey('confidence')) continue;
        String wordLang = defaultLang;
        if (words[k].containsKey('property')) {
          wordLang =
              words[k]['property']['detectedLanguages'][0]['languageCode'];
        }
        if (wordLang == 'en') {
          List symbols = words[k]['symbols'];
          String string = '';
          for (int m = 0; m < symbols.length; m++) {
            dynamic breakType = '';
            if (symbols[m].containsKey('property') &&
                symbols[m]['property'].containsKey('detectedBreak')) {
              breakType = symbols[m]['property']['detectedBreak']['type'];
            }
            if (breakType == 'SPACE' ||
                breakType == 'SURE_SPACE' ||
                breakType == 'EOL_SURE_SPACE') {
              string += (symbols[m]['text']) + ' ';
            } else if (breakType == 'LINE_BREAK') {
              string += (symbols[m]['text']) + '\n';
            } else {
              string += (symbols[m]['text']);
            }
          }
          texts.add(string);
          List box = words[k]['boundingBox']['vertices'];
          boxes.add(box);
          confidences.add(words[k]['confidence'] * 1.0);
        } else {
          List symbols = words[k]['symbols'];
          for (int m = 0; m < symbols.length; m++) {
            if (!symbols[m].containsKey('confidence')) continue;
            List box = symbols[m]['boundingBox']['vertices'];
            boxes.add(box);
            dynamic breakType = '';
            if (symbols[m].containsKey('property') &&
                symbols[m]['property'].containsKey('detectedBreak')) {
              breakType = symbols[m]['property']['detectedBreak']['type'];
            }
            if (breakType == 'SPACE' ||
                breakType == 'SURE_SPACE' ||
                breakType == 'EOL_SURE_SPACE') {
              texts.add((symbols[m]['text']) + ' ');
            } else if (breakType == 'LINE_BREAK') {
              texts.add((symbols[m]['text']) + '\n');
            } else {
              texts.add(symbols[m]['text']);
            }
            confidences.add(symbols[m]['confidence'] * 1.0);
          }
        }
      }
    }
  }
  return Future.value({'words': texts, 'boxes': boxes, 'confidences': confidences, 'bestLang': bestLang});
}

/// Arrage words by index
Future<List<String>> _arrangeWordsToSegments(Map info) {
  List<dynamic> indexes = info['indexes'];
  List<String> words = info['words'];
  List<String> segments = [];
  int segmentCount = indexes.length;
  for (int i = 0; i < segmentCount; i++) {
    String segment = '';
    int wordCount = indexes[i].length;
    for (int j = 0; j < wordCount; j++) {
      int index = indexes[i][j];
      segment += words[index];
    }
    segments.add(segment);
  }
  return Future.value(segments);
}
