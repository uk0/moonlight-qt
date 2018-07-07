#include "computermodel.h"

ComputerModel::ComputerModel(QObject* object)
    : QAbstractListModel(object) {}

void ComputerModel::initialize(ComputerManager* computerManager)
{
    m_ComputerManager = computerManager;
    connect(m_ComputerManager, &ComputerManager::computerStateChanged,
            this, &ComputerModel::handleComputerStateChanged);
    connect(m_ComputerManager, &ComputerManager::pairingCompleted,
            this, &ComputerModel::handlePairingCompleted);

    m_Computers = m_ComputerManager->getComputers();
}

QVariant ComputerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() == m_Computers.count()) {
        // We insert a synthetic item at the end for the Add PC option
        switch (role) {
        case NameRole:
            return "Add PC";
        case AddPcRole:
            return true;
        default:
            return QVariant();
        }
    }

    Q_ASSERT(index.row() < m_Computers.count());

    NvComputer* computer = m_Computers[index.row()];
    QReadLocker lock(&computer->lock);

    switch (role) {
    case NameRole:
        return computer->name;
    case OnlineRole:
        return computer->state == NvComputer::CS_ONLINE;
    case PairedRole:
        return computer->pairState == NvComputer::PS_PAIRED;
    case BusyRole:
        return computer->currentGameId != 0;
    case AddPcRole:
        return false;
    default:
        return QVariant();
    }
}

int ComputerModel::rowCount(const QModelIndex& parent) const
{
    // We should not return a count for valid index values,
    // only the parent (which will not have a "valid" index).
    if (parent.isValid()) {
        return 0;
    }

    // Add PC placeholder counts as 1
    return m_Computers.count() + 1;
}

QHash<int, QByteArray> ComputerModel::roleNames() const
{
    QHash<int, QByteArray> names;

    names[NameRole] = "name";
    names[OnlineRole] = "online";
    names[PairedRole] = "paired";
    names[BusyRole] = "busy";
    names[AddPcRole] = "addPc";

    return names;
}

void ComputerModel::deleteComputer(int computerIndex)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    beginRemoveRows(QModelIndex(), computerIndex, computerIndex);

    // m_Computer[computerIndex] will be deleted by this call
    m_ComputerManager->deleteHost(m_Computers[computerIndex]);

    // Remove the now invalid item
    m_Computers.removeAt(computerIndex);

    endRemoveRows();
}

bool ComputerModel::wakeComputer(int computerIndex)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    return m_Computers[computerIndex]->wake();
}

void ComputerModel::pairComputer(int computerIndex, QString pin)
{
    Q_ASSERT(computerIndex < m_Computers.count());

    m_ComputerManager->pairHost(m_Computers[computerIndex], pin);
}

void ComputerModel::handlePairingCompleted(NvComputer*, QString error)
{
    emit pairingCompleted(error.isNull() ? QVariant() : error);
}

void ComputerModel::handleComputerStateChanged(NvComputer* computer)
{
    // If this is an existing computer, we can report the data changed
    int index = m_Computers.indexOf(computer);
    if (index >= 0) {
        // Let the view know that this specific computer changed
        emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    }
    else {
        // This is a new PC which may be inserted at an arbitrary point
        // in our computer list (since it comes from CM's QMap). Reload
        // the whole model state to ensure it stays consistent.
        beginResetModel();
        m_Computers = m_ComputerManager->getComputers();
        endResetModel();
    }
}
