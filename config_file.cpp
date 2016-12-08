#include "config_file.h"



void extractParameter(QString keyword, QStringList *value)
{

    QFile file(FILEPATH);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH<<"opening error";
        return;
    }


    QTextStream in (&file);
    QString line;
    QString buf;

    bool keyWordFound = false;

    do {
        line = in.readLine();
        if (line.contains(keyword, Qt::CaseSensitive)) {
            if(!keyWordFound)
            {
                buf.append(line+"\n");


                keyWordFound = true;
                line = line.remove(keyword);
                line = line.remove(" ");//remove all spaces
                QStringList value2=line.split(';', QString::SkipEmptyParts);
                *value=value2.toSet().toList();//remove duplicates



            }
            else
            {
                qDebug()<<keyword<<"extra entries have been removed, check configuration";

            }
        }
        else
        {
            if(line!="")
            buf.append(line+"\n");
        }
    } while (!line.isNull());


    if(!keyWordFound) qDebug()<<"no"<<keyword<<"set in file"<<FILEPATH;


    //clean up the file
    file.resize(0);

    in<<buf;

    file.close();



}


void setParameter(QString keyword, QStringList parameter, bool reset)
{


    QFile file(FILEPATH);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH<<"opening error";
        return;
    }


    QTextStream in (&file);
    QString line;
    QString buf;
    QStringList value;

    bool keyWordFound = false;

    do {
        line = in.readLine();
        if (line.contains(keyword, Qt::CaseSensitive)) {
            if(!keyWordFound)
            {



                keyWordFound = true;
                line = line.remove(keyword);
                line = line.remove(" ");//remove all spaces
                value=line.split(';', QString::SkipEmptyParts);

                if(!reset)
                    value.append(parameter);
                else
                    value = parameter;


                value=value.toSet().toList();//remove duplicates

                buf.append(keyword+" "+value.join(";")+"\n");


            }
            else
            {
                qDebug()<<keyword<<"extra entries have been removed, check configuration";

            }
        }
        else
        {
            if(line!="")
            buf.append(line+"\n");
        }
    } while (!line.isNull());


    if(!keyWordFound)
    {
       parameter = parameter.toSet().toList();
       buf.append(keyword+" "+parameter.join(";")+"\n");
//       qDebug()<<"not found";
    }



    //clean up the file
    file.resize(0);

    in<<buf;

    file.close();


}


