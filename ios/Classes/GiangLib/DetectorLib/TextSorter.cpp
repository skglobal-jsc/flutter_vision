//
//  TextSorter.cpp
//  TextSorter
//
//  Created by ptgiang on 10/1/19.
//  Copyright © 2019 ptgiang. All rights reserved.
//

#include "TextSorter.hpp"
#include <opencv2/imgproc.hpp>

#define STANDARD_DETECT_SIZE 640.0
#define LIMIT_ANGLE 0.2f
#define LIMIT_WORD_CONFIDENCE 0.5f
#define LIMIT_SEGMENT_CONFIDENCE 0.75f
#define WORD_HORIZONTAL_EXPAND 0.45f
#define WORD_VERTICAL_EXPAND 0.9f
#define HORIZONTAL_OVERLAP_BIAS 5
#define MAGNETIC_DISTANCE 20

/************************************************************/
// API function
/************************************************************/
vector<vector<int>> TextSorter::sort(Mat& image, vector<Word> words)
{
    // vertices in boxes is clockwise start from top left
    correctRotation(image, words);

    // scale for performance optimize
    Mat detectImage = image.clone();
    float scale = scaleImageIfNeeded(detectImage, STANDARD_DETECT_SIZE);

    // scale word vertices
    // also find standard word height (the height which almost all word have)
    vector<float> wordHeights;
    vector<int> wordHeightCount;
    int wordCount = (int)words.size();
    for (int i=0;i < wordCount;i ++) {
        for (int j=0;j < 4;j ++) {
            words[i]._vertices[j] *= scale;
        }
        words[i]._rect.x *= scale;
        words[i]._rect.y *= scale;
        words[i]._rect.width *= scale;
        words[i]._rect.height *= scale;
        float wordHeight = words[i]._rect.height;
        bool match = false;
        for (int j=(int)wordHeights.size()-1;j >= 0;j --) {
            if (fabs(wordHeight-wordHeights[j]) < 3) {
                wordHeightCount[j] ++;
                match = true;
                break;
            }
        }
        if (!match) {
            wordHeights.push_back(wordHeight);
            wordHeightCount.push_back(1);
        }
    }

    int maxIndex = 0;
    for (int i=(int)wordHeightCount.size()-1;i >= 0;i --) {
        if (wordHeightCount[i] > wordHeightCount[maxIndex]) {
            maxIndex = i;
        }
    }
    float standardWordHeight = wordHeights[maxIndex];
    float verticeExpand = WORD_VERTICAL_EXPAND*standardWordHeight;

    // filter wrong order word
    for (int i=wordCount-1;i >= 0;i --) {
        Point2i normal = words[i]._vertices[2]-words[i]._vertices[3];
        if (fabs(atan2(normal.y, normal.x)) > LIMIT_ANGLE) {
            words.erase(words.begin()+i);
        }
    }

    // create words mask
    Mat boxMask = Mat::zeros(detectImage.rows, detectImage.cols, CV_8U);
    wordCount = (int)words.size();
    for (int i=0;i < wordCount;i ++) {
        if (words[i]._confidence < LIMIT_WORD_CONFIDENCE) continue;
        Rect rect = words[i]._rect;
        float hscale = WORD_HORIZONTAL_EXPAND*rect.height;
        float vscale = WORD_VERTICAL_EXPAND*rect.height;
        if (vscale > verticeExpand) vscale = verticeExpand;
        rect.x -= 0.5f*hscale;
        rect.y -= 0.5f*vscale;
        rect.width += hscale;
        rect.height += vscale;
        rectangle(boxMask, rect, Scalar(255), CV_FILLED);
    }

    // remove wrong connection
    vector<vector<Point2i>> contours;
    findContours(boxMask, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    boxMask = Mat::zeros(boxMask.rows, boxMask.cols, CV_8U);
    for (int i=0;i < (int)contours.size();i ++) {
        vector<vector<Point2i>> _contours;
        _contours.push_back(contours[i]);
        drawContours(boxMask, _contours, -1, Scalar(255), CV_FILLED);
    }

    // find segment boxes
    contours.clear();
    findContours(boxMask, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    boxMask = Mat::zeros(boxMask.rows, boxMask.cols, CV_8U);
    float minArea = 0.015f*STANDARD_DETECT_SIZE;
    minArea *= minArea;
    vector<Segment> segments;
    for (int i=0;i < (int)contours.size();i ++) {
        Rect rect = boundingRect(contours[i]);
        if (rect.area() < minArea) continue;
        segments.push_back(Segment(rect));
        rectangle(boxMask, rect, Scalar(255), CV_FILLED);
    }

    // separate words into nearby segment
    segmentWords(segments, words);
    filterSegments(segments);

    // sort word and sort segment
    int segmentCount = (int)segments.size();
    for (int i=0;i < segmentCount;i ++) {
        segments[i]._lines = sortWords(segments[i]._words);
    }
    vector<Segment> verticalSegments;
    for (int i=segmentCount-1;i >= 0;i --) {
        if (segments[i]._lines > 1 && segments[i]._words.size() == segments[i]._lines) {
            verticalSegments.push_back(segments[i]);
            segments.erase(segments.begin()+i);
        }
    }
    // TODO: process for vertical line

    sortSegments(segments);

    // create return data
    segmentCount = (int)segments.size();
    vector<vector<int>> flatSegments;
    for (int i=0;i < segmentCount;i ++) {
        vector<int> flatSegment;
        int wordCount = (int)segments[i]._words.size();
        for (int j=0;j < wordCount;j ++) {
            flatSegment.push_back(segments[i]._words[j]._index);
        }
        flatSegments.push_back(flatSegment);
    }

    // debug
    // detectImage = Mat::zeros(boxMask.rows, boxMask.cols, CV_8U);
    for (int i=0;i < segmentCount;i ++) {
        rectangle(detectImage, segments[i]._rect, Scalar(255));
        Rect rect = segments[i]._rect;
        ostringstream oss;
        oss.str(""); oss << i;
        putText(detectImage, oss.str().c_str(), Point2i(rect.x, rect.y), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255));
//        int wordCount = (int)segments[i]._words.size();
//        for (int j=0;j < wordCount;j ++) {
//            Rect rect = segments[i]._words[j]._rect;
//            int order = j % 10;
//            ostringstream oss;
//            oss.str(""); oss << order;
//            putText(detectImage, oss.str().c_str(), Point2i(rect.x, rect.y), FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar(255,255,255));
//        }
    }
    detectImage.copyTo(image);

    return flatSegments;
}

/************************************************************/
// Support function
/************************************************************/
float TextSorter::scaleImageIfNeeded(Mat& image, int maxSize)
{
    float width = image.size().width;
    float height = image.size().height;
    // in limit then do nothing
    if (width <= maxSize && height <= maxSize) return 1;
    // do scale
    float scaleW = maxSize / width;
    float scaleH = maxSize / height;
    float scale = scaleW < scaleH ? scaleW : scaleH;
    resize(image, image, Size2i(width*scale, height*scale), 0, 0, INTER_AREA);
    return scale;
}

void TextSorter::correctRotation(Mat& image, vector<Word>& words)
{
    int count = (int)words.size();
    vector<Point2f> slots;
    vector<int> slotsSize;
    for (int i=0;i < count;i ++) {
        Point2f direct = words[i]._vertices[1]-words[i]._vertices[0];
        float length = sqrt(direct.dot(direct));
        if (length < 1) continue;
        direct *= 1.0f/length;
        float bias = 0.05f; // ~ 3 deg
        int slot = -1;
        for (int j=(int)slots.size()-1;j >= 0;j --) {
            Point2f normal = direct-slots[j];
            if (normal.dot(normal) < bias) {
                slot = j;
                break;
            }
        }
        if (slot >= 0) {
            slots[slot] = (slots[slot]*slotsSize[slot] + direct) / (slotsSize[slot]+1);
            slots[slot] *= 1.0f/sqrt(slots[slot].dot(slots[slot]));
            slotsSize[slot] ++;
        } else {
            slots.push_back(direct);
            slotsSize.push_back(1);
        }
    }
    int slotCount = (int)slots.size();
    if (slotCount == 0) return;
    int biggestSlot = 0;
    for (int i=1;i < slotCount;i ++) {
        if (slotsSize[i] > slotsSize[biggestSlot]) {
            biggestSlot = i;
        }
    }

    Point2f direct = slots[biggestSlot];
    direct.y = -direct.y;
    Point2f rotPoint(0.5f*image.size().width, 0.5f*image.size().height);

    float angle = -atan2(direct.y, direct.x);
    angle *= 180/M_PI;
    // rotate image (note that this axis will be flipped with direct demension)
    Point2f center((image.cols-1)/2.0, (image.rows-1)/2.0);
    Mat rot = getRotationMatrix2D(center, angle, 1.0);
    Rect2f bbox = RotatedRect(Point2f(), image.size(), angle).boundingRect();
    rot.at<double>(0,2) += bbox.width/2.0 - image.cols/2.0;
    rot.at<double>(1,2) += bbox.height/2.0 - image.rows/2.0;
    warpAffine(image, image, rot, bbox.size());
    // target rotate point
    Point2f newRotPoint(0.5f*image.size().width, 0.5f*image.size().height);

    for (int i=0;i < count;i ++) {
        for (int j=(int)words[i]._vertices.size()-1;j >= 0;j --) {
            Point2f vertex(words[i]._vertices[j].x, words[i]._vertices[j].y);
            Point2f normal = vertex-rotPoint;
            Point2f rotx(direct.x, -direct.y);
            Point2f roty(direct.y, direct.x);
            words[i]._vertices[j] = Point2f(normal.dot(rotx)+newRotPoint.x, normal.dot(roty)+newRotPoint.y);
        }
        words[i]._rect = boundingRect(words[i]._vertices);
    }
}

Rect2f TextSorter::mergeRect(Rect2f rectA, Rect2f rectB)
{
    Point2f maxA(rectA.x+rectA.width, rectA.y+rectA.height);
    Point2f maxB(rectB.x+rectB.width, rectB.y+rectB.height);
    Point2f min(rectA.x < rectB.x ? rectA.x : rectB.x, rectA.y < rectB.y ? rectA.y : rectB.y);
    Point2f max(maxA.x > maxB.x ? maxA.x : maxB.x, maxA.y > maxB.y ? maxA.y : maxB.y);
    return Rect2f(min, max);
}

float TextSorter::rectDistance(Rect2f rectA, Rect2f rectB)
{
    Point2f cenA(rectA.x+0.5f*rectA.width, rectA.y+0.5f*rectA.height);
    Point2f cenB(rectB.x+0.5f*rectB.width, rectB.y+0.5f*rectB.height);
    Point2f normal = cenB-cenA;
    normal.x = fabs(normal.x);
    normal.y = fabs(normal.y);
    float totalW = rectA.width+rectB.width;
    float totalH = rectA.height+rectB.height;
    // incase overlap, distance will be overlap area
    // in other, distance will be smallest distance to get overlap
    if (normal.x <= 0.5f*totalW && normal.y <= 0.5f*totalH) {
        float overlapW = 0.5f*totalW - normal.x;
        if (overlapW > rectA.width) overlapW = rectA.width;
        if (overlapW > rectB.width) overlapW = rectB.width;
        float overlapH = 0.5f*totalH - normal.y;
        if (overlapH > rectA.height) overlapH = rectA.height;
        if (overlapH > rectB.height) overlapH = rectB.height;
        return -overlapW*overlapH;
    } else {
        float outW = normal.x - 0.5f*totalW;
        if (outW < 0) outW = 0;
        float outH = normal.y - 0.5f*totalH;
        if (outH < 0) outH = 0;
        return sqrt(outW*outW + outH*outH);
    }
}

void TextSorter::segmentWords(vector<Segment>& segments, vector<Word>& words)
{
    bool processing = false;
    for (int nearDistance = 0;nearDistance <= MAGNETIC_DISTANCE; nearDistance += 5) {
        do {
            processing = false;
            for (int i=(int)words.size()-1;i >= 0;i --) {
                int segmentCount = (int)segments.size();
                float minDis = std::numeric_limits<float>::max();
                int segmentId = 0;
                int breakDistance = -0.8f*words[i]._rect.area();
                for (int j=0;j < segmentCount;j ++) {
                    float dis = rectDistance(words[i]._rect, segments[j]._rect);
                    if (dis < minDis) {
                        minDis = dis;
                        segmentId = j;
                        if (minDis < breakDistance) {
                            break;
                        }
                    }
                }
                if (minDis < nearDistance) {
                    segments[segmentId]._rect = mergeRect(words[i]._rect, segments[segmentId]._rect);
                    segments[segmentId]._words.push_back(words[i]);
                    words.erase(words.begin()+i);
                    processing = true;
                }
            }
        } while (processing);
    }
}

void TextSorter::filterSegments(vector<Segment>& segments)
{
    bool processing = false;
    do { // since segments.size is small so I don't need to optimize this
        processing = false;
        int size = (int)segments.size();
        for (int i=0;i < size-1;i ++) {
            for (int j=i+1;j < size;j ++) {
                float iarea = segments[i]._rect.width*segments[i]._rect.height;
                float jarea = segments[j]._rect.width*segments[j]._rect.height;
                float area = iarea < jarea ? iarea : jarea;
                int smallerId = iarea < jarea ? i : j;
                int biggerId = iarea >= jarea ? i : j;
                float dis = rectDistance(segments[i]._rect, segments[j]._rect);
                if (-dis > 0.5f*area) {
                    processing = true;
                    segments[biggerId]._rect = mergeRect(segments[biggerId]._rect, segments[smallerId]._rect);
                    mergeWords(segments[biggerId]._words, segments[smallerId]._words);
                    segments.erase(segments.begin()+smallerId);
                    i = size; // break block trigger
                    break;
                }
            }
        }
    } while (processing);

    // also merge small segment to biger one
    float smallArea = 0.09f*STANDARD_DETECT_SIZE;
    smallArea *= smallArea;
    vector<Segment> smallSegments;
    for (int i=(int)segments.size()-1;i >= 0;i --) {
        if (segments[i]._rect.area() < smallArea) {
            smallSegments.push_back(segments[i]);
            segments.erase(segments.begin()+i);
        }
    }
    // this is same as way to merge words into segments
    for (int nearDistance = 0;nearDistance <= MAGNETIC_DISTANCE; nearDistance += 5) {
        do {
            processing = false;
            for (int i=(int)smallSegments.size()-1;i >= 0;i --) {
                int segmentCount = (int)segments.size();
                float minDis = std::numeric_limits<float>::max();
                int segmentId = 0;
                for (int j=0;j < segmentCount;j ++) {
                    float dis = rectDistance(smallSegments[i]._rect, segments[j]._rect);
                    if (dis < minDis) {
                        minDis = dis;
                        segmentId = j;
                    }
                }
                if (minDis < nearDistance) {
                    // horizontal merge only
                    float bias = HORIZONTAL_OVERLAP_BIAS;
                    float top = smallSegments[i]._rect.y-segments[segmentId]._rect.y;
                    float bottom = smallSegments[i]._rect.y+smallSegments[i]._rect.height
                                    -segments[segmentId]._rect.y-segments[segmentId]._rect.height;
                    if (top > -bias && bottom < bias) {
                        segments[segmentId]._rect = mergeRect(smallSegments[i]._rect, segments[segmentId]._rect);
                        mergeWords(segments[segmentId]._words, smallSegments[i]._words);
                        smallSegments.erase(smallSegments.begin()+i);
                        processing = true;
                    }
                }
            }
        } while (processing);
    }
    // merger the rest small segments
    for (int nearDistance = 0;nearDistance <= MAGNETIC_DISTANCE; nearDistance += 5) {
        do {
            processing = false;
            int size = (int)smallSegments.size();
            for (int i=size-1;i >= 1;i --) {
                float minDis = std::numeric_limits<float>::max();
                int segmentId = 0;
                for (int j=i-1;j >= 0;j --) {
                    float dis = rectDistance(smallSegments[i]._rect, smallSegments[j]._rect);
                    if (dis < minDis) {
                        minDis = dis;
                        segmentId = j;
                    }
                }
                if (minDis < nearDistance) {
                    // horizontal merge only
                    float bias = HORIZONTAL_OVERLAP_BIAS;
                    float top = smallSegments[i]._rect.y-smallSegments[segmentId]._rect.y;
                    float bottom = smallSegments[i]._rect.y+smallSegments[i]._rect.height
                                    -smallSegments[segmentId]._rect.y-smallSegments[segmentId]._rect.height;
                    if ((top > -bias && bottom < bias) ||
                        (top < bias && bottom > -bias)) // check both case since we don't check bigger box
                    {
                        smallSegments[segmentId]._rect = mergeRect(smallSegments[i]._rect, smallSegments[segmentId]._rect);
                        mergeWords(smallSegments[segmentId]._words, smallSegments[i]._words);
                        smallSegments.erase(smallSegments.begin()+i);
                        processing = true;
                        break;
                    }
                }
            }
        } while (processing);
    }

    // turn back the existing
    for (int i=(int)smallSegments.size()-1;i >= 0;i --) {
        segments.push_back(smallSegments[i]);
    }

    // remove low confidence segment
    for (int i=(int)segments.size()-1;i >= 0;i --) {
        float confidence = 0;
        for (int j=(int)segments[i]._words.size()-1;j >= 0;j --) {
            confidence += segments[i]._words[j]._confidence;
        }
        confidence /= segments[i]._words.size();
        if (confidence < LIMIT_SEGMENT_CONFIDENCE) {
            segments.erase(segments.begin()+i);
        }
    }
}

void TextSorter::mergeWords(vector<Word>& wordsA, vector<Word>& wordsB)
{
    int count = (int)wordsB.size();
    for (int i=0;i < count;i ++) {
        wordsA.push_back(wordsB[i]);
    }
}

void TextSorter::mergeSegments(vector<Segment>& segsA, vector<Segment>& segsB)
{
    int count = (int)segsB.size();
    for (int i=0;i < count;i ++) {
        segsA.push_back(segsB[i]);
    }
}

int TextSorter::sortWords(vector<Word>& words)
{
    int count = (int)words.size();
    if (count == 0) return 0;
    // * world slowest sort
    // invert array first can speed up sort
    reverse(words.begin(),words.end());
    for (int i=0;i < count-1;i ++) {
        for (int j=i+1;j < count;j ++) {
            if (words[i]._rect.y > words[j]._rect.y) {
                Word swap = words[i];
                words[i] = words[j];
                words[j] = swap;
            }
        }
    }
    vector<vector<Word>> lines;
    float lineBottom = 0;
    for (int i=0;i < count;i ++) {
        int wordHeight = words[i]._rect.height;
        int wordTop = words[i]._rect.y;
        int wordBottom = wordTop + wordHeight;
        if (wordTop + 0.5*wordHeight > lineBottom) {
            lines.push_back(vector<Word>());
            lineBottom = wordBottom;
        } else {
            int lastLineLength = (int)lines[lines.size()-1].size();
            lineBottom = (lineBottom*lastLineLength+wordBottom)/(lastLineLength+1);
        }
        lines[lines.size()-1].push_back(words[i]);
    }
    words.clear();
    int lineCount = (int)lines.size();
    for (int r=0;r < lineCount;r ++) {
        vector<Word>& row = lines[r];
        count = (int)row.size();
        for (int i=0;i < count-1;i ++) {
            for (int j=i+1;j < count;j ++) {
                if (row[i]._rect.x > row[j]._rect.x) {
                    Word swap = row[i];
                    row[i] = row[j];
                    row[j] = swap;
                }
            }
        }
        mergeWords(words, row);
    }
    return lineCount;
}

SegmentGroup TextSorter::groupSegments(vector<Segment>& segments)
{
    int count = (int)segments.size();
    if (count == 0) return SegmentGroup();
    for (int i=0;i < count-1;i ++) {
        for (int j=i+1;j < count;j ++) {
            if (segments[i]._rect.x > segments[j]._rect.x) {
                Segment swap = segments[i];
                segments[i] = segments[j];
                segments[j] = swap;
            }
        }
    }

    SegmentGroup group;
    group._columns.push_back(SegmentColumn());
    group._columns[0]._segments.push_back(segments[0]);
    Rect colBox = segments[0]._rect;
    group._columns[0]._rect = colBox;
    for (int i=1;i < count;i ++) {
        Rect nextBox = segments[i]._rect;
        if (nextBox.x+0.5f*nextBox.width > colBox.x+colBox.width) {
            colBox = nextBox;
            group._columns.push_back(SegmentColumn());
        } else {
            colBox = mergeRect(colBox, nextBox);
        }
        group._columns[group._columns.size()-1]._rect = colBox;
        group._columns[group._columns.size()-1]._segments.push_back(segments[i]);
    }

    int colCount = (int)group._columns.size();
    for (int c=0;c < colCount;c ++) {
        vector<Segment>& col = group._columns[c]._segments;
        count = (int)col.size();
        for (int i=0;i < count-1;i ++) {
            for (int j=i+1;j < count;j ++) {
                if (col[i]._rect.y > col[j]._rect.y) {
                    Segment swap = col[i];
                    col[i] = col[j];
                    col[j] = swap;
                }
            }
        }
    }
    return group;
}

void TextSorter::sortSegments(vector<Segment>& segments)
{
    int count = (int)segments.size();
    if (count == 0) return;
    Rect pageRect;
    pageRect = segments[0]._rect;
    for (int i=1;i < count;i ++) {
        pageRect = mergeRect(pageRect, segments[i]._rect);
    }
    // sort segment in y order
    for (int i=0;i < count-1;i ++) {
        for (int j=i+1;j < count;j ++) {
            if (segments[i]._rect.y > segments[j]._rect.y) {
                Segment swap = segments[i];
                segments[i] = segments[j];
                segments[j] = swap;
            }
        }
    }
    // separate segments into lines
    vector<SegmentGroup> segmentLines;
    for (int i=0;i < count;i ++) {
        if (i == count-1) {
            SegmentGroup group;
            SegmentColumn column;
            column._segments.push_back(segments[i]);
            column._rect = segments[i]._rect;
            group._columns.push_back(column);
            segmentLines.push_back(group);
            break;
        }
        Rect box = segments[i]._rect;
        int nextLineId=i+1;
        for (;nextLineId < count;nextLineId ++) {
            Rect nextBox = segments[nextLineId]._rect;
            if (nextBox.y+0.5f*nextBox.height > box.y+box.height) {
                break;
            }
        }
        vector<Segment> segmentLine;
        for (int j = i;j < nextLineId;j ++) {
            segmentLine.push_back(segments[j]);
        }
        segmentLines.push_back(groupSegments(segmentLine));
        i = nextLineId-1;
    }

    // merge segment lines
    count = (int)segmentLines.size();
    for (int i=0;i < count-1;i ++) {
        SegmentGroup line1 = segmentLines[i];
        SegmentGroup line2 = segmentLines[i+1];
        int col1Num = (int)line1._columns.size();
        int col2Num = (int)line2._columns.size();
        for (int c1=0;c1 < col1Num;c1 ++) {
            Rect2f col1Box = line1._columns[c1]._rect;
            for (int c2=0;c2 < col2Num;c2 ++) {
                Rect2f col2Box = line2._columns[c2]._rect;
                float col1Bottom = col1Box.y+col1Box.height;
                float col2Top = col2Box.y;
                if (col2Top-col1Bottom > 50) {
                    continue;
                }
                float bias = HORIZONTAL_OVERLAP_BIAS;
                float left = col1Box.x-col2Box.x;
                float right = col1Box.x+col1Box.width-col2Box.x-col2Box.width;
                if ((left > -bias && right < bias) ||
                    (left < bias && right > -bias)) // check both case since we don't check bigger box
                {
                    line1._columns[c1]._rect = mergeRect(col1Box, col2Box);
                    mergeSegments(line1._columns[c1]._segments, line2._columns[c2]._segments);
                    line2._columns.erase(line2._columns.begin()+c2);
                    c2 --;
                    col2Num --;
                }
            }
        }

        if (line2._columns.size() > 0) continue;

        // column width validate
        float colWidth = 0;
        for (int c1=0;c1 < col1Num;c1 ++) {
            colWidth += line1._columns[0]._rect.width;
        }
        colWidth /= col1Num;
        if (colWidth < 0.2f*pageRect.width) continue;

        bool mergeFail = false;
        for (int c1=0;c1 < col1Num;c1 ++) {
            if (line1._columns[c1]._rect.width < 0.9f*colWidth) {
                mergeFail = true;
                break;
            }
        }
        if (mergeFail) continue;

        // overlap column validate
        for (int c1=0;c1 < col1Num-1;c1 ++) {
            Rect rect = line1._columns[c1]._rect;
            Rect nextRect = line1._columns[c1+1]._rect;
            if (nextRect.x-rect.x-rect.width < -10) {
                mergeFail = true;
                break;
            }
        }
        if (mergeFail) continue;

        segmentLines[i] = line1;
        segmentLines.erase(segmentLines.begin()+i+1);
        i --;
        count --;
    }

    count = (int)segmentLines.size();
    segments.clear();
    for (int i=0;i < count;i ++) {
        int colCount = (int)segmentLines[i]._columns.size();
        for (int c=0;c < colCount;c ++) {
            int segCount = (int)segmentLines[i]._columns[c]._segments.size();
            for (int s=0;s < segCount;s ++) {
                segments.push_back(segmentLines[i]._columns[c]._segments[s]);
            }
        }
    }
}
