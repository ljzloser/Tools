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
    ADD_TEXT_PROPERTY(identifier)
    ADD_TEXT_PROPERTY(datetime)
    ADD_TEXT_PROPERTY(chat_name)
    ADD_INTEGER_PROPERTY(isLegal)
    ADD_TEXT_PROPERTY(content)
};