#pragma once

#include <QFile>
#include <QTextSTream>

#include <string>
#include <map>

using namespace std;

class MapReader
{
public:
    template<class Key, class Value>
    static size_t ReadTabFile(
            const string filename,  map<Key, Value> outmap,
            std::function<Key(const Key&)> keyModifier = [](const Key& key) -> Key{return key; },
            std::function<Value(const Value&)> valueModifier = [](const Value& value) -> Value{return value; })
    {
        size_t lines=0;
        Q_INIT_RESOURCE(klive);

        QFile file(filename .c_str());
        if (file.open(QFile::ReadOnly))
        {
            QTextStream stream(&file);
            while(!stream.atEnd())
            {

                string line = stream.readLine().toStdString();

                auto pos = line.find('\t');

                if (pos != string::npos)
                {
                    string key=keyModifier(line.substr(0,pos));
                    string value=valueModifier(line.substr(pos+1));

                    if (key.length() &&
                            key[0] != '/' &&
                            key[0] != '#')
                    {
                        if (outmap.insert({key, value}).second)
                            lines++;
                    }
                }
            }
        }
        file.close();
        return lines;
    }
};

