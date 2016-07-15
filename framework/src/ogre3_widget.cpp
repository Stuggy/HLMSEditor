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

#include "ogre3_widget.h"
#include "ogre3_renderman.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "OgreRenderSystem.h"
#include "OgreCommon.h"
#include "OgreTextureManager.h"
#include "OgreTimer.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsPbsDatablock.h"
#include "constants.h"

namespace Magus
{
    //****************************************************************************/
    QOgreWidget::QOgreWidget(QWidget* parent) :
        QWidget(parent),
        mRoot(0),
        mOgreRenderWindow(0),
        mCamera(0),
        mCameraManager(0),
        mTimeSinceLastFrame (0.0f),
        mItem(0),
        mLightAxisItem(0),
        mLight(0),
        mSceneNode(0),
        mLightNode(0),
        mLightAxisNode(0),
        mSceneCreated(false),
        mSystemInitialized(false),
        mRotateCameraMode(true),
        mShiftDown(false),
        mMouseDown(false)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_PaintOnScreen);
        setMinimumSize(240,240);
        resize(800,600);
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);
        mBackground = Ogre::ColourValue(0.1f, 0.1f, 0.1f);
        mAbsolute = Ogre::Vector2::ZERO;
        mRelative = Ogre::Vector2::ZERO;
    }

    //****************************************************************************/
    QOgreWidget::~QOgreWidget()
    {
    }

    //****************************************************************************/
    HGLRC QOgreWidget::getCurrentGlContext(void)
    {
        #if defined(Q_OS_WIN)
            return wglGetCurrentContext(); // Windows
        #else
            return glXGetCurrentContext(); // Linux
        #endif

        return 0;
    }

    //****************************************************************************/
    void QOgreWidget::createRenderWindow(OgreManager* ogreManager)
    {
        if (!ogreManager)
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "No Ogre Manager available", "QOgreWidget::createRenderWindow");

        Ogre::Root* root = ogreManager->getOgreRoot();
        if (!root)
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "No Ogre Root available", "QOgreWidget::createRenderWindow");

        // Get rendersystem and assign window handle
        mRoot = root;
        Ogre::NameValuePairList parameters;

        // Reuse the glContext if available
        HGLRC glContext = 0;
        if (ogreManager->isRenderSystemGL())
        {
            parameters["currentGLContext"] = Ogre::String("false");
            glContext = ogreManager->getGlContext();
            if (glContext)
            {
                parameters["externalGLContext"] = Ogre::StringConverter::toString( (size_t)(glContext) );
                parameters["vsync"] = "No";
            }
        }

        #if defined(Q_OS_MAC) || defined(Q_OS_WIN)
            Ogre::String windowHandle = Ogre::StringConverter::toString((size_t)(this->winId()));;
            parameters["externalWindowHandle"] = windowHandle;
            parameters["parentWindowHandle"] = windowHandle;
        #else
            Ogre::String windowHandle = Ogre::StringConverter::toString((unsigned long)(this->winId()));
            parameters["externalWindowHandle"] = windowHandle;
            parameters["parentWindowHandle"] = windowHandle;
        #endif

        #if defined(Q_OS_MAC)
            parameters["macAPI"] = "cocoa";
            parameters["macAPICocoaUseNSView"] = "true";
        #endif

        mOgreRenderWindow = mRoot->createRenderWindow(Ogre::StringConverter::toString(mRoot->getTimer()->getMicroseconds()),
                                                      this->width(),
                                                      this->height(),
                                                      false,
                                                      &parameters);
        mOgreRenderWindow->setVisible(true);

        // Determine whether the GL context can be reused
        if (ogreManager->isRenderSystemGL() && !glContext)
        {
            // Store the glContext in the ogre manager
            glContext = getCurrentGlContext();
            ogreManager->setGlContext(glContext);
        }

        // Create scene manager
        const size_t numThreads = std::max<int>(1, Ogre::PlatformInformation::getNumLogicalCores());
        Ogre::InstancingThreadedCullingMethod threadedCullingMethod = (numThreads > 1) ? Ogre::INSTANCING_CULLING_THREADED : Ogre::INSTANCING_CULLING_SINGLETHREAD;
        mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC, numThreads, threadedCullingMethod);
        mSceneManager->getRenderQueue()->setRenderQueueMode(1, Ogre::RenderQueue::FAST);
        mSceneManager->getRenderQueue()->setRenderQueueMode(2, Ogre::RenderQueue::FAST);
        mSceneManager->setShadowDirectionalLightExtrusionDistance( 500.0f );
        mSceneManager->setShadowFarDistance( 500.0f );

        // Create camera
        mCamera = mSceneManager->createCamera("MainCamera");
        mCamera->setAspectRatio(Ogre::Real(mOgreRenderWindow->getWidth()) / Ogre::Real(mOgreRenderWindow->getHeight()));
        mCameraManager = new CameraMan(mCamera);

        // Create the compositor
        createCompositor();
    }

    //****************************************************************************/
    void QOgreWidget::createScene()
    {
        // Create the node and attach the entity
        mSceneNode = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->createChildSceneNode( Ogre::SCENE_DYNAMIC );
        mSceneNode->setPosition(0.0, 0.0, 0.0);
        mCameraManager->setTarget(mSceneNode);

        // Create an item
        Ogre::Vector3 scale(25.0f, 25.0f, 25.0f);
        createItem ("cube.mesh", scale);

        // Remove the datablock currently set on this mesh
        Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
        Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

        setDefaultDatablockItem();
        if (hlmsPbs->getDatablock(DATABLOCK_DEBUG_CUBE))
            hlmsPbs->destroyDatablock(DATABLOCK_DEBUG_CUBE);

        // Create light
        mLight = mSceneManager->createLight();
        mLightNode = mCameraManager->mCameraNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        mLightNode->attachObject( mLight );
        mLight->setPowerScale( Ogre::Math::PI ); // Since we don't do HDR, counter the PBS' division by PI
        mLight->setType( Ogre::Light::LT_DIRECTIONAL );
        mLight->setDiffuseColour( Ogre::ColourValue::White );
        mLight->setSpecularColour( Ogre::ColourValue::White );
        mLight->setDirection(Ogre::Vector3(0, 1, 0));

        // Light axis node
        mLightAxisNode = mCameraManager->mCameraNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        mLightAxisNode->setPosition(mCamera->getPosition() + Ogre::Vector3(0, -27, -100));
        mLightAxisItem = mSceneManager->createItem("axis.mesh",
                                                   Ogre::ResourceGroupManager::
                                                   AUTODETECT_RESOURCE_GROUP_NAME,
                                                   Ogre::SCENE_DYNAMIC );
        mLightAxisItem->setRenderQueueGroup(2);
        mLightAxisNode->attachObject(mLightAxisItem);
        mLightAxisNode->setScale(Ogre::Vector3(0.12f, 0.12f, 0.12f));
        createLightAxisMaterial();
        mLightAxisItem->setVisible(false); // TEST

        // Put some light at the bottom, so the materials are not completely dark
        mSceneManager->setAmbientLight( Ogre::ColourValue::White,
                                        Ogre::ColourValue::White,
                                        Ogre::Vector3( 0, 1, 0 ).normalisedCopy());

        mSystemInitialized = true;
    }

    //****************************************************************************/
    void QOgreWidget::createLightAxisMaterial(void)
    {
        try
        {
            // Create a Pbs datablock
            Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
            Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
            Ogre::HlmsMacroblock macroblock;
            macroblock.mDepthCheck = false;
            macroblock.mDepthWrite = false;
            Ogre::HlmsPbsDatablock* datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( AXIS_MATERIAL_NAME,
                                                  AXIS_MATERIAL_NAME,
                                                  macroblock,
                                                  Ogre::HlmsBlendblock(),
                                                  Ogre::HlmsParamVec()));
            datablock->setDiffuse(Ogre::Vector3(1, 0, 0));
            mLightAxisItem->setDatablock(AXIS_MATERIAL_NAME);
        }
        catch (Ogre::Exception e){}
    }

    //****************************************************************************/
    void QOgreWidget::destroyLightAxisMaterial(void)
    {
        try
        {
             mLightAxisItem->setDatablock(DEFAULT_DATABLOCK_NAME);
             Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
             Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
             if (hlmsPbs->getDatablock(AXIS_MATERIAL_NAME))
                 hlmsPbs->destroyDatablock(AXIS_MATERIAL_NAME);
        }
        catch (Ogre::Exception e){}
    }

    //****************************************************************************/
    void QOgreWidget::setLightAxisMaterial(void)
    {
        try
        {
             mLightAxisItem->setDatablock("1234567890HlmsLightAxisItem");
        }
        catch (Ogre::Exception e){}
    }

    //****************************************************************************/
    void QOgreWidget::createItem(const Ogre::String& itemName, const Ogre::Vector3& scale)
    {
        try
        {
            Ogre::String datablockName = "";

            // Delete the old item if available
            if (mItem)
            {
                datablockName = *(mItem->getSubItem(0)->getDatablock()->getFullName());
                setDefaultDatablockItem();
                mSceneNode->detachAllObjects();
                mSceneManager->destroyItem(mItem);
            }

            // Create a new item
            mItem = mSceneManager->createItem(itemName,
                                              Ogre::ResourceGroupManager::
                                              AUTODETECT_RESOURCE_GROUP_NAME,
                                              Ogre::SCENE_DYNAMIC );

            mSceneNode->attachObject(mItem);
            mSceneNode->setScale(scale);
            if (!datablockName.empty())
                mItem->setDatablock(datablockName);
            mItem->setRenderQueueGroup(1);
        }
        catch (Ogre::Exception e)
        {
        }
    }

    //****************************************************************************/
    void QOgreWidget::setItem(Ogre::Item* item, const Ogre::Vector3& scale)
    {
        Ogre::String datablockName = "";

        // Delete the old item if available
        if (mItem)
        {
            datablockName = *(mItem->getSubItem(0)->getDatablock()->getFullName());
            setDefaultDatablockItem();
            mSceneNode->detachAllObjects();
            mSceneManager->destroyItem(mItem);
        }

        // Set the new item
        mItem = item;
        mSceneNode->attachObject(mItem);
        mSceneNode->setScale(scale);
        if (!datablockName.empty())
            mItem->setDatablock(datablockName);
        mItem->setRenderQueueGroup(1);
    }

    //****************************************************************************/
    void QOgreWidget::setDefaultDatablockItem(void)
    {
        Ogre::HlmsDatablock* itemDatablock = mItem->getSubItem(0)->getDatablock();
        Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
        Ogre::HlmsPbs* hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS));
        Ogre::HlmsUnlit* hlmsUnlit = static_cast<Ogre::HlmsUnlit*>( hlmsManager->getHlms(Ogre::HLMS_UNLIT));

        if (itemDatablock != hlmsUnlit->getDefaultDatablock())
            mItem->setDatablock(hlmsUnlit->getDefaultDatablock());
        else
            if (itemDatablock != hlmsPbs->getDefaultDatablock())
                mItem->setDatablock(hlmsPbs->getDefaultDatablock());
            else
                mItem->setDatablock(DEFAULT_DATABLOCK_NAME);
    }

    //****************************************************************************/
    void QOgreWidget::setBackgroundColour(const Ogre::ColourValue colour)
    {
        Ogre::CompositorManager2* compositorManager = mRoot->getCompositorManager2();
        Ogre::CompositorManager2::CompositorNodeDefMap mNodeDefinitions = compositorManager->getNodeDefinitions();
        Ogre::CompositorManager2::CompositorNodeDefMap::const_iterator iter;
        Ogre::CompositorManager2::CompositorNodeDefMap::const_iterator iterStart = mNodeDefinitions.begin();
        Ogre::CompositorManager2::CompositorNodeDefMap::const_iterator iterEnd = mNodeDefinitions.end();
        Ogre::CompositorNodeDef* nodeDef;
        Ogre::CompositorTargetDef* targetDef;
        Ogre::CompositorPassDef* passDef;
        Ogre::CompositorPassClearDef* clearDef;
        for (iter = iterStart; iter != iterEnd; ++iter)
        {
            nodeDef = iter->second;
            if (nodeDef)
            {
                if (nodeDef->getNumTargetPasses() > 0)
                {
                    targetDef = nodeDef->getTargetPass( 0 );
                    Ogre::CompositorPassDefVec passDefs = targetDef->getCompositorPasses();
                    Ogre::CompositorPassDefVec::const_iterator iterPass;
                    Ogre::CompositorPassDefVec::const_iterator iterPassStart = passDefs.begin();
                    Ogre::CompositorPassDefVec::const_iterator iterPassEnd = passDefs.end();
                    for (iterPass = iterPassStart; iterPass != iterPassEnd; ++iterPass)
                    {
                        passDef = *iterPass;
                        if (Ogre::PASS_CLEAR == passDef->getType())
                        {
                            clearDef = static_cast<Ogre::CompositorPassClearDef*>( passDef );
                            clearDef->mColourValue = colour;
                        }
                    }
                }
            }
        }
    }

    //****************************************************************************/
    void QOgreWidget::createCompositor()
    {
        Ogre::CompositorManager2* compositorManager = mRoot->getCompositorManager2();
        const Ogre::String workspaceName = Ogre::StringConverter::toString(mRoot->getTimer()->getMicroseconds());
        const Ogre::IdString workspaceNameHash = workspaceName;
        compositorManager->createBasicWorkspaceDef(workspaceName, mBackground);
        mWorkspace = compositorManager->addWorkspace(mSceneManager, mOgreRenderWindow, mCamera, workspaceNameHash, true);
    }

    //****************************************************************************/
    void QOgreWidget::updateOgre(Ogre::Real timeSinceLastFrame)
    {
        if (!mSceneCreated)
        {
            createScene();
            mSceneCreated = true;
        }

        mTimeSinceLastFrame = timeSinceLastFrame;
        repaint();
        if (this->size() != this->parentWidget()->size())
            resize(this->parentWidget()->size());
    }

    //-------------------------------------------------------------------------------------
    QPaintEngine* QOgreWidget::paintEngine() const
    {
        // We don't want another paint engine to get in the way for our Ogre based paint engine.
        // So we return nothing.
        return 0;
    }

    //-------------------------------------------------------------------------------------
    void QOgreWidget::paintEvent(QPaintEvent *e)
    {
    }

    //-------------------------------------------------------------------------------------
    void QOgreWidget::resizeEvent(QResizeEvent *e)
    {
        if(e->isAccepted())
        {
            const QSize &newSize = e->size();
            if(mCamera && mOgreRenderWindow)
            {
                mOgreRenderWindow->resize(newSize.width(), newSize.height());
                mOgreRenderWindow->windowMovedOrResized();
                Ogre::Real aspectRatio = Ogre::Real(newSize.width()) / Ogre::Real(newSize.height());
                mCamera->setAspectRatio(aspectRatio);
            }
        }
    }

    //****************************************************************************/
    void QOgreWidget::keyPressEvent(QKeyEvent * ev)
    {
        if(mSystemInitialized)
            mCameraManager->injectKeyDown(ev);

        if(ev->key() == Qt::Key_Shift)
            mShiftDown = true;
    }

    //****************************************************************************/
    void QOgreWidget::keyReleaseEvent(QKeyEvent * ev)
    {
        if(mSystemInitialized)
            mCameraManager->injectKeyUp(ev);

        if(ev->key() == Qt::Key_Shift)
            mShiftDown = false;
    }

    //****************************************************************************/
    void QOgreWidget::enableLightItem(bool enabled)
    {
        if (mLightAxisItem)
        {
            mRotateCameraMode = !enabled; // We want to rotate the light if enabled = true
            mLightAxisItem->setVisible(enabled);
        }
    }

    //****************************************************************************/
    void QOgreWidget::mouseMoveEvent( QMouseEvent* e )
    {
        if(mSystemInitialized)
        {
            Ogre::Vector2 oldPos = mAbsolute;
            mAbsolute = Ogre::Vector2(e->pos().x(), e->pos().y());
            mRelative = mAbsolute - oldPos;
            if (mRotateCameraMode)
                mCameraManager->injectMouseMove(mRelative);
            else
                rotateLight(mRelative);
        }
    }

    //****************************************************************************/
    void QOgreWidget::rotateLight(Ogre::Vector2 relativeMouseMove)
    {
        if (mMouseDown)
        {
            mLightAxisNode->roll(Ogre::Degree(relativeMouseMove.x * 0.25f));
            mLightAxisNode->pitch(Ogre::Degree(relativeMouseMove.y * 0.25f));
            mLight->setDirection(mLightAxisNode->getOrientation().yAxis()); // Light direction follows light axis
        }
    }

    //****************************************************************************/
    void QOgreWidget::wheelEvent(QWheelEvent *e)
    {
        if(mSystemInitialized)
            mCameraManager->injectMouseWheel(e);

        mLightAxisNode->setPosition(mCamera->getPosition() + Ogre::Vector3(0, -27, -100));
    }

    //****************************************************************************/
    void QOgreWidget::mousePressEvent( QMouseEvent* e )
    {
        if(mSystemInitialized)
            mCameraManager->injectMouseDown(e);

        if (e->button() == Qt::MiddleButton || e->button() == Qt::LeftButton)
            mMouseDown = true;
    }

    //****************************************************************************/
    void QOgreWidget::mouseReleaseEvent( QMouseEvent* e )
    {
        if(mSystemInitialized)
            mCameraManager->injectMouseUp(e);

        if (e->button() == Qt::MiddleButton || e->button() == Qt::LeftButton)
            mMouseDown = false;
    }

    //****************************************************************************/
    const Ogre::Vector3& QOgreWidget::getItemScale(void)
    {
        if (mItem && mItem->getParentSceneNode())
            return mItem->getParentSceneNode()->getScale();
    }

    //****************************************************************************/
    void QOgreWidget::setItemScale(const Ogre::Vector3& scale)
    {
        if (mItem && mItem->getParentSceneNode())
            mItem->getParentSceneNode()->setScale(scale);
    }

    //****************************************************************************/
    void QOgreWidget::saveToFile(const Ogre::String& fileName)
    {
        mOgreRenderWindow->writeContentsToFile(fileName);
    }
}
