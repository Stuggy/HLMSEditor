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

#ifndef BRUSH_WIDGET_H
#define BRUSH_WIDGET_H

#include <QWidget>
#include "tool_default_texturewidget.h"

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class BrushDockWidget;

/****************************************************************************
Main class for brush widget. This widgets displays brushes.
***************************************************************************/
class BrushWidget : public QWidget
{
    Q_OBJECT

    public:
        BrushWidget(const QString& brushDir, BrushDockWidget* brushDockWidget, QWidget* parent = 0);
        ~BrushWidget(void);

    signals:
        void brushDoubleClicked(const QString& name, const QString& baseName);

    private slots:
        void handleBrushDoubleClicked(const QString& name, const QString& baseName);

    protected:
        void loadBrushesRecursively(const QString& searchPath);

    private:
        BrushDockWidget* mBrushDockWidget; // Reference to the dockwidget that contains this widget
        Magus::QtDefaultTextureWidget* mTextureWidget;
        QWidget* mParent;
        QString mBrushDir;
};

#endif
