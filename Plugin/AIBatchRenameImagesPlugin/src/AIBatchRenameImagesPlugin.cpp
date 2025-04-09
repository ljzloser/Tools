#include "AIBatchRenameImagesPlugin.h"
#include "AIBatchRenameImagesPluginWidget.h"
#include <QStandardPaths>

AIBatchRenameImagesPlugin::AIBatchRenameImagesPlugin(Logger *logger, TConfig *config, QObject *parent)
    : AbstractPlugin(logger, config, parent)
{
}

AIBatchRenameImagesPlugin::~AIBatchRenameImagesPlugin()
{
}

/**
 * @brief 插件组名称
 * @return
 */
QString AIBatchRenameImagesPlugin::group()
{
    return "图像处理";
}

/**
 * @brief  插件名称
 * @return
 */
QString AIBatchRenameImagesPlugin::name()
{
    return "AI批量图片重命名";
}

/**
 * @brief  插件版本
 * @return
 */
QString AIBatchRenameImagesPlugin::version()
{
    return "1.0.0";
}

/**
 * @brief  插件作者
 * @return
 */
QString AIBatchRenameImagesPlugin::author()
{
    return "ljzloser";
}

/**
 * @brief  插件描述
 * @return
 */
QString AIBatchRenameImagesPlugin::description()
{
    return "使用AI批量图片重命名";
}

/**
 * @brief  插件图标
 * @return
 */
QIcon AIBatchRenameImagesPlugin::icon()
{
    return QIcon(":res/icon/AIBatchRenameImagesPlugin.png");
}

/**
 * @brief 启动插件
 * @return QWidget
 */
QWidget *AIBatchRenameImagesPlugin::start()
{
    AbstractPlugin::start();
    return new AIBatchRenameImagesWidget(_logger, _config);
}

/**
 * @brief 停止插件
 */
void AIBatchRenameImagesPlugin::stop()
{
    AbstractPlugin::stop();
}

/**
 * @brief 在写入配置文件前,在这里可以做一些校验操作,或者因为某些原因不允许修改
 * @param WriteConfigEvent
 * @details
 */
void AIBatchRenameImagesPlugin::writeConfigBeforeEvent(WriteConfigEvent &event)
{
    AbstractPlugin::writeConfigBeforeEvent(event);
}

/**
 * @brief  写入配置文件后，在这里可以做一些因为配置文件修改导致的操作
 * 如果event.isValid为false，那么不会触发该函数，界面也会回滚到原来的值
 * @param WriteConfigEvent
 */
void AIBatchRenameImagesPlugin::writeConfigAfterEvent(WriteConfigEvent &event)
{
    AbstractPlugin::writeConfigAfterEvent(event);
}

/**
 * @brief 读取配置文件前,在这里可以做一些校验操作,比如你可以修改要读取的key
 * @param ReadConfigEvent
 * @details
 */
void AIBatchRenameImagesPlugin::readConfigBeforeEvent(ReadConfigEvent &event)
{
    AbstractPlugin::readConfigBeforeEvent(event);
}

/**
 * @brief 读取配置文件后，比如你可以修改要展示的值
 * @param ReadConfigEvent
 * @details
 */
void AIBatchRenameImagesPlugin::readConfigAfterEvent(ReadConfigEvent &event)
{
    AbstractPlugin::readConfigAfterEvent(event);
}

/**
 * @brief 该函数用于注册插件的配置项
 * @details  在该函数中，需要使用CONFIG_REGISTER宏来注册插件的配置项
 *          CONFIG_REGISTER的第一个参数是配置项的Key，第二个参数是配置项的描述，
 *          第三个参数是配置项的类型，第四个参数是配置项的默认值，第五个参数
 *          是一个bool值，表示该配置项是否可以在界面上显示
 *          或使用指定类型的宏来注册 示例
 *          Int_CONFIG_REGISTER("Int", "大小", 1, true);
 */
void AIBatchRenameImagesPlugin::registerConfig()
{
    AbstractPlugin::registerConfig();
}
