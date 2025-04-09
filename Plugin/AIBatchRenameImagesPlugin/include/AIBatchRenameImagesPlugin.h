#ifndef AIBatchRenameImagesPlugin_H
#define AIBatchRenameImagesPlugin_H

#include "AIBatchRenameImagesPlugin_global.h"
#include "AbstractPlugin.h"
#include "config.h"
#include "AIBatchRenameImagesModel.h"

class AIBatchRenameImagesPlugin_EXPORT AIBatchRenameImagesPlugin : public AbstractPlugin
{
    Q_OBJECT
public:
    AIBatchRenameImagesPlugin(Logger * logger,TConfig *config,QObject *parent = nullptr);
    ~AIBatchRenameImagesPlugin() override;
    virtual QString group() override;
    virtual QString name() override;
    virtual QString version() override;
    virtual QString author() override;
    virtual QString description() override;
    virtual QIcon icon() override;
    virtual QWidget *start() override;
    virtual void stop() override;
    virtual void writeConfigBeforeEvent(WriteConfigEvent &event) override;
    virtual void writeConfigAfterEvent(WriteConfigEvent &event) override;
    virtual void readConfigBeforeEvent(ReadConfigEvent &event) override;
    virtual void readConfigAfterEvent(ReadConfigEvent &event) override;
    virtual void registerConfig() override;
};


class AIBatchRenameImagesPlugin_EXPORT AIBatchRenameImagesPluginFactory : public QObject, public PluginFactory
{
	Q_OBJECT
		Q_PLUGIN_METADATA(IID AbstractPlugin_IID)
		Q_INTERFACES(PluginFactory)
public:
	AbstractPlugin* create(Logger *logger,TConfig *config) override { return new AIBatchRenameImagesPlugin(logger,config); };
	~AIBatchRenameImagesPluginFactory() override = default;
} ;


#endif // AIBatchRenameImagesPlugin_H
