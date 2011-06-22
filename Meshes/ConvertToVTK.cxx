// ITK
#include "itkLineCell.h"
#include "itkMesh.h"
#include "itkTriangleCell.h"
#include "itkQuadrilateralCell.h"

// VTK
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

typedef itk::Mesh< float, 3 >   MeshType;

// Functions
MeshType::Pointer CreateMeshWithEdges();
void ConvertMeshToUnstructuredGrid(MeshType::Pointer, vtkUnstructuredGrid*);


class VistVTKCellsClass
{
  vtkCellArray* m_Cells;
  int* m_LastCell;
  int* m_TypeArray;
public:
  // typedef the itk cells we are interested in
  typedef itk::CellInterface<
                      MeshType::PixelType,
                      MeshType::CellTraits >  CellInterfaceType;

  typedef itk::TriangleCell<CellInterfaceType>      floatTriangleCell;
  typedef itk::QuadrilateralCell<CellInterfaceType> floatQuadrilateralCell;

  // Set the vtkCellArray that will be constructed
  void SetCellArray(vtkCellArray* a)
    {
      m_Cells = a;
    }
  // Set the cell counter pointer
  void SetCellCounter(int* i)
    {
      m_LastCell = i;
    }
  // Set the type array for storing the vtk cell types
  void SetTypeArray(int* i)
    {
      m_TypeArray = i;
    }
  // Visit a triangle and create the VTK_TRIANGLE cell
  void Visit(unsigned long, floatTriangleCell* t)
    {
      m_Cells->InsertNextCell(3,  (vtkIdType*)t->PointIdsBegin());
      m_TypeArray[*m_LastCell] = VTK_TRIANGLE;
      (*m_LastCell)++;
    }
  // Visit a triangle and create the VTK_QUAD cell
  void Visit(unsigned long, floatQuadrilateralCell* t)
    {
      m_Cells->InsertNextCell(4,  (vtkIdType*)t->PointIdsBegin());
      m_TypeArray[*m_LastCell] = VTK_QUAD;
      (*m_LastCell)++;
    }
};


int main(int, char *[])
{
  MeshType::Pointer mesh = CreateMeshWithEdges();
  
  vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  ConvertMeshToUnstructuredGrid(mesh, unstructuredGrid);

  // Write file
  vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
    vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  writer->SetFileName("output.vtu");
  writer->SetInputConnection(unstructuredGrid->GetProducerPort());
  writer->Write();

  return EXIT_SUCCESS;
}



MeshType::Pointer CreateMeshWithEdges()
{

  MeshType::Pointer  mesh = MeshType::New();

  // Create points
  MeshType::PointType p0,p1,p2,p3;

  p0[0]= -1.0; p0[1]= -1.0; p0[2]= 0.0; // first  point ( -1, -1, 0 )
  p1[0]=  1.0; p1[1]= -1.0; p1[2]= 0.0; // second point (  1, -1, 0 )
  p2[0]=  1.0; p2[1]=  1.0; p2[2]= 0.0; // third  point (  1,  1, 0 )
  p3[0]=  1.0; p3[1]=  1.0; p3[2]= 1.0; // third  point (  1,  1, 1 )

  mesh->SetPoint( 0, p0 );
  mesh->SetPoint( 1, p1 );
  mesh->SetPoint( 2, p2 );
  mesh->SetPoint( 3, p3 );

  std::cout << "Points = " << mesh->GetNumberOfPoints() << std::endl;

  //access points
  typedef MeshType::PointsContainer::Iterator     PointsIterator;

  PointsIterator  pointIterator = mesh->GetPoints()->Begin();

  PointsIterator end = mesh->GetPoints()->End();
  while( pointIterator != end )
    {
    MeshType::PointType p = pointIterator.Value();  // access the point
    std::cout << p << std::endl;                    // print the point
    ++pointIterator;                                // advance to next point
    }

  typedef MeshType::CellType::CellAutoPointer         CellAutoPointer;
  typedef itk::LineCell< MeshType::CellType >         LineType;

  
  CellAutoPointer line0;
  line0.TakeOwnership(  new LineType  );
  line0->SetPointId(0, 0); // line between points 0 and 1
  line0->SetPointId(1, 1);
  mesh->SetCell( 0, line0);

  CellAutoPointer line1;
  line1.TakeOwnership(  new LineType  );
  line1->SetPointId(0, 1); // line between points 1 and 2
  line1->SetPointId(1, 2);
  mesh->SetCell( 1, line1);

  CellAutoPointer line2;
  line2.TakeOwnership(  new LineType  );
  line2->SetPointId(0, 2); // line between points 2 and 3
  line2->SetPointId(1, 3);
  mesh->SetCell( 2, line2);
  
  /*
  typedef MeshType::CellsContainer::Iterator  CellIterator;
  CellIterator  cellIterator = mesh->GetCells()->Begin();
  CellIterator  CellsEnd          = mesh->GetCells()->End();

  while( cellIterator != CellsEnd )
    {
    MeshType::CellType * cellptr = cellIterator.Value();
    LineType * line = dynamic_cast<LineType *>( cellptr );

    long unsigned int* linePoint0 = line->PointIdsBegin();
    //long unsigned int* linePoint1 = line->PointIdsEnd();
    long unsigned int* linePoint1 = linePoint0+1;
    std::cout << "line first point id: " << *linePoint0 << std::endl;
    std::cout << "line second point id: " << *linePoint1 << std::endl;

    ++cellIterator;
    }
  */
  return mesh;
}

