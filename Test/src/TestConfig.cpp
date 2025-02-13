#include "TestConfig.h"
#include <iostream>
#include <windows.h>
void TestConfig::registerConfig()
{
    TConfig *config = new TConfig("config.ini");
    config->registerConfig("DateTime", "名称", TConfig::Type::DateTime, QDateTime::fromString("2020-01-01 00:00:00", "yyyy-MM-dd hh:mm:ss"));
    config->registerConfig("Color", "名称", TConfig::Type::Color, QColor(255, 0, 0));
    config->registerConfig("File", "名称", TConfig::Type::File, "D:\\test.txt");
    config->registerConfig("String", "名称", TConfig::Type::String, "test");
    config->registerConfig("Int", "名称", TConfig::Type::Int, 1);
    config->registerConfig("Bool", "名称", TConfig::Type::Bool, true);
    config->registerConfig("Float", "名称", TConfig::Type::Float, 1.1f);
    config->registerConfig("Double", "名称", TConfig::Type::Double, 1.1);
    config->registerConfig("Directory", "名称", TConfig::Type::Directory, "D:\\test");
    config->registerConfig("Date", "名称", TConfig::Type::Date, QDate::fromString("2020-01-01", "yyyy-MM-dd"));
    config->registerConfig("Time", "名称", TConfig::Type::Time, QTime::fromString("00:00:00", "hh:mm:ss"));

    // 判断读取后是否和预期一致
    QCOMPARE(config->read("DateTime").value.toDateTime(), QDateTime::fromString("2020-01-01 00:00:00", "yyyy-MM-dd hh:mm:ss"));
    QCOMPARE(config->read("Color").value.value<QColor>(), QColor(255, 0, 0));
    QCOMPARE(config->read("File").value.toString(), "D:\\test.txt");
    QCOMPARE(config->read("String").value.toString(), "test");
    QCOMPARE(config->read("Int").value.toInt(), 1);
    QCOMPARE(config->read("Bool").value.toBool(), true);
    QCOMPARE(config->read("Float").value.toFloat(), 1.1f);
    QCOMPARE(config->read("Double").value.toDouble(), 1.1);
    QCOMPARE(config->read("Directory").value.toString(), "D:\\test");
    QCOMPARE(config->read("Date").value.toDate(), QDate::fromString("2020-01-01", "yyyy-MM-dd"));
    QCOMPARE(config->read("Time").value.toTime(), QTime::fromString("00:00:00", "hh:mm:ss"));
}

void TestConfig::writeConfig()
{

    TConfig *config = new TConfig("config.ini");

    // 1. 注册初始配置，确保有数据可测试
    config->registerConfig("DateTime", "名称", TConfig::Type::DateTime, QDateTime::fromString("2020-01-01 00:00:00", "yyyy-MM-dd hh:mm:ss"));
    config->registerConfig("Int", "名称", TConfig::Type::Int, 1);
    QString message;
    // 2. 测试 key 存在且类型匹配
    QVariant newDateTime = QDateTime::fromString("2024-12-03 12:00:00", "yyyy-MM-dd hh:mm:ss");
    bool writeResult1 = config->write("DateTime", newDateTime, message);
    QCOMPARE(writeResult1, true);                                       // 更新应该成功
    QCOMPARE(config->read("DateTime").value.toDateTime(), newDateTime); // 验证值已更新

    // 4. 测试 key 不存在
    QVariant newValue = 42;
    bool writeResult3 = config->write("NonExistentKey", newValue, message);
    QCOMPARE(writeResult3, false); // 更新失败，因为 key 不存在

    delete config;
}

void TestConfig::deepSeek()
{
    SetConsoleOutputCP(CP_UTF8);
    QEventLoop loop;
    TConfig *config = new TConfig("DeepSeek");
    DeepSeek *deepSeek = new DeepSeek(config->read("token").value.toString());
    deepSeek->setModel("deepseek-reasoner");
    deepSeek->setStream(true);
    bool flag = true;
    connect(deepSeek, &DeepSeek::replyStreamMessage, [&](const DeepSeek::Message &message)
            {   

                if (message.finish_reason && deepSeek->model() == "deepseek-reasoner" && flag)
                {
                    std::cout << "\n think finish" << std::endl;
                    flag = false;
                }
                if (!message.finish_reason)
                    std::cout << message.reasoning_content.toStdString();
                else
                    std::cout << message.content.toStdString(); });
    connect(deepSeek, &DeepSeek::replyMessage, [=](const DeepSeek::Message &message)
            { 
                if (deepSeek->model() == "deepseek-reasoner")
                {
                    std::cout << "\n think finish" << std::endl;
                    std::cout << message.reasoning_content.toStdString() << std::endl;
                }
                std::cout << message.content.toStdString(); });
    connect(deepSeek, &DeepSeek::replyFinished, [&](QNetworkReply::NetworkError error, int httpStatusCode, const QString &errorString)
            { std::cout << "\nFinished" << std::endl;
                auto usage = deepSeek->lastUsage();
                loop.exit(); });
    connect(deepSeek, &DeepSeek::replyBalance, [&](DeepSeek::Balance balance)
            { qDebug() << balance.toString(); });
    deepSeek->queryBalance();
    deepSeek->seedMessage({}, "你好");
    if (deepSeek->model() == "deepseek-reasoner")
        std::cout << "think" << std::endl;
    loop.exec();
    delete deepSeek;
}
