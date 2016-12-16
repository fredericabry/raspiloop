#include "config_file.h"



void extractParameter(QString keyword, QStringList *value)
{

    QFile file(FILEPATH_CONFIG);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONFIG<<"opening error";
        return;
    }


    QTextStream in (&file);
    QString line;
    QString buf;

    value->clear();

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


    if(!keyWordFound) qDebug()<<"no"<<keyword<<"set in file"<<FILEPATH_CONFIG;


    //clean up the file
    file.resize(0);

    in<<buf;

    file.close();



}

void setParameter(QString keyword, QStringList parameter, bool reset)
{


    QFile file(FILEPATH_CONFIG);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONFIG<<"opening error";
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


//define a control in file
void fileSetControl(QString keyword, QStringList controls)
{

    fileRemoveControl(keyword);

    QString buf;
    QFile file(FILEPATH_CONTROL);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONTROL<<"opening error";
        return;
    }

    QTextStream in (&file);
    buf=in.readAll();

    //character # is added to avoid ambiguity
    buf.append(keyword+"\n"+controls.join("\n")+"\n"+KEYWORD_CONTROL_END);

    //clean up the file
    file.resize(0);

    in<<buf;

    file.close();


}


//remove one control already defined
void fileRemoveControl(QString keyword)
{


    QFile file(FILEPATH_CONTROL);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONTROL<<"opening error";
        return;
    }


    QTextStream in (&file);
    QString line;
    QString buf;
    bool keyWordFound = false;

    bool deleteLine = false;//if keyword has been found, is true until KEYWORD_CONTROL_END is reached

    do {
        line = in.readLine();
        if (line.contains(keyword, Qt::CaseSensitive))
        {
            keyWordFound = true;
            deleteLine = true;//all line will be deleted until END is reached
        }
        else if(deleteLine)
        {
            if(line.contains(KEYWORD_CONTROL_END, Qt::CaseSensitive)) deleteLine = false;
        }
        else
        {
            if(line!="")
                buf.append(line+"\n");
        }
    } while (!line.isNull());


    if(!keyWordFound)
    {

             // qDebug()<<"not found";
    }

    //clean up the file
    file.resize(0);

    in<<buf;

    file.close();
}


//return the control binded to a keyword
void fileGetControl(QString keyword, QStringList *control)
{

    QFile file(FILEPATH_CONTROL);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONTROL<<"opening error";
        return;
    }

    control->clear();

    QTextStream in (&file);
    QString line;

    bool keyWordFound = false;

    do {
        line = in.readLine();
        if (line.contains(keyword, Qt::CaseSensitive))
        {
            keyWordFound = true;

        }
        else if(keyWordFound)
        {
            if(line.contains(KEYWORD_CONTROL_END, Qt::CaseSensitive)) break;
            else
            {
                //we are between keyword and KEYWORD_CONTROL_END, let us copy
                if(line!="")

                control->append(line);
            }
        }
        else
        {

        }
    } while (!line.isNull());


    if(!keyWordFound)
    {

            qDebug()<<"control "+keyword+" not found in file";
    }


    file.close();
}


//return the list of every key defined in file
void fileGetControlKeyList(QStringList *keyControl) // return all the keywords
{
    QFile file(FILEPATH_CONTROL);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONTROL<<"opening error";
        return;
    }

    keyControl->clear();

    QTextStream in (&file);
    QString line;


    do {
        line = in.readLine();
        if ((line.contains("KEY_", Qt::CaseSensitive))
                ||(line.contains("MIDI_", Qt::CaseSensitive)))
        {
         keyControl->append(line);
        }
    } while (!line.isNull());

    file.close();
}


//check if control is defined in file
bool fileIsControlDefined(QString keyword)
{

    QFile file(FILEPATH_CONTROL);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"file"<<FILEPATH_CONTROL<<"opening error";
        return false;
    }

    QTextStream in (&file);
    QString line;



    do {
        line = in.readLine();
        if (line.contains(keyword, Qt::CaseSensitive))
        {
            file.close();
           return true;

        }


    } while (!line.isNull());





    file.close();
    return false;

}
