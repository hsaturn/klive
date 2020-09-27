#pragma once

#include <QFile>
#include <QTextStream>

#include <string>
#include <map>

using namespace std;

class MapReader
{
public:

    template<class Key, class Value>
    static size_t ReadTabFile(
            const string filename,  map<Key, Value>& outmap,
            void keyValueModifier(long line, Key&, Value& value) = [](long line, Key& key, Value& value) {})
    {
        size_t inserted=0;
        size_t line_nr=0;

        QFile file(filename .c_str());
        if (file.open(QFile::ReadOnly))
        {
            QTextStream stream(&file);
            while(!stream.atEnd())
            {
                line_nr++;
                string line = stream.readLine().toStdString();

                auto pos = line.find('\t');

                if (pos != string::npos)
                {
                    string key=line.substr(0,pos);
                    string value=line.substr(pos+1);
                    keyValueModifier(line_nr, key, value);

                    if (key.length() &&
                            key[0] != '/' &&
                            key[0] != '#')
                    {
                        if (outmap.insert({key, value}).second)
                            inserted++;
                    }
                }
            }
        }
        file.close();
        return inserted;
    }
};

