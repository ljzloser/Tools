#pragma once
#include <DBModel.h>
#include "TemplatePlugin_global.h"

class TemplatePlugin_EXPORT TemplateModel : public DBModel
{
    Q_OBJECT
public:
    TemplateModel();
    ~TemplateModel();

public:
    /*
    * 使用 ADD_TEXT_PROPERTY() 添加文本属性
    * 使用 ADD_INT_PROPERTY() 添加整数属性
    * 使用 ADD_REAL_PROPERTY() 添加实数属性
    * 使用 ADD_BLOB_PROPERTY() 添加二进制属性
    * 使用 DBModelHelper类可批量操作DBModel,详见DBModelHpler类的函数声明
    */
    //ADD_TEXT_PROPERTY(identifier)
};