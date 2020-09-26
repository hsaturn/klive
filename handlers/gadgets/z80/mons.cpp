#include "mons.h"

#include <string>

#include <QFile>
#include <QTextSTream>

using namespace std;

Mons::Mons()
{
    Q_INIT_RESOURCE(klive);

    QFile file(":/z80.txt");
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);

        string line = stream.readLine().toStdString();

        auto pos = line.find('\t');

        if (pos != string::npos)
        {
            string left=line.substr(0,pos);
            string right=line.substr(pos+1);

            if (left.length() &&
                    left[0] != '/' &&
                    left[0] != '#')
            {
                opcodes[left] = right;
            }
        }
    }
    file.close();

}
