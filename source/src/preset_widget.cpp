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
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QDir>
#include <QFileInfo>
#include "magus_core.h"
#include "preset_widget.h"
#include "brush_preset_dockwidget.h"
#include "tool_default_texturewidget.h"

//****************************************************************************/
PresetWidget::PresetWidget (const QString& presetDir, BrushPresetDockWidget* brushPresetDockWidget, QWidget* parent) :
    QWidget(parent),
    mBrushPresetDockWidget(brushPresetDockWidget)
{
    setWindowTitle(QString("Presets"));
    QHBoxLayout* mainLayout = new QHBoxLayout;
    QVBoxLayout* tableLayout = new QVBoxLayout;
    mPresetDir = presetDir;

    mTextureWidget = new Magus::QtDefaultTextureWidget(this);
    mTextureWidget->setTextureSize(QSize(113, 64)); // Take 16:9 aspect into account and add 8 pixels to the width to compensate the frame width
    loadPresets (mPresetDir);
    connect(mTextureWidget, SIGNAL(doubleClicked(QString,QString)), this, SLOT(handlePresetDoubleClicked(QString,QString)));

    // Layout
    mainLayout->addWidget(mTextureWidget);
    mainLayout->addLayout(tableLayout);
    setLayout(mainLayout);
}

//****************************************************************************/
PresetWidget::~PresetWidget (void)
{
}

//****************************************************************************/
void PresetWidget::loadPresets(const QString& searchPath)
{
    // Get all texture files from all dirs/subdirs
    QDir dir(searchPath);
    dir.makeAbsolute();

    if (dir.exists())
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
            {
                loadPresets(info.absoluteFilePath());
            }
            else
            {
                QString fileName = info.absoluteFilePath();
                if (Magus::isTypeBasedOnExtension(fileName, Magus::MAGUS_SUPPORTED_IMAGE_FORMATS, Magus::MAGUS_SUPPORTED_IMAGE_FORMATS_LENGTH))
                {
                    // It is an image and must contain a ".json." string, to distinguish them from regular image files (only the thumb image may contain this ".json." string)
                    if (info.fileName().contains(".json."))
                    {
                        QPixmap texturePixmap = QPixmap(info.absoluteFilePath());
                        mTextureWidget->addTexture(texturePixmap, info.absoluteFilePath(), info.fileName());
                    }
                }
            }
        }
    }
}

//****************************************************************************/
void PresetWidget::handlePresetDoubleClicked (const QString& name, const QString& baseName)
{
    emit presetDoubleClicked(name, baseName);
}

//****************************************************************************/
void PresetWidget::addPreset (const QString& path, const QString& thumbName)
{
    QString fileName = path + thumbName;
    QPixmap texturePixmap = QPixmap(fileName);
    mTextureWidget->addTexture(texturePixmap, fileName, thumbName);
}
