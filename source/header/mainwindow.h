/****************************************************************************
**
** Copyright (C) 2016
**
** This file is generated by the Magus toolkit
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QToolBar>
#include <QList>
#include "OgreDataStream.h"
#include "OgreMesh2.h"
#include "renderwindow_dockwidget.h"
#include "properties_dockwidget.h"
#include "texture_dockwidget.h"
#include "nodeeditor_dockwidget.h"
#include "material_browser_dialog.h"
#include "ogre3_renderman.h"
#include "hlms_editor_plugin.h"
#include "recent_file_action.h"
#include "hlms_utils_manager.h"
#include "constants.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

/****************************************************************************
 MainWindow is the main container window
 ***************************************************************************/
class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow(void);
		~MainWindow(void);
        bool eventFilter(QObject* object, QEvent* event);
		void update(void);
		bool mIsClosing;
        Magus::OgreManager* getOgreManager(void) const {return mOgreManager;}
        PropertiesDockWidget* mPropertiesDockWidget; // Make is public for easy access
        TextureDockWidget* mTextureDockWidget; // Make is public for easy access
        void initCurrentDatablockFileName(void); // Set the name of the current json file to ""
        void getListOfResources(void); // Function to test which resources are loaded
        //void destroyAllDatablocks(void); // Destroy all datablocks
        void destroyDatablock (const QString& datablockName); // Destroy a particular datablock
        //void destroyDatablocksExceptGiven(const Ogre::String& datablockName); // Destroy all datablocks, except the one passed
        EditorHlmsTypes getCurrentDatablockType(void); // Returns the current hlms type
        void loadTextureBrowserCfg(void);
        void setCurrentDatablockNames(const Ogre::IdString& name, const Ogre::String& fullName);
        const Ogre::String& getCurrentDatablockFullName (void) {return mCurrentDatablockFullName;}
        const Ogre::IdString& getCurrentDatablockName (void) {return mCurrentDatablockName;}

    protected:
        // Save the content of a resource vector
        void saveResources(const QString& fileName, const QVector<Magus::QtResourceInfo*>& resources);
        QMessageBox::StandardButton fileDoesNotExistsWarning(const QString& fileName);
        void newProjectName(void);
        void appendRecentHlms(const QString fileName); // Used for recent Hlms files in menu
        void appendRecentProject(const QString fileName); // Used for recent Project files in menu
        bool isMeshV1(const QString modelFileName);
        bool isMeshV2(const QString modelFileName);
        Ogre::MeshPtr convertMeshV1ToV2(const QString fileNameMeshV1);
        void saveV2Mesh(Ogre::MeshPtr v2MeshPtr, QString modelFileName);
        void detachMaterialsFromItem (void);
        void restoreMaterialsOfItem (void);
        void createSpecialDatablocks (void);
        void destroySpecialDatablocks(void);

	private slots:
        void doNewProjectAction(void);
        void doNewHlmsPbsAction(void);
        void doNewHlmsUnlitAction(void);
        void doOpenProjectMenuAction(void);
        void doOpenDatablockMenuAction(void);
        void doOpenModelMenuAction(void);
        void doSaveProjectMenuAction(void);
        void doSaveDatablockMenuAction(void);
        void doSaveAsProjectMenuAction(void);
        void doSaveAsDatablockMenuAction(void);
        void doMaterialBrowserOpenMenuAction(void);
        void doMaterialBrowserAddMenuAction(void);
        void doQuitMenuAction(void);
        void doTextureBrowserImportMenuAction(void);
        void doTextureBrowserAddImageMenuAction(void);
        void doConfigureMenuAction(void);
        void doResetWindowLayoutMenuAction(void);
        void handleTextureDoubleClicked(const QString& fileName, const QString& baseName);
        void handleCustomContextMenuItemSelected(const QString& menuItemText);
        void handleTextureMutationOccured(void);
        void saveTextureBrowserCfg(void);
        void doImport(Ogre::HlmsEditorPlugin* plugin);
        void doExport(Ogre::HlmsEditorPlugin* plugin);
        void constructHlmsEditorPluginData(Ogre::HlmsEditorPluginData* data);
        void doRecentHlmsFileAction(const QString& fileName);
        void doRecentProjectFileAction(const QString& fileName);
        //void doMaterialBrowserAccepted(void);
        void doMaterialBrowserAccepted(const QString& fileName); // TEST
        void doMaterialBrowserRejected(void);
        void doMaterialBrowserClosed(void);

	private:
		void createActions(void);
		void createMenus(void);
		void createToolBars(void);
		void createStatusBar(void);
		void createDockWindows(void);
		void closeEvent(QCloseEvent* event);
        void loadDatablockAndSet(const QString jsonFileName);
        void loadModel(const QString modelFileName);
        void loadProject(const QString& fileName);
        //bool loadDatablock(const QString& jsonFileName);
        void saveDatablock(void);
        void loadMaterialBrowserCfg(void);
        void saveMaterialBrowserCfg(void);
        void loadRecentHlmsFilesCfg(void);
        void saveRecentHlmsFilesCfg(void);
        void loadRecentProjectFilesCfg(void);
        void saveRecentProjectFilesCfg(void);
        //void clearDatablocks(void);
        Ogre::DataStreamPtr openFile(Ogre::String source);

        bool mFirst;
        QString mTempString;
        Ogre::String mTempOgreString;
        MaterialBrowserDialog* mMaterialBrowser;
        QMenu* mFileMenu;
        QMenu* mMaterialBrowserMenu;
        QMenu* mTextureBrowserMenu;
        QMenu* mWindowMenu;
        QMenu* mRecentHlmsFilesMenu;
        QMenu* mRecentProjectFilesMenu;
        QAction* mNewProjectAction;
        QAction* mNewHlmsPbsAction;
        QAction* mNewHlmsUnlitAction;
        QAction* mOpenProjectMenuAction;
        QAction* mOpenDatablockMenuAction;
        QAction* mOpenModelMenuAction;
        QAction* mSaveProjectMenuAction;
        QAction* mSaveDatablockMenuAction;
        QAction* mSaveAsProjectMenuAction;
        QAction* mSaveAsDatablockMenuAction;
        QAction* mMaterialBrowserOpenMenuAction;
        QAction* mMaterialBrowserAddMenuAction;
        QAction* mTextureBrowserImportMenuAction;
        QAction* mConfigureMenuAction;
        QAction* mTextureBrowserAddImageMenuAction;
        QAction* mRecentHlmsFilesMenuAction;
        QAction* mRecentProjectFilesMenuAction;
        QAction* mQuitMenuAction;
        QAction* mResetWindowLayoutMenuAction;
        RenderwindowDockWidget* mRenderwindowDockWidget;
        NodeEditorDockWidget* mNodeEditorDockWidget;
        Magus::OgreManager* mOgreManager;
        QString mProjectName;
        QString mProjectPath;
        QString mMaterialFileName;
        QString mTextureFileName;
        QString mHlmsName; // Used to determine whether a hlms was already saved
        Ogre::IdString mCurrentDatablockName; // The datablock name
        Ogre::String mCurrentDatablockFullName; // The datablock full name
        bool mSaveTextureBrowserTimerActive;
        struct RecentFileStruct
        {
            RecentFileAction* action;
            QString fileName;
        };
        QList<RecentFileStruct> mRecentHlmsFiles; // Used for recent Hlms files in menu
        QList<RecentFileStruct> mRecentProjectFiles; // Used for recent Project files in menu
        QPoint mMaterialBrowserPosition;
        QSize mMaterialBrowserSize;
        HlmsUtilsManager* mHlmsUtilsManager;
};

#endif

