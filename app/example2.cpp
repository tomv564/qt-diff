
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMainWindow>
#include <QSplitter>
#include <QFileInfo>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

#include "edbee/edbee.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textgrammar.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/texttheme.h"
#include "edbee/models/texteditorconfig.h"

//#include "diff_match_patch.h"

typedef diff_match_patch<string> stringdiff;

string getFileContents(string path)
{
    ifstream myfile(path);
    if (myfile.is_open())
    {
        stringstream buffer;
        buffer << myfile.rdbuf();
        
        // TODO: who deallocates buffer and myfile?
        return buffer.str();
    }
    else qDebug() << "Unable to open file" << path.c_str();
    return NULL;
}

void summarizeLines(QVector<QVector<stringdiff::Diff>> lookup) {
    
    for (int line = 0; line < lookup.count(); ++line) {
        
        QVector<stringdiff::Diff> diffs = lookup.at(line);

        for (int i = 0; i < diffs.count(); ++i) {
            stringdiff::Diff diff = diffs.at(i);
            qDebug() << "line" << line << "diff" << i << "op" << diff.operation;
        }
        
    }
}

QVector<QVector<stringdiff::Diff>> createDiffLookup(list<stringdiff::Diff> diffs, stringdiff::Operation operation) {
    int lineIndex = 0;
    
    // we want pointers to the diffs, not the original object.
	QVector<QVector<stringdiff::Diff>> diffsPerLine;
	diffsPerLine.append(QVector<stringdiff::Diff>());
    qDebug() << "Diff lookup for operation" << operation;
    
    // iterator over each diff
    for (list<stringdiff::Diff>::iterator it=diffs.begin(); it != diffs.end(); ++it)
    {
		qDebug() << it->text.c_str();
        bool isRelevant = it->operation == stringdiff::EQUAL || it->operation == operation; // no change or delete/insert
        int lineCount = (int) count(it->text.begin(), it->text.end(), '\n');
        //qDebug() << it->operation << ", relevant is" << isRelevant;
		
		if (isRelevant) {
			stringdiff::Diff diff = *it;
		
			if (lineCount == 0)
			{
				diffsPerLine[lineIndex].append(diff);
			}
			else 
			{
				for (int offset = 0; offset < lineCount; ++offset)
				{
					diffsPerLine[lineIndex].append(diff);
					diffsPerLine.append(QVector<stringdiff::Diff>());
					lineIndex = lineIndex + 1;
					qDebug() << "line" << lineIndex << it->strOperation(it->operation).c_str();
				}
			}
						
		}
    }
    
    return diffsPerLine;
    
}

bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	QString appDataPath_;
#ifdef Q_OS_MAC
	appDataPath_ = applicationDirPath() + "/../Resources/";
#else
	appDataPath_ = qApp->applicationDirPath() + "/data/";
#endif

    // initialize edbee
    edbee::Edbee* edbee = edbee::Edbee::instance();
	edbee->setKeyMapPath(QString("%1%2").arg(appDataPath_).arg("keymaps"));
	edbee->setGrammarPath(QString("%1%2").arg(appDataPath_).arg("syntaxfiles"));
	edbee->setThemePath(QString("%1%2").arg(appDataPath_).arg("themes"));
    edbee->autoInit();

	//string leftContent = "<html>\n\t<title>Yo</title>\n\t<body>\n\t\t<h1>Hello Left</h1>\n\t</body>\n</html>\n";
	//string rightContent = "<html>\n\t<body>\n\t\t<h1>Hello Right</h1>\n\t</body>\n\t<footer>fin</footer>\n</html>\n";

	//string leftContent = "Mary had a little lamb,\nwhose fleece was white as snow.\n\n\nIt followed her to school one day,\nschool one day, school one day,\nwhich was against the rules.\n\n";
	//string rightContent = "Mary had a little lamb,\nwhose fleece was red as snow.\n\n\And everywhere that Mary went,\nthe lamb was sure to go.\n\nIt followed her to school one day,\nwhich was against the rules.\n\n";

	string leftContent = "Mary had a little lamb,\nwhose fleece was white as snow.\n\n";
	string rightContent = "Mary had a little lamb,\nwhose fleece was red as snow.\n\nAnd everywhere that Mary went,\nthe lamb was sure to go.\n";

	string leftFile = "left.txt";
	string rightFile = "right.txt";

    if (argc > 2) {
        
		leftFile = argv[1];
		rightFile = argv[2];

        if (fileExists(QString::fromStdString(leftFile))) {
            leftContent = getFileContents(leftFile);
        }
        
        if (fileExists(QString::fromStdString(rightFile))) {
            rightContent = getFileContents(rightFile);
        }
        
    }
   
    diff_match_patch<string> dmp;
/*
    auto diffs = dmp.diff_main(leftContent, rightContent);
    auto deletionLookup = createDiffLookup(diffs, stringdiff::DELETE);
    auto insertLookup = createDiffLookup(diffs, stringdiff::INSERT);
*/

	auto diffs = dmp.diff_lines(leftContent, rightContent);
	auto deletionLookup = createDiffLookup(diffs, stringdiff::DELETE);
	auto insertLookup = createDiffLookup(diffs, stringdiff::INSERT);

	/*

     We need support for:
     
     * Shading of line number columns?
     * Scrolling in a sane way?
     * Detecting inline changes.
     
     */
    
	QFont font = QFont("Consolas", 12);
    // TODO: read only?
    edbee::TextEditorWidget left;
	left.config()->setFont(font);
	left.config()->setThemeName("Oceanic Next");
	left.textDocument()->setLanguageGrammar(edbee::Edbee::instance()->grammarManager()->detectGrammarWithFilename(QString::fromStdString(leftFile)));
    left.textDocument()->setText(QString::fromStdString(leftContent));
	left.textDocument()->setDiffLookup(deletionLookup);

    edbee::TextEditorWidget right;
	right.config()->setFont(font);
	right.config()->setThemeName("Oceanic Next");
	right.textDocument()->setLanguageGrammar(edbee::Edbee::instance()->grammarManager()->detectGrammarWithFilename(QString::fromStdString(rightFile)));
    right.textDocument()->setText(QString::fromStdString(rightContent));
	right.textDocument()->setDiffLookup(insertLookup);


    
    QSplitter *splitter = new QSplitter();
    splitter->addWidget(&left);
    splitter->addWidget(&right);

    // show the window
    // next create the main window and the editor
    QMainWindow win;
	win.setMinimumSize(800, 600);
    win.setCentralWidget( splitter );
	win.setWindowTitle(QString::fromStdString(rightFile));
	win.show();

	// scroll to first change
	int leftOffset = deletionLookup[0][0].text.length();
	left.controller()->scrollOffsetVisible(leftOffset);
	int rightOffset = insertLookup[0][0].text.length();
	right.controller()->scrollOffsetVisible(rightOffset);
    
	return a.exec();
}
