#include "DeepSeekPlugin.h"
#include "DeepSeekPluginWidget.h"
#include <QStandardPaths>

DeepSeekPlugin::DeepSeekPlugin(Logger *logger, TConfig *config, QObject *parent)
    : AbstractPlugin(logger, config, parent)
{
}

DeepSeekPlugin::~DeepSeekPlugin()
{
}

/**
 * @brief 插件组名称
 * @return
 */
QString DeepSeekPlugin::group()
{
    return "AI";
}

/**
 * @brief  插件名称
 * @return
 */
QString DeepSeekPlugin::name()
{
    return "DeepSeek";
}

/**
 * @brief  插件版本
 * @return
 */
QString DeepSeekPlugin::version()
{
    return "1.0";
}

/**
 * @brief  插件作者
 * @return
 */
QString DeepSeekPlugin::author()
{
    return "author";
}

/**
 * @brief  插件描述
 * @return
 */
QString DeepSeekPlugin::description()
{
    return "description";
}

/**
 * @brief  插件图标
 * @return
 */
QIcon DeepSeekPlugin::icon()
{
    return QIcon(":res/icon/DeepSeekPlugin.png");
}

/**
 * @brief 启动插件
 * @return QWidget
 */
QWidget *DeepSeekPlugin::start()
{
    AbstractPlugin::start();
    _widget = new DeepSeekWidget(_logger, _config);
    return _widget;
}

/**
 * @brief 停止插件
 */
void DeepSeekPlugin::stop()
{
    AbstractPlugin::stop();
}

/**
 * @brief 在写入配置文件前,在这里可以做一些校验操作,或者因为某些原因不允许修改
 * @param WriteConfigEvent
 * @details
 */
void DeepSeekPlugin::writeConfigBeforeEvent(WriteConfigEvent &event)
{
    AbstractPlugin::writeConfigBeforeEvent(event);
    auto key = event.key;
    auto value = event.newValue();
    if (key == "top_p")
    {
        if (!(DeepSeek::verifyTopP(value.toDouble())))
            event.cancel("top_p值必须在0到1之间");
    }
    else if (key == "temperature")
    {
        if (!(DeepSeek::verifyTemperature(value.toDouble())))
            event.cancel("temperature值必须在0到2之间");
    }
    else if (key == "max_tokens")
    {
        if (!(DeepSeek::verifyMaxTokens(value.toInt())))
            event.cancel("max_tokens值必须大于0 小于8192");
    }
    else if (key == "frequency_penalty")
    {
        if (!(DeepSeek::verifyFrequencyPenalty(value.toDouble())))
            event.cancel("frequency_penalty值必须在-2到2之间");
    }
    else if (key == "presence_penalty")
    {
        if (!(DeepSeek::verifyPresencePenalty(value.toDouble())))
            event.cancel("presence_penalty值必须在-2到2之间");
    }
    else if (key == "blance_update_interval")
    {
        int interval = value.toInt();
        if (interval < 60 || interval > 3600)
            event.cancel("余额更新时间必须大于60小于3600");
    }
}

/**
 * @brief  写入配置文件后，在这里可以做一些因为配置文件修改导致的操作
 * 如果event.isValid为false，那么不会触发该函数，界面也会回滚到原来的值
 * @param WriteConfigEvent
 */
void DeepSeekPlugin::writeConfigAfterEvent(WriteConfigEvent &event)
{
    AbstractPlugin::writeConfigAfterEvent(event);
    if (_widget == nullptr)
        return;
    _widget->setParmas(event.key, event.newValue());
}

/**
 * @brief 读取配置文件前,在这里可以做一些校验操作,比如你可以修改要读取的key
 * @param ReadConfigEvent
 * @details
 */
void DeepSeekPlugin::readConfigBeforeEvent(ReadConfigEvent &event)
{
    AbstractPlugin::readConfigBeforeEvent(event);
}

/**
 * @brief 读取配置文件后，比如你可以修改要展示的值
 * @param ReadConfigEvent
 * @details
 */
void DeepSeekPlugin::readConfigAfterEvent(ReadConfigEvent &event)
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
void DeepSeekPlugin::registerConfig()
{
    AbstractPlugin::registerConfig();
    String_CONFIG_REGISTER("token", "Api-key", "", true);
    Double_CONFIG_REGISTER("top_p", "top_p", 1, 1);
    Double_CONFIG_REGISTER("temperature", "temperature", 1, 1);
    Int_CONFIG_REGISTER("max_tokens", "max_tokens", 4096, 1);
    Double_CONFIG_REGISTER("frequency_penalty", "frequency_penalty", 0, 1);
    Bool_CONFIG_REGISTER("isStream", "是否流式输出", 1, 1);
    ComBox_CONFIG_REGISTER("model", "默认模型",
                           ComboxData(0, {"deepseek-chat", "deepseek-reasoner"}), 1);

    Double_CONFIG_REGISTER("presencePenalty", "presencePenalty", 0, 1);
    String_CONFIG_REGISTER("system_messages", "系统提示词", "You are a helpful assistant", 1);
    ComBox_CONFIG_REGISTER("seed_key", "发送快捷键", ComboxData(1, {"Enter", "Ctrl+Enter"}), 0);
    Int_CONFIG_REGISTER("blance_update_interval", "余额更新间隔", 60, 1);
}
