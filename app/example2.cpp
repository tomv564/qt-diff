
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
#include "edbee/models/textdocument.h"
#include "edbee/models/textgrammar.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/texttheme.h"
//#include "diff_match_patch.h"

typedef diff_match_patch<string> stringdiff;

string getFileContents(const char* path)
{
    ifstream myfile(path);
    if (myfile.is_open())
    {
        stringstream buffer;
        buffer << myfile.rdbuf();
        
        // TODO: who deallocates buffer and myfile?
        return buffer.str();
    }
    else qDebug() << "Unable to open file" << path;
    return NULL;
}

void summarizeLines(QVector<QVector<stringdiff::Diff*>> lookup) {
    
    for (int line = 0; line < lookup.size(); ++line) {
        
        QVector<stringdiff::Diff*> diffs = lookup.at(line);
        
        
            for (int i = 0; i < diffs.size(); ++i) {
                stringdiff::Diff* diff = diffs.at(i);
                if (diff != NULL) {
                    qDebug() << "line" << line << "diff" << i << "op" << (*diff).operation;
                }
            }
        
        
    }
}

QVector<QVector<stringdiff::Diff*>> createDiffLookup(list<stringdiff::Diff> diffs, stringdiff::Operation operation) {
    int lineNumber = 0;
    
    // we want pointers to the diffs, not the original object.
    QVector<QVector<stringdiff::Diff*>> diffsPerLine(100, QVector<stringdiff::Diff*>());
    qDebug() << "Diff lookup for operation" << operation;
    
    // iterator over each diff
    for (list<stringdiff::Diff>::iterator it=diffs.begin(); it != diffs.end(); ++it)
    {
        qDebug() << it->text.c_str();
        bool isRelevant = it->operation == stringdiff::EQUAL || it->operation == operation; // no change or delete/insert
        int lineCount = (int) count(it->text.begin(), it->text.end(), '\n');
        qDebug() << lineCount+1 << "lines of" << it->operation << ", relevant is" << isRelevant;

        if (isRelevant) {
            for (int lineOffset = 0; lineOffset <= lineCount; ++lineOffset) {
                int currentLine = lineNumber + lineOffset;
                
//                qDebug() << "Trying line" << currentLine;
                stringdiff::Diff* diff = &*it;
                diffsPerLine[currentLine].append(diff);
                qDebug() << "appended" << diff->operation;
//                QVector<stringdiff::Diff*> diffsAtLine = diffsPerLine.at(currentLine);
//                diffsAtLine.append(diff);
//                qDebug() << diffsAtLine;
//
                // dereference and grab address of iterator.
                
                
////                auto diffsAtLine = diffsPerLine->at(currentLine
//                
//                //            int currentLine = lineNumber + lineOffset;
//                //            if (diffsPerLine[currentLine] == NULL) { diffsPerLine[currentLine]
//                qDebug() << "Line " << lineNumber + lineOffset << " change " << it->operation;
            }
            lineNumber += lineCount;
        }
    }
    
    return diffsPerLine;
    
}

bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}


/// This is an example that shows how to load a grammar file and a theme file
/// manually and initialise the editor with it
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // initialize edbee
    edbee::Edbee* edbee = edbee::Edbee::instance();
    edbee->autoInit();
    
    // TODO: Check the edbee app for a proper way to detect grammars and themes.

    // read the grammar file
    edbee::TextGrammarManager* grammarManager = edbee->grammarManager();
    edbee::TextGrammar* grammar = grammarManager->readGrammarFile( "HTML.tmLanguage" );
    if( !grammar ) {
        qDebug() << "Grammar couldn't be loaded: " << grammarManager->lastErrorMessage();
        edbee->shutdown(); // call this method manuall to prevent memory leak warnings. When the exec method is called this isn't required
        return 1;
    }

    // read the theme file
    edbee::TextThemeManager* themeManager = edbee->themeManager();
    edbee::TextTheme* theme = themeManager->readThemeFile( "Solarized (Dark).tmTheme" );
    if( !theme) {
        qDebug() << "Theme couldn't be loaded: " << themeManager->lastErrorMessage();
        edbee->shutdown(); // call this method manuall to prevent memory leak warnings. When the exec method is called this isn't required
        return 1;
    }

//    string leftContent = "<html>\n\t<title>Yo</title>\n\t<body>\n\t\t<h1>Hello Left</h1>\n\t</body>\n</html>\n";
//    string rightContent = "<html>\n\t<body>\n\t\t<h1>Hello Right</h1>\n\t</body>\n\t<footer>fin</footer>\n</html>\n";
//    
    string leftContent = "asdf";
    string rightContent = "asdf";
    
//    if (argc > 2) {
//        
//        char* file1 = argv[1];
//        char* file2 = argv[2];
//  
//        if (fileExists(file1)) {
//            leftContent = getFileContents(file1);
//        }
//        
//        if (fileExists(file2)) {
//            rightContent = getFileContents(file2);
//        }
//        
//    }
   
    // get the minimal diff
    diff_match_patch<string> dmp;
    auto diffs = dmp.diff_main(leftContent, rightContent);
    
    auto deletionLookup = createDiffLookup(diffs, stringdiff::DELETE);
    summarizeLines(deletionLookup);
    
    
    auto insertLookup = createDiffLookup(diffs, stringdiff::INSERT);
    summarizeLines(insertLookup);
//
//    auto deletions = findDiffsByLine(diffs, stringdiff::DELETE);
//    auto insertions = findDiffsByLine(diffs, stringdiff::INSERT);
    
//    for (int i = 0; i < deletions.size(); ++i) {
//        qDebug() << deletions[i] << " ";
//    }
//    

    // TODO: generate a diff object to be used by both.
    /*
     
     Options:
     
     Set a unified diff to both sides, with left interpreting differently from right.
     * left: '-' means red
     * right: '+' means green
     * both: '-' followed by '+' means change
     
     We need support for:
     
     * Shading of line number columns?
     * Scrolling in a sane way?
     * Detecting inline changes.
     
     */
    
    // TODO: read only?
    edbee::TextEditorWidget left;
    left.textDocument()->setLanguageGrammar( grammar );
    left.textRenderer()->setTheme( theme );
    left.textDocument()->setDiffLookup(deletionLookup);
    left.textDocument()->setText(QString::fromStdString(leftContent));
//    left.textDocument()->setDiffStatus(&deletions);
    
    edbee::TextEditorWidget right;
    right.textDocument()->setLanguageGrammar( grammar );
    right.textRenderer()->setTheme( theme );
    right.textDocument()->setDiffLookup(insertLookup);
    right.textDocument()->setText(QString::fromStdString(rightContent));
//    right.textDocument()->setDiffStatus(&insertions);
    
    QSplitter *splitter = new QSplitter();
    splitter->addWidget(&left);
    splitter->addWidget(&right);

    // show the window
    // next create the main window and the editor
    QMainWindow win;
    win.setCentralWidget( splitter );
    win.show();
    return a.exec();
}
