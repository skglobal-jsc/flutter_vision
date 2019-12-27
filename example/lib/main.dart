import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'dart:async';
import 'package:flutter/services.dart';
import 'package:flutter/services.dart' show rootBundle;




void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        floatingActionButton: FloatingActionButton(child: Icon(Icons.visibility), onPressed: () async {
          ByteData bytes = await rootBundle.load('assets/sample.jpeg');
          Uint8List imageBytes = bytes.buffer.asUint8List(bytes.offsetInBytes, bytes.lengthInBytes);
          String image64 = Base64Encoder().convert(imageBytes);
//          dynamic r = await VisionService.instance.detectDocumentInImage(image64);
//          print(r);
        }),
        body: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
            FlatButton(child: Text('Get blocks'), onPressed: _getBlocks),
          ],),
        ),
      ),
    );
  }


  // Logic methods

  _getBlocks() async {
    String jsonString = await rootBundle.loadString('assets/json/ocr-result-sample.json');
    // String jsonString = await rootBundle.loadString('assets/json/1980.json');

    Map response = json.decode(jsonString);

    // final blocks = getBlocksInList(response);
//    final doc =  DocumentOcrResponse.fromJson(response);
//    print(doc);
    // print(blocks);
  }



  // Util methods

  static Map getLanguageCodeInList(Map response) {
    return ((response['responses'] as List).first['fullTextAnnotation']['pages'] as List).first['detectedLanguages'];
  }

  static List<String> getBlocksInList(Map response) {
    List blocks = ((response['responses'] as List).first['fullTextAnnotation']['pages'] as List).first['blocks'] as List;
    List<String> rawBlocksList = blocks.map((item) {
      return (item['paragraphs'] as List).map((p) {
        return (p['words'] as List).map((s) {
          return (s['symbols'] as List).map((t) => t['text']).join();
        }).join(' ');
      }).join('\n').toString();
    }).toList();

    rawBlocksList.forEach((item) => print(item));
    return rawBlocksList;
  }

  static List<String> getBlocksInList2(Map response) {
    List blocks = ((response['responses'] as List).first['fullTextAnnotation']['pages'] as List).first['blocks'] as List;
    List<String> rawBlocksList = blocks.map((item) {
      return (item['paragraphs'] as List).map((p) {
        return (p['words'] as List).map((s) {
          return (s['symbols'] as List).map((t) => t['text']).join();
        }).join(' ');
      }).join('\n').toString();
    }).toList();

    rawBlocksList.forEach((item) => print(item));
    return rawBlocksList;
  }
}
