// TetraQEM.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkLine.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>
#include <vtkTetra.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCamera.h>
#include <vtkUnstructuredGridQuadricDecimation.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <Eigen/dense>

#include "Model.h"

#define START_TIME startTime = chrono::system_clock::now();

#define END_TIME(str) endTime = chrono::system_clock::now();\
dur = endTime - startTime;\
second = chrono::duration<double>(dur);\
cout << str <<"Use time : " << second.count() << endl

using namespace std;
using namespace chrono;

int main()
{
    chrono::system_clock::time_point startTime = chrono::system_clock::now();
    chrono::system_clock::time_point endTime = chrono::system_clock::now();
    auto dur = endTime - startTime;
    chrono::duration<double> second(dur);

    Model M("box1m");
    
    START_TIME;
    M.selectBorder();
    M.setEdgeCost();
    END_TIME("Input");

    cout << "Edge" << M.eCnt << endl;

    START_TIME;
    M.simplification(0.22);
    END_TIME("Simplification");
    M.getErr();
    cout << Edge::keni << endl;
    cout << Edge::bukeni << endl;

    M.outputVtk("box1m_Greed_22");

}

//{
    //vtkNew<vtkUnstructuredGridReader>reader;
//
//reader->SetFileName("./input/tetBox.vtk");
//reader->Update();
//vtkUnstructuredGrid* unGrid = reader->GetOutput();
//vtkNew<vtkUnstructuredGridQuadricDecimation>dec;
//dec->SetInputData(unGrid);
//dec->SetScalarsName("U");
//dec->SetTargetReduction(0.7);
//dec->Update();

//vtkUnstructuredGrid* output = dec->GetOutput();


//vtkSmartPointer<vtkUnstructuredGridWriter> writer = vtkSmartPointer<vtkUnstructuredGridWriter>::New();
//writer->SetFileName("./outputVtk/tetBox0.3.vtk");
//writer->SetInputData(output);
//writer->Write();
//}

