#pragma once

#include "BaseTraceViewModel.h"
#include "UnifiedTraceItem.h"
#include <decoders/ProtocolManager.h>
#include <memory>
#include <map>

class UnifiedTraceViewModel : public BaseTraceViewModel
{
    Q_OBJECT

public:
    enum Category {
        Cat_All,
        Cat_UDS,
        Cat_J1939
    };

    UnifiedTraceViewModel(Backend &backend, Category category = Cat_All);
    ~UnifiedTraceViewModel();

    void setCategory(Category category) { m_category = category; }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual bool hasChildren(const QModelIndex &parent) const override;

    virtual CanMessage getMessage(const QModelIndex &index) const override;

    virtual QVariant data(const QModelIndex &index, int role) const override;

private slots:
    void beforeAppend(int num_messages);
    void afterAppend();
    void beforeRemove(int count);
    void afterRemove(int count);
    void beforeClear();
    void afterClear();
    void onSetupChanged();
    void onResumeMeasurement();
    void onPauseMeasurement();

private:
    bool m_isActive = true;
    std::shared_ptr<UnifiedTraceItem> m_rootItem;
    std::shared_ptr<UnifiedTraceItem> m_aggHeader;
    std::shared_ptr<UnifiedTraceItem> m_udsHeader;
    std::shared_ptr<UnifiedTraceItem> m_j1939Header;
    
    Category m_category;
    ProtocolManager m_protocolManager;
    int m_lastProcessedIndex = -1;
    uint32_t m_globalIndexCounter = 1;
    uint64_t m_firstTimestamp = 0;
    uint64_t m_previousRowTimestamp = 0;
    int m_maxRows = 10000;

    std::map<uint32_t, std::shared_ptr<UnifiedTraceItem>> m_j1939AggregatedMap;
    uint32_t getJ1939Key(const ProtocolMessage& pmsg) const;

    QVariant data_DisplayRole(const QModelIndex &index) const;
    QVariant data_TextColorRole(const QModelIndex &index) const;
    QString formatUnifiedTimestamp(uint64_t ts, uint64_t prevTs) const;
};
