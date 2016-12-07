#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "qstringlist.h"
#include "qfile.h"
#include "qdebug.h"


#define FILEPATH "/home/pi/test/config.txt"

#define KEYWORD_PLAYBACK_LIST "PLAYBACK_DEVICES"
#define KEYWORD_CAPTURE_LIST "CAPTURE_DEVICES"

void extractParameter(QString keyword, QStringList *value);
void setParameter(QString keyword, QStringList parameter, bool reset);




#endif // CONFIG_FILE_H
