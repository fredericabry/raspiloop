#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "qstringlist.h"
#include "qfile.h"
#include "qdebug.h"


#define FILEPATH_CONFIG "/home/pi/test/config.txt"
#define FILEPATH_CONTROL "/home/pi/test/control.txt"

#define KEYWORD_PLAYBACK_LIST "PLAYBACK_DEVICES"
#define KEYWORD_CAPTURE_LIST "CAPTURE_DEVICES"
#define KEYWORD_CLICK_PORTS "CLICK_PORTS"
#define KEYWORD_MIX_STRATEGY "MIX_STRATEGY"
#define MIX_STRATEGY_AUTO "AUTO"
#define MIX_STRATEGY_PRESET "PRESET"

#define KEYWORD_MIX_PRESET_NUMBER "MIX_PRESET_NUMBER"

#define KEYWORD_CONTROL_END "CONTROL_END"





void extractParameter(QString keyword, QStringList *value);
void setParameter(QString keyword, QStringList parameter, bool reset);

void fileSetControl(QString keyword, QStringList parameter);
void fileRemoveControl(QString keyword);
void fileGetControl(QString keyword, QStringList *control);
bool fileIsControlDefined(QString keyword);
#endif // CONFIG_FILE_H
