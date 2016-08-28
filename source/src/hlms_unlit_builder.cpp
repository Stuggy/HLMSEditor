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

// Include
#include <QMessageBox>
#include "constants.h"
#include "node_port.h"
#include "hlms_unlit_builder.h"
#include "OgreItem.h"
#include "OgreResourceGroupManager.h"

//****************************************************************************/
HlmsUnlitBuilder::HlmsUnlitBuilder(Magus::QtNodeEditor* nodeEditor) :
    HlmsBuilder(),
    mNodeEditor(nodeEditor)
{
    mTempOgreString = "";
}

//****************************************************************************/
HlmsUnlitBuilder::~HlmsUnlitBuilder(void)
{
}

//****************************************************************************/
void HlmsUnlitBuilder::deleteUnlitDatablock (Magus::OgreManager* ogreManager, const QString& datablockName)
{
    Ogre::String name = datablockName.toStdString();
    if (name == DEFAULT_DATABLOCK_NAME)
        return;

    // Get the ogre manager, root and hlms managers
    Ogre::Root* root = ogreManager->getOgreRoot();
    Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
    Ogre::HlmsUnlit* hlmsUnlit = static_cast<Ogre::HlmsUnlit*>( hlmsManager->getHlms(Ogre::HLMS_UNLIT) );
    if (hlmsUnlit->getDatablock(name))
    {
        // Destroy any existing datablock with that name
        try
        {
            ogreManager->getOgreWidget(OGRE_WIDGET_RENDERWINDOW)->setDefaultDatablockItem();
            hlmsUnlit->destroyDatablock(name);
        }
        catch (Ogre::Exception e){}
    }
}

