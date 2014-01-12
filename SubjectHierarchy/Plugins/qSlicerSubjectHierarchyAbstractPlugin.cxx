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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "vtkSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin::qSlicerSubjectHierarchyAbstractPlugin(QObject *parent)
  : Superclass(parent)
  , m_Name(QString())
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin::~qSlicerSubjectHierarchyAbstractPlugin()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::name()const
{
  if (m_Name.isEmpty())
  {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::name: Empty plugin name!";
  }
  return this->m_Name;
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyAbstractPlugin::dependencies()const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::exportNode(vtkMRMLSubjectHierarchyNode* node)
{
  qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::exportNode: This plugin ("
    << this->m_Name << ") does not support exporting!";
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::nodeContextMenuActions()const
{
  return QList<QAction*>();
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::sceneContextMenuActions()const
{
  return QList<QAction*>();
}

//----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy(vtkMRMLNode* nodeToAdd, vtkMRMLSubjectHierarchyNode* parentNode)
{
  if (!nodeToAdd || !parentNode)
  {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy: Invalid node to add or parent node!";
    return false;
  }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
  {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy: Invalid MRML scene!";
    return false;
  }

  // Associate to a new hierarchy node and put it in the tree under the parent
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, parentNode, this->childLevel(parentNode->GetLevel()).toLatin1().constData(), nodeToAdd->GetName(), nodeToAdd);

  return true;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::reparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* nodeToReparent, vtkMRMLSubjectHierarchyNode* parentNode)
{
  nodeToReparent->SetParentNodeID(parentNode->GetID());
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setTooltip(vtkMRMLSubjectHierarchyNode* node, QStandardItem* item)
{
  if (!node)
  {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::setTooltip: Subject hierarchy node is NULL!";
    item->setToolTip("Invalid!");
  }

  // Display node type and level in the tooltip
  QString tooltipString("");
  vtkMRMLNode* associatedNode = node->GetAssociatedDataNode();
  if (associatedNode)
  {
    tooltipString.append(associatedNode->GetNodeTagName());
    tooltipString.append(" (");
  }

  tooltipString.append("Level:");
  tooltipString.append(node->GetLevel());
  tooltipString.append(" Plugin:");
  tooltipString.append(node->GetOwnerPluginName() ? node->GetOwnerPluginName() : "None");

  if (associatedNode)
  {
    tooltipString.append(")");
  }

  item->setToolTip(tooltipString);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible)
{
  // Default behavior is to call SetDisplayVisibility on all displayable
  // associated nodes in the whole branch
  node->SetDisplayVisibilityForBranch(visible);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyAbstractPlugin::getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)
{
  return node->GetDisplayVisibilityForBranch();
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::childLevel(QString parentLevel)
{
  // Get child level from this plugin
  QString childLevel;
  if (this->m_ChildLevelMap.contains(parentLevel))
  {
    childLevel = this->m_ChildLevelMap[parentLevel];
  }
  // If this plugin does not have child level for this parent level, then log a warning
  else
  {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::childLevel: Could not get child level for level '"
      << parentLevel << "'!";
    return QString();
  }

  return childLevel;
}

//--------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyAbstractPlugin::createChildNode(vtkMRMLSubjectHierarchyNode* parentNode, QString nodeName, vtkMRMLNode* associatedNode/*=NULL*/)
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();

  // If there is current node, parent level will be an empty string, which means the scene
  QString parentLevel;
  if (parentNode)
  {
    parentLevel = QString(parentNode->GetLevel());
  }
  QString childLevel = this->childLevel(parentLevel);

  // Create child subject hierarchy node
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, parentNode, childLevel.toLatin1().constData(), nodeName.toLatin1().constData(), associatedNode);

  emit requestExpandNode(childSubjectHierarchyNode);

  return childSubjectHierarchyNode;
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::createChildForCurrentNode()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();

  // If there is current node, parent level will be an empty string, which means the scene
  QString parentLevel;
  if (currentNode)
  {
    parentLevel = QString(currentNode->GetLevel());
  }
  QString childLevel = this->childLevel(parentLevel);

  // Create child subject hierarchy node
  std::string childNodeName = vtkSubjectHierarchyConstants::SUBJECTHIERARCHY_NEW_NODE_NAME_PREFIX + childLevel.toLatin1().constData();
  this->createChildNode(currentNode, childNodeName.c_str());
}