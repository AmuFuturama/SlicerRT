/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "SlicerRtCommon.h"
#include "vtkConvertContourRepresentations.h"
#include "vtkSlicerContoursModuleLogic.h"
#include <vtkCollection.h>
#include <vtkMRMLContourNode.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkPolyData.h>
#include <vtksys/SystemTools.hxx>

// ITK includes
#if ITK_VERSION_MAJOR > 3
  #include "itkFactoryRegistration.h"
#endif

bool CheckIfResultIsWithinOneTenthPercentFromBaseline(double result, double baseline);

//-----------------------------------------------------------------------------
int vtkSlicerContoursModuleLogicTestConversions ( int argc, char * argv[] )
{
  int argIndex = 1;
  std::ostream& outputStream = std::cout;
  std::ostream& errorStream = std::cerr;

  // TestSceneFile
  const char *testSceneFileName  = NULL;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-TestSceneFile") == 0)
    {
      testSceneFileName = argv[argIndex+1];
      outputStream << "Test MRML scene file name: " << testSceneFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      testSceneFileName = "";
    }
  }
  else
  {
    errorStream << "Invalid arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  const char *temporarySceneFileName = NULL;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-TemporarySceneFile") == 0)
    {
      temporarySceneFileName = argv[argIndex+1];
      outputStream << "Temporary scene file name: " << temporarySceneFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      temporarySceneFileName = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  // Labelmap metrics
  int nonZeroVoxelCount(-1);
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-NonZeroVoxelCount") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected non-zero voxel count: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> nonZeroVoxelCount;
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  // Labelmap extents
  int expectedExtents[6];
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-LabelMapXMinExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected LabelMapXMinExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedExtents[0];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-LabelMapXMaxExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected LabelMapXMaxExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedExtents[1];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-LabelMapYMinExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected LabelMapYMinExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedExtents[2];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-LabelMapYMaxExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected LabelMapYMaxExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedExtents[3];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-LabelMapZMinExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected LabelMapZMinExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedExtents[4];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-LabelMapZMaxExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected LabelMapZMaxExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedExtents[5];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  // Closed surface metrics
  int expectedNumberOfPoints;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ExpectedNumberOfPoints") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected number of points: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedNumberOfPoints;
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  int expectedNumberOfCells;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ExpectedNumberOfCells") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected number of cells: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedNumberOfCells;
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  int expectedNumberOfPolys;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ExpectedNumberOfPolys") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected number of cells: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedNumberOfPolys;
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  // Closed surface bounds
  double expectedBounds[6];
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ClosedSurfaceXMinExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected ClosedSurfaceXMinExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedBounds[0];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ClosedSurfaceXMaxExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected ClosedSurfaceXMaxExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedBounds[1];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ClosedSurfaceYMinExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected ClosedSurfaceYMinExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedBounds[2];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ClosedSurfaceYMaxExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected ClosedSurfaceYMaxExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedBounds[3];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ClosedSurfaceZMinExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected ClosedSurfaceZMinExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedBounds[4];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-ClosedSurfaceZMaxExtent") == 0)
    {
      char* arg = argv[argIndex+1];
      outputStream << "Expected ClosedSurfaceZMaxExtent: " << arg << std::endl;
      argIndex += 2;
      std::stringstream ss;
      ss << arg;
      ss >> expectedBounds[5];
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  // Make sure NRRD reading works
#if ITK_VERSION_MAJOR > 3
  itk::itkFactoryRegistration();
#endif

  // Create scene
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkSmartPointer<vtkSlicerContoursModuleLogic> logic = vtkSmartPointer<vtkSlicerContoursModuleLogic>::New();
  logic->SetMRMLScene(mrmlScene);

  // Load test scene into temporary scene
  mrmlScene->SetURL(testSceneFileName);
  mrmlScene->Import();

  vtksys::SystemTools::RemoveFile(temporarySceneFileName);
  mrmlScene->SetRootDirectory( vtksys::SystemTools::GetParentDirectory(temporarySceneFileName).c_str() );
  mrmlScene->SetURL(temporarySceneFileName);
  mrmlScene->Commit();

  // Get CT volume
  vtkSmartPointer<vtkCollection> doseVolumeNodes = 
    vtkSmartPointer<vtkCollection>::Take( mrmlScene->GetNodesByName("Dose") );
  if (doseVolumeNodes->GetNumberOfItems() != 1)
  {
    mrmlScene->Commit();
    errorStream << "ERROR: Failed to get CT volume!" << std::endl;
    return EXIT_FAILURE;
  }
  vtkMRMLScalarVolumeNode* doseScalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(doseVolumeNodes->GetItemAsObject(0));
  doseScalarVolumeNode->GetImageData()->SetWholeExtent(doseScalarVolumeNode->GetImageData()->GetExtent());

  // Get the body contour  
  vtkMRMLContourNode* bodyContourNode = vtkMRMLContourNode::SafeDownCast(mrmlScene->GetNodeByID("vtkMRMLContourNode1"));
  if (bodyContourNode == NULL)
  {
    mrmlScene->Commit();
    errorStream << "ERROR: Failed to get body contour!" << std::endl;
    return EXIT_FAILURE;
  }

  bodyContourNode->SetAndObserveRasterizationReferenceVolumeNodeId(doseScalarVolumeNode->GetID());
  bodyContourNode->SetRasterizationOversamplingFactor(2.0);

  vtkMRMLScalarVolumeNode* indexedLabelmapNode(NULL);
  {
    vtkSmartPointer<vtkConvertContourRepresentations> converter = vtkSmartPointer<vtkConvertContourRepresentations>::New();
    converter->SetContourNode(bodyContourNode);
    converter->ReconvertRepresentation(vtkMRMLContourNode::IndexedLabelmap);

    indexedLabelmapNode = bodyContourNode->GetIndexedLabelmapVolumeNode();
  }

  vtkImageData* image = indexedLabelmapNode->GetImageData();
  int extents[6];
  image->GetExtent(extents);

  if( extents[0] != expectedExtents[0] || 
    extents[1] != expectedExtents[1] || 
    extents[2] != expectedExtents[2] || 
    extents[3] != expectedExtents[3] || 
    extents[4] != expectedExtents[4] || 
    extents[5] != expectedExtents[5] )
  {
    errorStream << "Extents don't match." << std::endl;
    errorStream << "extents[0]: " << extents[0] << std::endl;
    errorStream << "extents[1]: " << extents[1] << std::endl;
    errorStream << "extents[2]: " << extents[2] << std::endl;
    errorStream << "extents[3]: " << extents[3] << std::endl;
    errorStream << "extents[4]: " << extents[4] << std::endl;
    errorStream << "extents[5]: " << extents[5] << std::endl;
    return EXIT_FAILURE;
  }

  int voxelCount(0);
  for (int z = extents[4]; z < extents[5]; z++)
  {
    for (int y = extents[2]; y < extents[3]; y++)
    {
      for (int x = extents[0]; x < extents[1]; x++)
      {
        unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x,y,z));
        if( *pixel != 0 )
        {
          voxelCount++;
        }
      }
    }
  }

  if( voxelCount != nonZeroVoxelCount )
  {
    errorStream << "Non-zero voxel count does not match expected result. Got: " << voxelCount << ". Expected: " << nonZeroVoxelCount << std::endl;
    return EXIT_FAILURE;
  }

  vtkMRMLModelNode* closedSurfaceModelNode(NULL);
  {
    // Set closed surface model conversion parameters
    bodyContourNode->SetDecimationTargetReductionFactor(0.0);

    // Delete current existing representation and re-convert
    vtkSmartPointer<vtkConvertContourRepresentations> converter = vtkSmartPointer<vtkConvertContourRepresentations>::New();
    converter->SetContourNode(bodyContourNode);
    converter->ReconvertRepresentation(vtkMRMLContourNode::ClosedSurfaceModel);

    closedSurfaceModelNode = bodyContourNode->GetClosedSurfaceModelNode();
  }
  double bounds[6];
  closedSurfaceModelNode->GetRASBounds(bounds);

  if( !CheckIfResultIsWithinOneTenthPercentFromBaseline(bounds[0], expectedBounds[0]) || 
    !CheckIfResultIsWithinOneTenthPercentFromBaseline(bounds[1], expectedBounds[1]) || 
    !CheckIfResultIsWithinOneTenthPercentFromBaseline(bounds[2], expectedBounds[2]) || 
    !CheckIfResultIsWithinOneTenthPercentFromBaseline(bounds[3], expectedBounds[3]) || 
    !CheckIfResultIsWithinOneTenthPercentFromBaseline(bounds[4], expectedBounds[4]) || 
    !CheckIfResultIsWithinOneTenthPercentFromBaseline(bounds[5], expectedBounds[5]) )
  {
    errorStream << "Closed surface bounds don't match." << std::endl;
    errorStream << "bounds[0]: " << bounds[0] << std::endl;
    errorStream << "bounds[1]: " << bounds[1] << std::endl;
    errorStream << "bounds[2]: " << bounds[2] << std::endl;
    errorStream << "bounds[3]: " << bounds[3] << std::endl;
    errorStream << "bounds[4]: " << bounds[4] << std::endl;
    errorStream << "bounds[5]: " << bounds[5] << std::endl;
    return EXIT_FAILURE;
  }

  if(closedSurfaceModelNode->GetPolyData()->GetNumberOfPoints() != expectedNumberOfPoints)
  {
    errorStream << "Number of points mismatch in closed surface model. Expected: " << expectedNumberOfPoints << ". Got: " << closedSurfaceModelNode->GetPolyData()->GetNumberOfPoints() << std::endl;
    return EXIT_FAILURE;
  }

  if(closedSurfaceModelNode->GetPolyData()->GetNumberOfCells() != expectedNumberOfCells)
  {
    errorStream << "Number of cells mismatch in closed surface model. Expected: " << expectedNumberOfCells << ". Got: " << closedSurfaceModelNode->GetPolyData()->GetNumberOfCells() << std::endl;
    return EXIT_FAILURE;
  }

  if(closedSurfaceModelNode->GetPolyData()->GetNumberOfPolys() != expectedNumberOfPolys)
  {
    errorStream << "Number of polys mismatch in closed surface model. Expected: " << expectedNumberOfPolys << ". Got: " << closedSurfaceModelNode->GetPolyData()->GetNumberOfPolys() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
bool CheckIfResultIsWithinOneTenthPercentFromBaseline(double result, double baseline)
{
  if (baseline == 0.0)
  {
    return (fabs(result - baseline) < 0.0001);
  }

  double ratio = result / baseline;
  double absoluteDifferencePercent = fabs(ratio - 1.0) * 100.0;

  return absoluteDifferencePercent < 0.1;
}