//****************************************************************************/
Ogre::HlmsUnlitDatablock* HlmsUnlitBuilder::createUnlitDatablock (Magus::OgreManager* ogreManager,
                                                                  HlmsNodeUnlitDatablock* unlitnode)
{
    // Get the ogre manager and root
    Ogre::Root* root = ogreManager->getOgreRoot();

    // Create an Unlit datablock
    Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
    Ogre::HlmsTextureManager* hlmsTextureManager = hlmsManager->getTextureManager();
    Ogre::HlmsUnlit* hlmsUnlit = static_cast<Ogre::HlmsUnlit*>( hlmsManager->getHlms(Ogre::HLMS_UNLIT) );

    // Determine whether a macro node is attached (and enabled)
    Ogre::HlmsMacroblock macroblock;
    Magus::QtNode* node = unlitnode->getNodeConnectedToPort(PORT_MACROBLOCK);
    if (node)
    {
        // There is a macroblock; enrich it with the node data if enabled
        HlmsNodeMacroblock* macronode = static_cast<HlmsNodeMacroblock*>(node);
        if (macronode->getMacroblockEnabled())
            enrichMacroblock(macronode, &macroblock);
    }

    // Determine whether a blend node is attached (and enabled)
    Ogre::HlmsBlendblock blendblock;
    Magus::QtNode* bnode = unlitnode->getNodeConnectedToPort(PORT_BLENDBLOCK);
    if (bnode)
    {
        // There is a blendblock; enrich it with the node data if enabled
        HlmsNodeBlendblock* blendnode = static_cast<HlmsNodeBlendblock*>(bnode);
        if (blendnode->getBlendblockEnabled())
            enrichBlendblock(blendnode, &blendblock);
    }

    // First destroy any existing datablock with that name
    Ogre::String datablockName = unlitnode->getName().toStdString();
    Ogre::HlmsDatablock* latestDatablock = hlmsUnlit->getDatablock(datablockName);
    if (latestDatablock && latestDatablock != hlmsUnlit->getDefaultDatablock())
        hlmsUnlit->destroyDatablock(latestDatablock->getName());

    // Create a new datablock and use the (new) name defined in the node
    Ogre::HlmsUnlitDatablock* datablock = static_cast<Ogre::HlmsUnlitDatablock*>(
                hlmsUnlit->createDatablock( datablockName,
                                            datablockName,
                                            macroblock,
                                            blendblock,
                                            Ogre::HlmsParamVec()));

    // Set the datablock properties
    enrichUnlitDatablock(datablock, unlitnode);

    // Run through all nodes connected to the unlitnode
    QVector<Magus::QtNode*> nodes = unlitnode->getNodes();
    Ogre::String texName;
    Ogre::String dataFolder;
    unsigned int texUnit = 0;
    foreach(Magus::QtNode* node, nodes)
    {
        if (node)
        {
            if (node->getType() == NODE_TYPE_SAMPLERBLOCK)
            {
                try
                {
                    // First check whether the texture/samperblock is enabled and must part of the datablock
                    HlmsNodeSamplerblock* samplernode = static_cast<HlmsNodeSamplerblock*>(node);
                    if (samplernode->getSamplerblockEnabled())
                    {
                        // Get the texture type from the sampler node and add a resourcelocation if needed
                        texName = samplernode->getBaseNameTexture().toStdString();
                        if (!texName.empty())
                        {
                            // Add resource location if needed
                            dataFolder = samplernode->getPathTexture().toStdString();
                            if (!isResourceLocationExisting(dataFolder))
                            {
                                root->addResourceLocation(dataFolder, "FileSystem", "General");
                                saveAllResourcesLocations();
                            }

                            // Get the texture from the samplernode
                            Ogre::HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                    createOrRetrieveTexture(texName, Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

                            // Create a samplerblock and add it to the datablock
                            Ogre::HlmsSamplerblock samplerblock;
                            samplernode->setTextureIndex(texUnit); // Needed to identify the corresponding texture when enriching the samplerblock
                            enrichSamplerblock(datablock, &samplerblock, samplernode);
                            datablock->setTexture(texUnit, texLocation.xIdx, texLocation.texture);
                            datablock->setSamplerblock(texUnit, samplerblock);
                            ++texUnit;
                        }
                    }
                }
                catch (Ogre::Exception e)
                {
                    QMessageBox::information(0, QString("Error"), QString("Cannot create textures. Check Ogre.log"));
                }
            }
        }
    }

    // Set the datablock in the item
    //Ogre::Item* item = ogreManager->getOgreWidget(OGRE_WIDGET_RENDERWINDOW)->getItem();
    //item->setDatablock(datablock);
    return datablock;
}

//****************************************************************************/
HlmsNodeUnlitDatablock* HlmsUnlitBuilder::createUnlitNodeStructure(Magus::OgreManager* ogreManager,
                                                                   const QString& datablockName)
{
    // Get the datablock
    HlmsNodeUnlitDatablock* unlitnode;
    Ogre::Root* root = ogreManager->getOgreRoot();
    Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
    Ogre::HlmsUnlit* hlmsUnlit = static_cast<Ogre::HlmsUnlit*>( hlmsManager->getHlms(Ogre::HLMS_UNLIT) );
    Ogre::String name = datablockName.toStdString();
    Ogre::HlmsUnlitDatablock* datablock = static_cast<Ogre::HlmsUnlitDatablock*>(hlmsUnlit->getDatablock(name));
    if (datablock)
    {
        mNodeEditor->clear();

        // Create the unlit node
        unlitnode = createUnlitNode();
        enrichUnlitNode(unlitnode, datablock);

        // Get all textures from the unlit
        // Note, that each texture becomes one samplernode, while a datablock may contain multiple textures, but for example only one samplerblock
        createSamplerNodes(ogreManager, unlitnode, datablock);

        // Create and fill properties of the macronode with the values of the macroblock and connect the node to the unlit node
        // Note, that the default macroblock of an Unlit datablock has cull mode == CULL_ANTICLOCKWISE (this differs from PBS)
        // Is this correct?
        const Ogre::HlmsMacroblock* macroblock = datablock->getMacroblock(mNodeEditor);
        if (!(!macroblock->mScissorTestEnabled &&
              macroblock->mDepthCheck &&
              macroblock->mDepthWrite &&
              macroblock->mDepthFunc == Ogre::CMPF_LESS_EQUAL &&
              macroblock->mDepthBiasConstant == 0.0f &&
              macroblock->mDepthBiasSlopeScale == 0.0f &&
              macroblock->mCullMode == Ogre::CULL_ANTICLOCKWISE &&
              macroblock->mPolygonMode == Ogre::PM_SOLID))
        {
            // It is not a default macroblock, so create a new macro node and and set it in the unlit
            HlmsNodeMacroblock* macronode = createMacroNode(mNodeEditor);
            macronode->setMacroblockEnabled(true);
            macronode->setScissorTestEnabled(macroblock->mScissorTestEnabled);
            macronode->setDepthCheck(macroblock->mDepthCheck);
            macronode->setDepthWrite(macroblock->mDepthWrite);
            macronode->setDepthFunc(getIndexFromCompareFunction(macroblock->mDepthFunc));
            macronode->setDepthBiasConstant(macroblock->mDepthBiasConstant);
            macronode->setDepthBiasSlopeScale(macroblock->mDepthBiasSlopeScale);

            switch (macroblock->mCullMode)
            {
                case Ogre::CULL_NONE:
                    macronode->setCullMode(0);
                break;
                case Ogre::CULL_CLOCKWISE:
                    macronode->setCullMode(1);
                break;
                case Ogre::CULL_ANTICLOCKWISE:
                    macronode->setCullMode(2);
                break;
            }

            switch (macroblock->mPolygonMode)
            {
                case Ogre::PM_POINTS:
                    macronode->setPolygonMode(0);
                break;
                case Ogre::PM_WIREFRAME:
                    macronode->setPolygonMode(1);
                break;
                case Ogre::PM_SOLID:
                    macronode->setPolygonMode(2);
                break;
            }
            connectNodes(unlitnode, macronode); // Connect both nodes
        }

        // Create and fill properties of the blendnode with the values of the blendblock and connect the node to the unlit node
        const Ogre::HlmsBlendblock* blendblock = datablock->getBlendblock(mNodeEditor);
        if (!(!blendblock->mAlphaToCoverageEnabled &&
              blendblock->mBlendChannelMask == Ogre::HlmsBlendblock::BlendChannelAll &&
              !blendblock->mIsTransparent &&
              !blendblock->mSeparateBlend &&
              blendblock->mSourceBlendFactor == Ogre::SBF_ONE &&
              blendblock->mDestBlendFactor == Ogre::SBF_ZERO &&
              blendblock->mSourceBlendFactorAlpha == Ogre::SBF_ONE &&
              blendblock->mDestBlendFactorAlpha == Ogre::SBF_ZERO &&
              blendblock->mBlendOperation == Ogre::SBO_ADD &&
              blendblock->mBlendOperationAlpha == Ogre::SBO_ADD))
        {
            // It is not a default blendblock, so create a new blend node and set it in the unlit
            HlmsNodeBlendblock* blendnode = createBlendNode(mNodeEditor);
            blendnode->setBlendblockEnabled(true);
            blendnode->setAlphaToCoverageEnabled(blendblock->mAlphaToCoverageEnabled);
            switch (blendblock->mBlendChannelMask)
            {
                case Ogre::HlmsBlendblock::BlendChannelRed:
                    blendnode->setBlendChannelMask(0);
                break;
                case Ogre::HlmsBlendblock::BlendChannelGreen:
                    blendnode->setBlendChannelMask(1);
                break;
                case Ogre::HlmsBlendblock::BlendChannelBlue:
                    blendnode->setBlendChannelMask(2);
                break;
                case Ogre::HlmsBlendblock::BlendChannelAlpha:
                    blendnode->setBlendChannelMask(3);
                break;
                case Ogre::HlmsBlendblock::BlendChannelAll:
                    blendnode->setBlendChannelMask(4);
                break;
            }
            blendnode->setTransparent(blendblock->mIsTransparent);
            blendnode->setSeparateBlend(blendblock->mSeparateBlend);
            blendnode->setSourceBlendFactor(getIndexFromSceneBlendFactor(blendblock->mSourceBlendFactor));
            blendnode->setDestBlendFactor(getIndexFromSceneBlendFactor(blendblock->mDestBlendFactor));
            blendnode->setSourceBlendFactorAlpha(getIndexFromSceneBlendFactor(blendblock->mSourceBlendFactorAlpha));
            blendnode->setDestBlendFactorAlpha(getIndexFromSceneBlendFactor(blendblock->mDestBlendFactorAlpha));
            blendnode->setBlendOperation(getIndexFromSceneBlendOperation(blendblock->mBlendOperation));
            blendnode->setBlendOperationAlpha(getIndexFromSceneBlendOperation(blendblock->mBlendOperationAlpha));
            connectNodes(unlitnode, blendnode); // Connect both nodes
        }
    }

    return unlitnode;
}

//****************************************************************************/
void HlmsUnlitBuilder::enrichUnlitDatablock(Ogre::HlmsUnlitDatablock* datablock,
                                            HlmsNodeUnlitDatablock* unlitnode)
{
    if (!datablock)
        return;

    if (!unlitnode)
        return;

    // ******** Colour ********
    Ogre::ColourValue colour;
    colour.r = (float)unlitnode->getColour().red() / 255.0f;
    colour.g = (float)unlitnode->getColour().green() / 255.0f;
    colour.b = (float)unlitnode->getColour().blue() / 255.0f;
    colour.a = (float)unlitnode->getColour().alpha() / 255.0f;

    // If the colour isn't white, set the flag
    if (colour != Ogre::ColourValue::White)
        datablock->setUseColour(true);
    else
        datablock->setUseColour(false);

    // Set the colour
    datablock->setColour(colour);

    // ******** Texture Swizzle ********
    // TODO: Not yet implemented (is it needed?)

    // ******** Alphatest threshold ********
    datablock->setAlphaTest(getCompareFunctionFromIndex(unlitnode->getAlphaTest()));

    // ******** Alphatest threshold ********
    datablock->setAlphaTestThreshold(unlitnode->getAlphaTestThreshold());
}

//****************************************************************************/
void HlmsUnlitBuilder::enrichUnlitNode(HlmsNodeUnlitDatablock* unlitnode,
                                       Ogre::HlmsUnlitDatablock* datablock)
{
    // ******** Name ********
    unlitnode->setName(datablock->getFullName()->c_str());

    // ******** Colour ********
    QColor colour;
    colour.setRed(255.0f * datablock->getColour().r);
    colour.setGreen(255.0f * datablock->getColour().g);
    colour.setBlue(255.0f * datablock->getColour().b);
    colour.setAlpha(255.0f * datablock->getColour().a);
    unlitnode->setColour(colour);

    // ******** Texture Swizzle ********
    // TODO: Not yet implemented (is it needed?)

    // ******** Alphatest ********
    unlitnode->setAlphaTest(getIndexFromCompareFunction(datablock->getAlphaTest()));

    // ******** Alphatest threshold ********
    unlitnode->setAlphaTestThreshold(datablock->getAlphaTestThreshold());
}

//****************************************************************************/
HlmsNodeUnlitDatablock* HlmsUnlitBuilder::createUnlitNode(void)
{
    HlmsNodeUnlitDatablock* node = new HlmsNodeUnlitDatablock(NODE_TITLE_UNLIT_DATABLOCK);
    node->setType(NODE_TYPE_UNLIT_DATABLOCK);
    mNodeEditor->addNode(node);
    repositionUnlitNode(node);
    return node;
}

//****************************************************************************/
void HlmsUnlitBuilder::createSamplerNodes (Magus::OgreManager* ogreManager,
                                           HlmsNodeUnlitDatablock* unlitnode,
                                           Ogre::HlmsUnlitDatablock* datablock)
{
    Ogre::uint8 max = Ogre::UnlitTextureTypes::NUM_UNLIT_TEXTURE_TYPES;
    Ogre::uint8 i;
    const Ogre::HlmsSamplerblock* samplerblock;
    HlmsNodeSamplerblock* samplernode;
    for (i = 0; i < max; ++i)
    {
        samplerblock = datablock->getSamplerblock(i);
        if (samplerblock)
        {
            samplernode = createSamplerNode(mNodeEditor);
            enrichSamplerNode (ogreManager, samplernode, samplerblock, datablock, i);
            connectNodes(unlitnode, samplernode);
        }
    }
}

//****************************************************************************/
void HlmsUnlitBuilder::enrichSamplerblock (Ogre::HlmsUnlitDatablock* datablock,
                                           Ogre::HlmsSamplerblock* samplerblock,
                                           HlmsNodeSamplerblock* samplernode)
{
    // ******** Generic properties ********
    enrichSamplerBlockGeneric(samplerblock, samplernode);

    // ******** UV set ********
    datablock->setTextureUvSource(samplernode->getTextureIndex(), samplernode->getUvSet());

    // ******** Blend mode ********
    Ogre::UnlitBlendModes blendMode = getBlendModeFromIndex (samplernode->getBlendMode());
    datablock->setBlendMode(samplernode->getTextureIndex(), blendMode);

    // ******** Animation Matrix ********
    // TODO: Not yet implemented

    // ******** Map weight ********
    // Not applicable; unlit does not use this
}


//****************************************************************************/
void HlmsUnlitBuilder::enrichSamplerNode (Magus::OgreManager* ogreManager,
                                          HlmsNodeSamplerblock* samplernode,
                                          const Ogre::HlmsSamplerblock* samplerblock,
                                          Ogre::HlmsUnlitDatablock* datablock,
                                          Ogre::uint8 textureType)
{
    // ******** Texture (name) ********
    // Getting the filename of the texture
    Ogre::HlmsManager* hlmsManager = ogreManager->getOgreRoot()->getHlmsManager();
    Ogre::HlmsTextureManager::TextureLocation texLocation;
    texLocation.texture = datablock->getTexture(textureType);
    Ogre::String basename;
    const Ogre::String* pBasename;
    if (!texLocation.texture.isNull())
    {
       texLocation.xIdx = 0;
       //texLocation.xIdx = datablock->_getTextureIdx( textureType ); // TODO: Change as result in Ogre commit 21b9540
       texLocation.yIdx = 0;
       texLocation.divisor = 1;
       pBasename = hlmsManager->getTextureManager()->findAliasName(texLocation); // findAliasName could return 0 pointer
       if (pBasename)
       {
           basename = *pBasename;
       }
       else
       {
           QMessageBox::information(0, QString("Error"), QString("Cannot find image file. Is the resource location present in " +
                                                                 getResourcesCfg() +
                                                                 QString("?")));
           return;
       }
    }

    // Search the file and path
    Ogre::String filename;
    Ogre::String path;
    Ogre::FileInfoListPtr list = Ogre::ResourceGroupManager::getSingleton().listResourceFileInfo(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME) ;
    Ogre::FileInfoList::iterator it;
    Ogre::FileInfoList::iterator itStart = list->begin();
    Ogre::FileInfoList::iterator itEnd = list->end();
    for(it = itStart; it != itEnd; ++it)
    {
        Ogre::FileInfo& fileInfo = (*it);
        if (fileInfo.basename == basename)
        {
            filename = fileInfo.filename;
            path = fileInfo.archive->getName();
            break;
        }
    }

    // ******** Texture ********
    samplernode->setFileNameTexture((path + "/" + basename).c_str());
    samplernode->setPathTexture(path.c_str());
    samplernode->setBaseNameTexture(basename.c_str());

    // ******** Texture type ********
    // Not applicable; unlit does not use this

    // ******** Generic attributes ********
    enrichSamplerNodeGeneric(samplernode, samplerblock);

    // ******** UV set ********
    Ogre::uint8 uvSource = 0;
    if (textureType < Ogre::NUM_UNLIT_TEXTURE_TYPES)
        uvSource = datablock->getTextureUvSource(textureType);
    samplernode->setUvSet(uvSource);

    // ******** Blend mode ********
    unsigned int index = getIndexFromBlendMode(datablock->getBlendMode(textureType));
    samplernode->setBlendMode(index);

    // ******** Animation Matrix ********
    // TODO: Not yet implemented

    // ******** Map weight ********
    // Not applicable; unlit does not use this
}

//****************************************************************************/
void HlmsUnlitBuilder::repositionUnlitNode(HlmsNodeUnlitDatablock* unlitnode)
{
    if (!unlitnode)
        return;

    // Reposition the unlit node (this is only done once)
    QPointF pos = unlitnode->pos();
    //pos.setX(-1.0f * unlitnode->getWidth());
    //pos.setY(-0.5f * unlitnode->getHeigth());
    unlitnode->setPos(pos);
}

//****************************************************************************/
void HlmsUnlitBuilder::connectNodes(HlmsNodeUnlitDatablock* unlitnode,
                                    HlmsNodeSamplerblock* samplernode)
{
    if (!unlitnode)
        return;

    if (!samplernode)
        return;

    // Connect unlit and sampler
    Magus::QtPort* portUnlit = unlitnode->getFirstFreePort(PORT_ID_UNLIT_DATABLOCK, PORT_ID_UNLIT_DATABLOCK + 7);
    if (!portUnlit)
        return;

    unlitnode->connectNode(PORT_ID_UNLIT_DATABLOCK,
                           PORT_ID_UNLIT_DATABLOCK + 7,
                           samplernode,
                           PORT_ID_SAMPLERBLOCK);

    // Reposition the sampler node
    QPointF pos = unlitnode->pos();
    pos.setX(pos.x() + unlitnode->getWidth() + 2 * portUnlit->pos().y());
    pos.setY(portUnlit->pos().y());
    samplernode->setPos(pos);
}

//****************************************************************************/
void HlmsUnlitBuilder::connectNodes(HlmsNodeUnlitDatablock* unlitnode,
                                    HlmsNodeMacroblock* macronode)
{
    if (!unlitnode)
        return;

    if (!macronode)
        return;

    // Connect unlitnode and macro
    Magus::QtPort* portUnlit = unlitnode->getFirstFreePort(PORT_ID_UNLIT_DATABLOCK + 8, PORT_ID_UNLIT_DATABLOCK + 8);
    if (!portUnlit)
        return;

    unlitnode->connectNode(PORT_MACROBLOCK, macronode, PORT_ID_MACROBLOCK);

    // Reposition the macro node
    QPointF pos = unlitnode->pos();
    pos.setX(pos.x() + unlitnode->getWidth() + 2 * portUnlit->pos().y());
    pos.setY(portUnlit->pos().y() - unlitnode->getHeigth());
    macronode->setPos(pos);
}

//****************************************************************************/
void HlmsUnlitBuilder::connectNodes(HlmsNodeUnlitDatablock* unlitnode,
                                    HlmsNodeBlendblock* blendnode)
{
    if (!unlitnode)
        return;

    if (!blendnode)
        return;

    // Connect unlitnode and blend
    Magus::QtPort* portUnlit = unlitnode->getFirstFreePort(PORT_ID_UNLIT_DATABLOCK + 9, PORT_ID_UNLIT_DATABLOCK + 9);
    if (!portUnlit)
        return;

    unlitnode->connectNode(PORT_BLENDBLOCK, blendnode, PORT_ID_BLENDBLOCK);

    // Reposition the blend node
    QPointF pos = unlitnode->pos();
    pos.setX(pos.x() + unlitnode->getWidth() + 2 * portUnlit->pos().y());
    pos.setY(portUnlit->pos().y() - unlitnode->getHeigth());
    blendnode->setPos(pos);
}

//****************************************************************************/
unsigned int HlmsUnlitBuilder::getIndexFromBlendMode (Ogre::UnlitBlendModes blendMode)
{
    switch (blendMode)
    {
        case Ogre::UNLIT_BLEND_NORMAL_NON_PREMUL:
            return 0;
        break;
        case Ogre::UNLIT_BLEND_NORMAL_PREMUL:
            return 1;
        break;
        case Ogre::UNLIT_BLEND_ADD:
            return 2;
        break;
        case Ogre::UNLIT_BLEND_SUBTRACT:
            return 3;
        break;
        case Ogre::UNLIT_BLEND_MULTIPLY:
            return 4;
        break;
        case Ogre::UNLIT_BLEND_MULTIPLY2X:
            return 5;
        break;
        case Ogre::UNLIT_BLEND_SCREEN:
            return 6;
        break;
        case Ogre::UNLIT_BLEND_OVERLAY:
            return 7;
        break;
        case Ogre::UNLIT_BLEND_LIGHTEN:
            return 8;
        break;
        case Ogre::UNLIT_BLEND_DARKEN:
            return 9;
        break;
        case Ogre::UNLIT_BLEND_GRAIN_EXTRACT:
            return 10;
        break;
        case Ogre::UNLIT_BLEND_GRAIN_MERGE:
            return 11;
        break;
        case Ogre::UNLIT_BLEND_DIFFERENCE:
            return 12;
        break;
    }
    return 0;
}

//****************************************************************************/
Ogre::UnlitBlendModes HlmsUnlitBuilder::getBlendModeFromIndex (unsigned int index)
{
    switch (index)
    {
        case 0:
            return Ogre::UNLIT_BLEND_NORMAL_NON_PREMUL;
        break;
        case 1:
            return Ogre::UNLIT_BLEND_NORMAL_PREMUL;
        break;
        case 2:
            return Ogre::UNLIT_BLEND_ADD;
        break;
        case 3:
            return Ogre::UNLIT_BLEND_SUBTRACT;
        break;
        case 4:
            return Ogre::UNLIT_BLEND_MULTIPLY;
        break;
        case 5:
            return Ogre::UNLIT_BLEND_MULTIPLY2X;
        break;
        case 6:
            return Ogre::UNLIT_BLEND_SCREEN;
        break;
        case 7:
            return Ogre::UNLIT_BLEND_OVERLAY;
        break;
        case 8:
            return Ogre::UNLIT_BLEND_LIGHTEN;
        break;
        case 9:
            return Ogre::UNLIT_BLEND_DARKEN;
        break;
        case 10:
            return Ogre::UNLIT_BLEND_GRAIN_EXTRACT;
        break;
        case 11:
            return Ogre::UNLIT_BLEND_GRAIN_MERGE;
        break;
        case 12:
            return Ogre::UNLIT_BLEND_DIFFERENCE;
        break;
    }
    return Ogre::UNLIT_BLEND_NORMAL_NON_PREMUL;
}

//****************************************************************************/
const Ogre::String& HlmsUnlitBuilder::getTextureName(Magus::OgreManager* ogreManager,
                                                     Ogre::HlmsUnlitDatablock* unlitDatablock,
                                                     Ogre::uint8 textureType)
{
    mTempOgreString = "";
    Ogre::Root* root = ogreManager->getOgreRoot();
    Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
    Ogre::HlmsTextureManager::TextureLocation texLocation;
    texLocation.texture = unlitDatablock->getTexture(textureType);
    const Ogre::String* pBasename;
    if (!texLocation.texture.isNull())
    {
       texLocation.xIdx = 0;
       //texLocation.xIdx = datablock->_getTextureIdx( textureType ); // TODO: Change as result in Ogre commit 21b9540
       texLocation.yIdx = 0;
       texLocation.divisor = 1;
       pBasename = hlmsManager->getTextureManager()->findAliasName(texLocation); // findAliasName could return 0 pointer
       if (pBasename)
       {
           mTempOgreString = *pBasename;
       }
    }

    return mTempOgreString;
}

//****************************************************************************/
void HlmsUnlitBuilder::getTexturesFromAvailableDatablocks(Magus::OgreManager* ogreManager, std::vector<Ogre::String>* v)
{
    // Get all textures from the currently available unlit datablocks
    Ogre::Root* root = ogreManager->getOgreRoot();
    Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
    Ogre::HlmsUnlit* hlmsUnlit = static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    // Iterate through all Unlit
    Ogre::Hlms::HlmsDatablockMap::const_iterator itorUnlit = hlmsUnlit->getDatablockMap().begin();
    Ogre::Hlms::HlmsDatablockMap::const_iterator endUnlit = hlmsUnlit->getDatablockMap().end();
    Ogre::HlmsUnlitDatablock* unlitDatablock;
    Ogre::String basename;
    while (itorUnlit != endUnlit)
    {
        unlitDatablock = static_cast<Ogre::HlmsUnlitDatablock*>(itorUnlit->second.datablock);
        Ogre::uint8 texType = 0;
        while (texType < Ogre::NUM_UNLIT_TEXTURE_TYPES)
        {
            basename = getTextureName(ogreManager, unlitDatablock, texType);
            if (!basename.empty())
                v->push_back(basename);

            ++texType;
        }

        ++itorUnlit;
    }
}
