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

#include <QApplication>
#include <QThread>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
	/*Help Qt find the "./platforms/qwindows.dll" library a little bit, by making it look next to the executable first.
	 *
	 *From Qt documentation :
	 *
	 *  The default path list consists of a single entry,
	 *  the installation directory for plugins. The default
	 *  installation directory for plugins is INSTALL/plugins,
	 *  where INSTALL is the directory where Qt was installed.
	 */
	QCoreApplication::addLibraryPath(".");
#endif
	QApplication app(argc, argv);
	MainWindow mainWin;
	mainWin.show();
	while (!mainWin.mIsClosing)
	{
		app.processEvents();
		mainWin.update();
		QThread::msleep(10);
	}
	return 0;
}