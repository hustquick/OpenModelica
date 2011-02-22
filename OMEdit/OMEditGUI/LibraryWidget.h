/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linkoping University,
 * Department of Computer and Information Science,
 * SE-58183 Linkoping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linkoping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 * Main Authors 2010: Syed Adeel Asghar, Sonia Tariq
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linkoping University
 * Main Authors 2009-2010:  Robert Braun, Bjorn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <string>
#include <map>
#include <QListWidget>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QStringList>

#include "mainwindow.h"
#include "StringHandler.h"

class MainWindow;
class Component;
class OMCProxy;
class LibraryWidget;
class ModelicaTree;
class LibraryComponent;

class ModelicaTreeNode : public QTreeWidgetItem
{
public:
    ModelicaTreeNode(QString text, QString parentName, QString tooltip, int type, QTreeWidget *parent = 0);
    static QIcon getModelicaNodeIcon(int type);

    int mType;
    QString mName;
    QString mParentName;
    QString mNameStructure;
};

class ModelicaTree : public QTreeWidget
{
    Q_OBJECT
public:
    ModelicaTree(LibraryWidget *parent = 0);
    ~ModelicaTree();
    void createActions();
    ModelicaTreeNode* getNode(QString name);
    void deleteNode(ModelicaTreeNode *item);
    void removeChildNodes(ModelicaTreeNode *item);

    LibraryWidget *mpParentLibraryWidget;
private:
    QList<ModelicaTreeNode*> mModelicaTreeNodesList;
    QAction *mpRenameAction;
    QAction *mpDeleteAction;
    QAction *mpCheckModelAction;
signals:
    void nodeDeleted();
public slots:
    void addNode(QString name, int type, QString parentName=QString(), QString parentStructure=QString());
    void openProjectTab(QTreeWidgetItem *item, int column);
    void showContextMenu(QPoint point);
    void renameClass();
    void checkModelicaModel();
    bool deleteNodeTriggered(ModelicaTreeNode *node = 0, bool askQuestion = true);
    void saveChildModels(QString modelName, QString filePath);
};

class LibraryTreeNode : public QTreeWidgetItem
{
public:
    LibraryTreeNode(QString text, QString parentName, QString tooltip, QTreeWidget *parent = 0);

    int mType;
    QString mName;
    QString mParentName;
    QString mNameStructure;
};

class LibraryTree : public QTreeWidget
{
    Q_OBJECT
private:
    QList<QString> mTreeList;

    QAction *mpShowComponentAction;
    QAction *mpViewDocumentationAction;
    QAction *mpCheckModelAction;
public:
    LibraryTree(LibraryWidget *pParent = 0);
    ~LibraryTree();
    void createActions();
    void addModelicaStandardLibrary();
    void loadModelicaLibraryHierarchy(QString value, QString prefixStr=QString());
    void addClass(QList<LibraryTreeNode*> *tempPackageNodesList, QList<LibraryTreeNode*> *tempNonPackageNodesList,
                  QString className, QString parentClassName=QString(), QString parentStructure=QString(),
                  bool hasIcon=false);
    void addNodes(QList<LibraryTreeNode*> nodes);
    bool isTreeItemLoaded(QTreeWidgetItem *item);
    static bool sortNodesAscending(const LibraryTreeNode *node1, const LibraryTreeNode *node2);

    LibraryWidget *mpParentLibraryWidget;
private slots:
    void expandLib(QTreeWidgetItem *item);
    void showContextMenu(QPoint point);
    void showComponent(QTreeWidgetItem *item, int column);
    void showComponent();
    void viewDocumentation();
    void checkLibraryModel();
    void treeItemPressed(QTreeWidgetItem *item);
public slots:
    void loadingLibraryComponent(LibraryTreeNode *treeNode, QString className);
};

class MSLSuggestCompletion;
class SearchMSLWidget;

class MSLSearchBox : public QLineEdit
{
public:
    MSLSearchBox(SearchMSLWidget *pParent = 0);

    SearchMSLWidget *mpSearchMSLWidget;
    MSLSuggestCompletion *mpMSLSuggestionCompletion;
    QString mDefaultText;
protected:
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
};

class MSLSuggestCompletion : QObject
{
    Q_OBJECT
public:
    MSLSuggestCompletion(MSLSearchBox *pParent = 0);
    ~MSLSuggestCompletion();
    bool eventFilter(QObject *pObject, QEvent *event);
    void showCompletion(const QStringList &choices);
    QTimer* getTimer();
public slots:
    void doneCompletion();
    void preventSuggestions();
    void getSuggestions();
private:
    MSLSearchBox *mpMSLSearchBox;
    QTreeWidget *mpPopup;
    QTimer *mpTimer;
};

class SearchMSLWidget : public QWidget
{
    Q_OBJECT
public:
    SearchMSLWidget(MainWindow *pParent = 0);
    QStringList getMSLItemsList();

    MSLSearchBox* getMSLSearchTextBox();
    MainWindow *mpParentMainWindow;
private:
    MSLSearchBox *mpSearchTextBox;
    QPushButton *mpSearchButton;
    LibraryTree *mpSearchedItemsTree;
    QStringList mMSLItemsList;

public slots:
    void searchMSL();
};

class LibraryWidget : public QWidget
{
    Q_OBJECT
public:
    LibraryTree *mpLibraryTree;
    ModelicaTree *mpModelicaTree;
    QTabWidget *mpLibraryTabs;
    //Member functions
    LibraryWidget(MainWindow *parent = 0);
    ~LibraryWidget();
    void addModelicaNode(QString name, int type, QString parentName=QString(), QString parentStructure=QString());
    void addModelFiles(QString fileName, QString parentFileName=QString(), QString parentStructure=QString());
    void loadModel(QString path, QStringList modelsList);
    void addComponentObject(LibraryComponent *libraryComponent);
    Component* getComponentObject(QString className);
    LibraryComponent* getLibraryComponentObject(QString className);
    void updateNodeText(QString text, QString textStructure, ModelicaTreeNode *node = 0);

    MainWindow *mpParentMainWindow;
    ModelicaTreeNode *mSelectedModelicaNode;
    QTreeWidgetItem *mSelectedLibraryNode;
signals:
    void addModelicaTreeNode(QString name, int type, QString parentName=QString(), QString parentStructure=QString());
private:
    //Member variables
    QVBoxLayout *mpGrid;
    QList<LibraryComponent*> mComponentsList;
};

class LibraryComponent
{
public:
    LibraryComponent(QString value, QString className, OMCProxy *omc);
    ~LibraryComponent();
    void generateSvg(QPainter *painter, Component *pComponent);
    QPixmap getComponentPixmap(QSize size);

    QString mClassName;
    Component *mpComponent;
    QGraphicsView *mpGraphicsView;
    QRectF mRectangle;
};

class LibraryLoader : public QThread
{
    Q_OBJECT
public:
    LibraryLoader(LibraryTreeNode *treeNode, QString className, LibraryTree *pParent = 0);

    LibraryTree *mpParentLibraryTree;
    LibraryTreeNode *mTreeNode;
    QString mClassName;
protected:
    void run();
signals:
    void loadLibraryComponent(LibraryTreeNode *treeNode, QString className);
};

#endif // LIBRARYWIDGET_H
