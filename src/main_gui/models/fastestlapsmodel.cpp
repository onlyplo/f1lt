/*******************************************************************************
 *                                                                             *
 *   F1LT - unofficial Formula 1 live timing application                       *
 *   Copyright (C) 2012-2013  Mariusz Pilarek (pieczaro@gmail.com)             *
 *                                                                             *
 *   This program is free software: you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation, either version 3 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                             *
 *******************************************************************************/


#include "fastestlapsmodel.h"

#include "../../core/colorsmanager.h"

FastestLapsModel::FastestLapsModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int FastestLapsModel::rowCount(const QModelIndex &) const
{
    return EventData::getInstance().getDriversData().size() + 1;
}

int FastestLapsModel::columnCount(const QModelIndex &) const
{
    return 8;
}

void FastestLapsModel::update()
{
    EventData &ed = EventData::getInstance();
    for (int i = 0; i < ed.getDriversData().size(); ++i)
    {
        DriverData &dd = ed.getDriversData()[i];
        if (fastestLaps.size() <= i)
            fastestLaps.append(dd.getSessionRecords().getBestLap());
        else
            fastestLaps[i] = dd.getSessionRecords().getBestLap();
    }
    qSort(fastestLaps);

    QModelIndex topLeft = QAbstractTableModel::index(0, 0);
    QModelIndex bottomRight = QAbstractTableModel::index(rowCount()-1, columnCount()-1);

    emit dataChanged(topLeft, bottomRight);
}

QVariant FastestLapsModel::data(const QModelIndex & index, int role) const
{
    if (index.row() == 0)
        return headerData(index, role);

    LapData ld = fastestLaps[index.row() - 1];
    DriverData *dd = EventData::getInstance().getDriverDataByIdPtr(ld.getCarID());
    if (dd == 0)
        return QVariant();

    switch (index.column())
    {
        case 0:
            if (role == Qt::DisplayRole)
            {
                if (ld.getTime().isValid())
                    return QString("%1.").arg(index.row());
            }

            if (role == Qt::ForegroundRole)
                return ColorsManager::getInstance().getColor(LTPackets::CYAN);

            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignVCenter | Qt::AlignRight);

            return QVariant();

        case 1:
            if (role == Qt::DisplayRole)
                return dd->getDriverName();

            if (role == Qt::ForegroundRole)
            {
                if (index.row() == 1)
                    return ColorsManager::getInstance().getColor(LTPackets::VIOLET);

                return ColorsManager::getInstance().getColor(LTPackets::WHITE);
            }
            return QVariant();

        case 2:
            if (role == Qt::DisplayRole)
                return ld.getTime().toString();

            if (role == Qt::ForegroundRole)
            {
                if (index.row() == 1)
                    return ColorsManager::getInstance().getColor(LTPackets::VIOLET);

                return ColorsManager::getInstance().getColor(LTPackets::GREEN);
            }
            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignCenter);

            return QVariant();

        case 3:
            if (role == Qt::DisplayRole && index.row() != 1)
                return DriverData::calculateGap(ld.getTime(), fastestLaps.first().getTime());


            if (role == Qt::ForegroundRole)
                return ColorsManager::getInstance().getColor(LTPackets::YELLOW);

            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignCenter);

            return QVariant();

        case 4:
            if (role == Qt::DisplayRole)
                return ld.getSectorTime(1).toString();

            if (role == Qt::ForegroundRole)
            {
                if (ld.getLapNumber() == EventData::getInstance().getSessionRecords().getSectorRecord(1).getLapNumber() &&
                    dd->getDriverName() == EventData::getInstance().getSessionRecords().getSectorRecord(1).getDriverName())
                    return ColorsManager::getInstance().getColor(LTPackets::VIOLET);


                if (ld.getLapNumber() == dd->getSessionRecords().getBestSectorLapNumber(1))
                    return ColorsManager::getInstance().getColor(LTPackets::GREEN);

                return ColorsManager::getInstance().getColor(LTPackets::WHITE);
            }

            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignCenter);

        case 5:
            if (role == Qt::DisplayRole)
                return ld.getSectorTime(2).toString();

            if (role == Qt::ForegroundRole)
            {
                if (ld.getLapNumber() == EventData::getInstance().getSessionRecords().getSectorRecord(2).getLapNumber() &&
                    dd->getDriverName() == EventData::getInstance().getSessionRecords().getSectorRecord(2).getDriverName())
                    return ColorsManager::getInstance().getColor(LTPackets::VIOLET);

                if (ld.getLapNumber() == dd->getSessionRecords().getBestSectorLapNumber(2))
                    return ColorsManager::getInstance().getColor(LTPackets::GREEN);

                return ColorsManager::getInstance().getColor(LTPackets::WHITE);
            }

            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignCenter);

        case 6:
            if (role == Qt::DisplayRole)
                return ld.getSectorTime(3).toString();

            if (role == Qt::ForegroundRole)
            {
                if (ld.getLapNumber() == EventData::getInstance().getSessionRecords().getSectorRecord(3).getLapNumber() &&
                    dd->getDriverName() == EventData::getInstance().getSessionRecords().getSectorRecord(3).getDriverName())
                    return ColorsManager::getInstance().getColor(LTPackets::VIOLET);

                if (ld.getLapNumber() == dd->getSessionRecords().getBestSectorLapNumber(3))
                    return ColorsManager::getInstance().getColor(LTPackets::GREEN);

                return ColorsManager::getInstance().getColor(LTPackets::WHITE);
            }

            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignCenter);

        case 7:
            if (role == Qt::DisplayRole)
            {
                if (ld.getTime().isValid())
                {
                    if (EventData::getInstance().getEventType() == LTPackets::QUALI_EVENT)
                        return QString("%1 (Q%2)").arg(ld.getLapNumber()).arg(ld.getQualiLapExtraData().getQualiPeriod());

                    return ld.getLapNumber();
                }
            }

            if (role == Qt::ForegroundRole)
                return ColorsManager::getInstance().getColor(LTPackets::WHITE);

            if (role == Qt::TextAlignmentRole)
                return (int)(Qt::AlignVCenter | Qt::AlignRight);
    }
    return QVariant();
}

QVariant FastestLapsModel::headerData(const QModelIndex & index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 0: return "P";
            case 1: return "Name";
            case 2: return "Time";
            case 3: return "Gap";
            case 4: return "S1";
            case 5: return "S2";
            case 6: return "S3";
            case 7: return "Lap";
        }
    }
    if (role == Qt::ForegroundRole)
        return ColorsManager::getInstance().getColor(LTPackets::DEFAULT);

    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() >= 2 && index.column() <= 6)
            return (int)(Qt::AlignCenter);

        if (index.column() == 0 || index.column() == 7)
            return (int)(Qt::AlignVCenter | Qt::AlignRight);
    }

    return QVariant();
}
