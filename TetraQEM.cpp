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

#include <iostream>
#include <fstream>

#include "Model.h"

int main()
{

    Model M("cow");
    //M.getUnionVertex();
    M.setEdgeCost();
    
    cout << "Heap Size:" << M.H.cnt << endl;

    for (int i = 0; i < 40000; i++) {
        if (i % 1000 == 0)
            cout << i << endl;
        M.collaspeMin();
    }


    vtkSmartPointer<vtkUnstructuredGrid> unGrid = M.outputVtk("cow");
    vtkNew<vtkNamedColors> colors;


    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(unGrid);
    //mapper->SetScalarRange(unstgrid->GetScalarRange());

    auto actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->RotateX(20);
    actor->RotateY(20);
    actor->RotateZ(20);
    actor->GetProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
    //Create a renderer, render window, and interactor
    auto renderer =
        vtkSmartPointer<vtkRenderer>::New();
    auto renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(640, 480);

    auto renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    //Add the actors to the scene
    renderer->AddActor(actor);
    renderer->SetBackground(colors->GetColor3d("black").GetData());

    //Render and interact
    renderWindow->Render();
    renderWindowInteractor->Start();

    



}


