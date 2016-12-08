#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "qstringlist.h"
#include "qfile.h"
#include "qdebug.h"


#define FILEPATH "/home/pi/test/config.txt"

#define KEYWORD_PLAYBACK_LIST "PLAYBACK_DEVICES"
#define KEYWORD_CAPTURE_LIST "CAPTURE_DEVICES"
#define KEYWORD_CLICK_PORTS "CLICK_PORTS"
#define KEYWORD_MIX_STRATEGY "MIX_STRATEGY"
#define MIX_STRATEGY_AUTO "AUTO"
#define MIX_STRATEGY_PRESET "PRESET"

#define KEYWORD_MIX_PRESET_NUMBER "MIX_PRESET_NUMBER"






void extractParameter(QString keyword, QStringList *value);
void setParameter(QString keyword, QStringList parameter, bool reset);




#endif // CONFIG_FILE_H
