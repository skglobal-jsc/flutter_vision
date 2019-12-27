//
//  TextSorter.h
//  TextSorter
//
//  Created by ptgiang on 10/1/19.
//  Copyright © 2019 ptgiang. All rights reserved.
//

#ifndef PupilDetector_h
#define PupilDetector_h

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

struct Word
{
    vector<Point2i> _vertices;
    Rect2f _rect;
    int _index;
    float _confidence;
    Word(vector<Point2i> vertices, int index, float confidence)
    {
        _vertices = vertices;
        _index = index;
        _rect = boundingRect(_vertices);
        _confidence = confidence;
    }
};

struct Segment
{
    vector<Word> _words;
    Rect2f _rect;
    int _lines;
    Segment(Rect2f rect)
    {
        _rect = rect;
        _lines = 0;
    }
};

struct SegmentColumn
{
    vector<Segment> _segments;
    Rect2f _rect;
    SegmentColumn()
    {
    }
};

struct SegmentGroup
{
    vector<SegmentColumn> _columns;
    SegmentGroup()
    {
    }
};

class TextSorter {
public:
    static vector<vector<int>> sort(Mat& image, vector<Word> words);
    
private:
    static float scaleImageIfNeeded(Mat& image, int maxSize);
    static void correctRotation(Mat& image, vector<Word>& words);
    static float rectDistance(Rect2f rectA, Rect2f rectB);
    static Rect2f mergeRect(Rect2f rectA, Rect2f rectB);
    static void segmentWords(vector<Segment>& segments, vector<Word>& words);
    static void filterSegments(vector<Segment>& segments);
    static void mergeWords(vector<Word>& wordsA, vector<Word>& wordsB);
    static void mergeSegments(vector<Segment>& segsA, vector<Segment>& segsB);
    static int sortWords(vector<Word>& words);
    static SegmentGroup groupSegments(vector<Segment>& segments);
    static void sortSegments(vector<Segment>& segments);
};

#endif /* PupilDetector_h */
