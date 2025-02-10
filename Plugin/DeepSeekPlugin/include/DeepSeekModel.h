#pragma once
#include <DBModel.h>
#include "DeepSeekPlugin_global.h"

class DeepSeekPlugin_EXPORT DeepSeekModel : public DBModel
{
    Q_OBJECT
public:
    DeepSeekModel();
    ~DeepSeekModel();

public:
    ADD_PROPERTY(QString, identifier)
    ADD_PROPERTY(QString, datetime)
    ADD_PROPERTY(QString, chat_name)
    ADD_PROPERTY(bool, isLegal)
    ADD_PROPERTY(QString, content)
};