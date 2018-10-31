#include<bits/stdc++.h>
#include "bitmap_image.hpp"

using namespace std;


class Point{
public:
    double x,y,z;
};

class Triangle{
public:
    Point points[3];
    int color[3];
};

int screenWidth, screenHeight;
double xLeftLimit, xRightLimit, yBottomLimit, yTopLimit, frontLimitOfZ, rearLimitOfZ;

vector<Triangle> triangles;


void readData() {

    ifstream input,input2;
    input.open("config.txt");

    input >> screenWidth >> screenHeight;
    input >> xLeftLimit;
    input >> yBottomLimit;
    input >> frontLimitOfZ >> rearLimitOfZ;

    input.close();

    xRightLimit = xLeftLimit * (-1);
    yTopLimit = yBottomLimit * (-1);

    Triangle triangle;

    input2.open("stage3.txt");

    while (!input2.eof()) {
        for (int i=0; i<3; i++) {
            input2 >> triangle.points[i].x >> triangle.points[i].y >> triangle.points[i].z;
            triangle.color[i] = rand()%256;
        }
        triangles.push_back(triangle);
    }

    input2.close();


}

double dx, dy, topY, leftX, zMax;
double** zBuffer;
Point** frameBuffer;


void initializeZBufferAndFrameBuffer() {

    dx = 2 * abs(xLeftLimit) / screenWidth;
    dy = 2 * abs(yBottomLimit) / screenHeight;

    topY = yTopLimit - dy / 2;
    leftX = xLeftLimit + dx / 2;

    zMax = abs(rearLimitOfZ - frontLimitOfZ);

    zBuffer = new double* [screenWidth];
    frameBuffer = new Point* [screenWidth];


    for (int i=0; i<screenWidth; i++) {
        zBuffer[i] = new double[screenHeight];
        frameBuffer[i] = new Point[screenHeight];
    }

    for (int i=0; i<screenWidth; i++) {
        for (int j=0; j<screenHeight; j++) {
            zBuffer[i][j] = zMax;
            frameBuffer[i][j].x = 0;
            frameBuffer[i][j].y = 0;
            frameBuffer[i][j].z = 0;
        }
    }

}



Point Intersection(int idx,int a, int b, double row) {

    Point point;
    Point start = triangles[idx].points[a];
    Point end = triangles[idx].points[b];
    point.y = row;

    double t = (point.y - start.y)/(end.y - start.y);

    point.x = start.x + t * (end.x - start.x);
    point.z = start.z + t * (end.z - start.z);

    return point;

}


void applyProcedure() {

    for (int i=0; i<triangles.size(); i++) {

        Triangle triangle = triangles[i];


        double yMax = triangle.points[0].y;
        for (int i=1; i<3; i++) {
            if (yMax < triangle.points[i].y) {
                yMax = triangle.points[i].y;
            }
        }

        double yMin = triangle.points[0].y;
        for (int i=1; i<3; i++) {
            if (yMin > triangle.points[i].y) {
                yMin = triangle.points[i].y;
            }
        }


        double xMax = triangle.points[0].x;
        for (int i=1; i<3; i++) {
            if (xMax < triangle.points[i].x) {
                xMax = triangle.points[i].x;
            }
        }

        double xMin = triangle.points[0].x;
        for (int i=1; i<3; i++) {
            if (xMin > triangle.points[i].x) {
                xMin = triangle.points[i].x;
            }
        }

        double topScanline = yMax<yTopLimit? yMax : yTopLimit;
        double bottomScanline = yMin>yBottomLimit? yMin : yBottomLimit;

        double colLeft = xMin>xLeftLimit? xMin : xLeftLimit;
        double colRight = xMax<xRightLimit? xMax : xRightLimit;

        int row = round((topY - topScanline)/dy);
        int rowEnd = round((topY - bottomScanline)/dy);


        for (; row<rowEnd; row++) {
            Point a = Intersection(i,0, 1, topY - row*dy);
            Point b = Intersection(i,1, 2, topY - row*dy);
            Point c = Intersection(i,0, 2, topY - row*dy);

            Point leftPoint, rightPoint;


            if ((a.x >= xMin && a.x <= xMax) && (b.x >= xMin && b.x <= xMax)) {
                leftPoint = a;
                rightPoint = b;
            }
            else if ((b.x >= xMin && b.x <= xMax) && (c.x >= xMin && c.x <= xMax)) {
                leftPoint = b;
                rightPoint = c;
            }
            else if ((c.x >= xMin && c.x <= xMax) && (a.x >= xMin && a.x <= xMax)) {
                leftPoint = c;
                rightPoint = a;
            }

            if (leftPoint.x > rightPoint.x) {
                Point temp = leftPoint;
                leftPoint = rightPoint;
                rightPoint = temp;
            }

            double leftcolumn = leftPoint.x>colLeft? leftPoint.x : colLeft;
            double rightcolumn = rightPoint.x<colRight? rightPoint.x : colRight;


            int col = floor((leftcolumn - leftX)/dx);
            int colEnd = floor((rightcolumn - leftX)/dx);

            for (; col<colEnd; col++) {

                double t = (leftX + col*dx - leftPoint.x);

                if (rightPoint.x - leftPoint.x){
                    t /= rightPoint.x - leftPoint.x;
                }

                double z = leftPoint.z + t * (rightPoint.z - leftPoint.z);

                if (z > frontLimitOfZ && z <= zBuffer[row][col]) {
                    zBuffer[row][col] = z;

                    frameBuffer[row][col].x = triangle.color[0];
                    frameBuffer[row][col].y = triangle.color[1];
                    frameBuffer[row][col].z = triangle.color[2];
                }
            }

        }
    }

}


void save() {


    bitmap_image image(screenWidth, screenHeight);

    for (int i=0; i<screenWidth; i++) {
        for (int j=0; j<screenHeight; j++) {
            image.set_pixel(j, i, frameBuffer[i][j].x, frameBuffer[i][j].y, frameBuffer[i][j].z);
        }
    }

    image.save_image("output.bmp");

    ofstream output;
    output.open("z_buffer.txt");
    output.precision(6);

    for (int i=0; i<screenWidth; i++) {
        bool flag = false;
        for (int j=0; j<screenHeight; j++) {
            if (zBuffer[i][j] < zMax) {
                output<<fixed<<zBuffer[i][j]<<"\t";
                flag = true;
            }
        }
        if (flag) {
            output<<endl;
        }
    }

    output.close();


}

void freeMemory() {

    triangles.erase(triangles.begin(),triangles.end());
    delete[] frameBuffer;
    delete[] zBuffer;
}


int main() {
    readData();
    initializeZBufferAndFrameBuffer();
    applyProcedure();
    save();
    freeMemory();
}