void ConvertMeshToUnstructuredGrid(MeshType::Pointer mesh, vtkUnstructuredGrid* unstructuredGrid)
{
  // Get the number of points in the mesh
  int numPoints = mesh->GetNumberOfPoints();
  if(numPoints == 0)
    {
    mesh->Print(std::cerr);
    std::cerr << "no points in Grid " << std::endl;
    exit(-1);
    }

  // Create the vtkPoints object and set the number of points
  vtkPoints* vpoints = vtkPoints::New();
  vpoints->SetNumberOfPoints(numPoints);
  // iterate over all the points in the itk mesh filling in
  // the vtkPoints object as we go
  MeshType::PointsContainer::Pointer points = mesh->GetPoints();
  for(MeshType::PointsContainer::Iterator i = points->Begin();
      i != points->End(); ++i)
    {
    // Get the point index from the point container iterator
    int idx = i->Index();
    // Set the vtk point at the index with the the coord array from itk
    // itk returns a const pointer, but vtk is not const correct, so
    // we have to use a const cast to get rid of the const
    vpoints->SetPoint(idx, const_cast<float*>(i->Value().GetDataPointer()));
    }
  // Set the points on the vtk grid
  unstructuredGrid->SetPoints(vpoints);

  // Now create the cells using the MulitVisitor
  // 1. Create a MultiVisitor
  MeshType::CellType::MultiVisitor::Pointer mv =
    MeshType::CellType::MultiVisitor::New();
  // 2. Create a triangle and quadrilateral visitor
  typedef itk::CellInterfaceVisitorImplementation<
    float, MeshType::CellTraits,
    itk::TriangleCell< itk::CellInterface<MeshType::PixelType, MeshType::CellTraits > >,
    VistVTKCellsClass> TriangleVisitor;

  typedef itk::CellInterfaceVisitorImplementation<
    float, MeshType::CellTraits,
    itk::QuadrilateralCell< itk::CellInterface<MeshType::PixelType, MeshType::CellTraits > >,
    VistVTKCellsClass> QuadrilateralVisitor;

  TriangleVisitor::Pointer tv = TriangleVisitor::New();
  QuadrilateralVisitor::Pointer qv =  QuadrilateralVisitor::New();
  // 3. Set up the visitors
  int vtkCellCount = 0; // running counter for current cell being inserted into vtk
  int numCells = mesh->GetNumberOfCells();
  int *types = new int[numCells]; // type array for vtk
  // create vtk cells and estimate the size
  vtkCellArray* cells = vtkCellArray::New();
  cells->EstimateSize(numCells, 4);
  // Set the TypeArray CellCount and CellArray for both visitors
  tv->SetTypeArray(types);
  tv->SetCellCounter(&vtkCellCount);
  tv->SetCellArray(cells);
  qv->SetTypeArray(types);
  qv->SetCellCounter(&vtkCellCount);
  qv->SetCellArray(cells);
  // add the visitors to the multivisitor
  mv->AddVisitor(tv);
  mv->AddVisitor(qv);
  // Now ask the mesh to accept the multivisitor which
  // will Call Visit for each cell in the mesh that matches the
  // cell types of the visitors added to the MultiVisitor
  mesh->Accept(mv);

  // Now set the cells on the vtk grid with the type array and cell array
  unstructuredGrid->SetCells(types, cells);
  std::cout << "Unstructured grid has " << unstructuredGrid->GetNumberOfCells() << " cells." << std::endl;

  // Clean up vtk objects (no vtkSmartPointer ... )
  cells->Delete();
  vpoints->Delete();

}