#include "eventrecorder.h"
#include <QDataStream>
#include <QFile>


EventRecorder::EventRecorder(QObject *parent) :
    QObject(parent), eventData(EventData::getInstance()), elapsedSeconds(0)
{
}

void EventRecorder::startRecording()
{
    //prepare everyting for record, clear old records and store the LTTeam and LTEvent data
    ltTeamList = LTData::ltTeams;
    ltEvent = eventData.getEventInfo();

    packets.clear();
    elapsedSeconds = 0;
    elapsedTimeToStop = -1;

    gatherInitialData();    

//    eventDataList.clear();
//    lapDataList.clear();

//    for (int i = 0; i < ltTeamList.size(); ++i)
//    {
//        //we've got 2 drivers in every team so we store the place to hold lap datas
//        lapDataList.append(QList<LapData>());
//        lapDataList.append(QList<LapData>());
//    }
}

/*Before start of the recording, we have to gather all initial data, that will be needed to correctly startup replaying the event. This includes:
  - event type
  - track status
  - remaining time
  - car position update and history, driver numbers and names
  - weather data
  - speed records
  - fastest lap data
  - commentary
  */
void EventRecorder::gatherInitialData()
{
    Packet packet;

    gatherSysData();
    gatherDriverData();

    //commentary
    packet.carID = 0;
    packet.type = LTData::SYS_COMMENTARY;
    packet.data = 0;
    packet.longData.append("  ");   //append 2 empty bytes
    packet.longData.append(eventData.getCommentary());
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));

    ++elapsedSeconds;
}

void EventRecorder::gatherSysData()
{
    Packet packet;

    //event type
    packet.carID = 0;
    packet.type = LTData::SYS_EVENT_ID;
    packet.data = eventData.getEventType();
    packet.length = 0;      //we don't need a number for the decryption key
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //track status
    packet.carID = 0;
    packet.type = LTData::SYS_TRACK_STATUS;
    packet.data = 1;
    packet.longData.append((char)(eventData.getFlagStatus()) + '0');
    packet.length = 1;
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //remaining time
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_SESSION_CLOCK;
    packet.longData.append(eventData.getRemainingTime().toString("h:mm:ss"));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();


    //weather data
    //track temp
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_TRACK_TEMP;
    packet.longData.append(QString::number(eventData.getWeather().getTrackTemp().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //air temp
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_AIR_TEMP;
    packet.longData.append(QString::number(eventData.getWeather().getAirTemp().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //wind speed
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_WIND_SPEED;
    packet.longData.append(QString::number(eventData.getWeather().getWindSpeed().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //humidity
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_HUMIDITY;
    packet.longData.append(QString::number(eventData.getWeather().getHumidity().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //pressure
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_PRESSURE;
    packet.longData.append(QString::number(eventData.getWeather().getPressure().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //wind dir.
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_WIND_DIRECTION;
    packet.longData.append(QString::number(eventData.getWeather().getWindDirection().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //wet track
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_WET_TRACK;
    packet.longData.append(QString::number(eventData.getWeather().getWetDry().getValue()));
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //speed records
    //sector 1
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::SPEED_SECTOR1));
    for (int i = 0; i < 6; ++i)
    {
        QString name = LTData::getDriverShortName(eventData.getSessionRecords().getSectorSpeed(1, i).getDriverName());
        QString speed = QString::number(eventData.getSessionRecords().getSectorSpeed(1, i).getSpeed());
        packet.longData.append(name + speed);
    }
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //sector 2
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::SPEED_SECTOR2));
    for (int i = 0; i < 6; ++i)
    {
        QString name = LTData::getDriverShortName(eventData.getSessionRecords().getSectorSpeed(2, i).getDriverName());
        QString speed = QString::number(eventData.getSessionRecords().getSectorSpeed(2, i).getSpeed());
        packet.longData.append(name + speed);
    }
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //sector 3
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::SPEED_SECTOR3));
    for (int i = 0; i < 6; ++i)
    {
        QString name = LTData::getDriverShortName(eventData.getSessionRecords().getSectorSpeed(3, i).getDriverName());
        QString speed = QString::number(eventData.getSessionRecords().getSectorSpeed(3, i).getSpeed());
        packet.longData.append(name + speed);
    }
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //speed trap
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::SPEED_TRAP));
    for (int i = 0; i < 6; ++i)
    {
        QString name = LTData::getDriverShortName(eventData.getSessionRecords().getSpeedTrap(i).getDriverName());
        QString speed = QString::number(eventData.getSessionRecords().getSpeedTrap(i).getSpeed());
        packet.longData.append(name + speed);
    }
    packet.length = packet.longData.size();
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //fastest lap data
    //FL_CAR
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::FL_CAR));
    packet.longData.append(QString::number(eventData.getSessionRecords().getFastestLap().getNumber()));
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //FL_DRIVER
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::FL_DRIVER));
    packet.longData.append(eventData.getSessionRecords().getFastestLap().getDriverName().toUpper());
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //FL_LAP
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::FL_LAP));
    packet.longData.append(QString::number(eventData.getSessionRecords().getFastestLap().getLapNumber()));
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();

    //FL_TIME
    packet.carID = 0;
    packet.type = LTData::SYS_SPEED;
    packet.data = 0;
    packet.longData.append(QString::number(LTData::FL_LAP));
    packet.longData.append(eventData.getSessionRecords().getFastestLap().getTime());
    packets.append(QPair<int, Packet>(elapsedSeconds, packet));
    packet.longData.clear();
}

void EventRecorder::gatherDriverData()
{
    Packet packet;
    //car position update
    for (int i = 0; i < eventData.getDriversData().size(); ++i)
    {
        packet.type = LTData::CAR_POSITION_UPDATE;
        packet.carID = eventData.getDriversData()[i].getCarID();
        packet.length = 0;
        packet.data = eventData.getDriversData()[i].getPosition();
        packets.append(QPair<int, Packet>(elapsedSeconds, packet));
        packet.longData.clear();
    }

    //car position history
    for (int i = 0; i < eventData.getDriversData().size(); ++i)
    {
        packet.type = LTData::CAR_POSITION_HISTORY;
        packet.carID = eventData.getDriversData()[i].getCarID();
        packet.data = 0;
        packet.length = eventData.getDriversData()[i].getPositionHistory().size();

        for (int j = 0; j < packet.length; ++j)
            packet.longData.append((char)(eventData.getDriversData()[i].getPositionHistory()[j]));

        packets.append(QPair<int, Packet>(elapsedSeconds, packet));
        packet.longData.clear();
    }

    //other car data
    for (int i = 0; i < eventData.getDriversData().size(); ++i)
    {
        //position
        packet.type = LTData::RACE_POSITION;
        packet.carID = eventData.getDriversData()[i].getCarID();
        packet.data = eventData.getDriversData()[i].getColorData().positionColor();
        packet.longData.append(QString::number(eventData.getDriversData()[i].getPosition()));
        packets.append(QPair<int, Packet>(elapsedSeconds, packet));
        packet.longData.clear();


        //number
        packet.type = LTData::RACE_NUMBER;
        packet.carID = eventData.getDriversData()[i].getCarID();
        packet.data = eventData.getDriversData()[i].getColorData().numberColor();
        packet.longData.append(QString::number(eventData.getDriversData()[i].getNumber()));
        packets.append(QPair<int, Packet>(elapsedSeconds, packet));
        packet.longData.clear();

        //car driver
        packet.type = LTData::RACE_DRIVER;
        packet.carID = eventData.getDriversData()[i].getCarID();
        packet.data = eventData.getDriversData()[i].getColorData().driverColor();
        packet.longData.append(eventData.getDriversData()[i].getDriverName().toUpper());
        packets.append(QPair<int, Packet>(elapsedSeconds, packet));
        packet.longData.clear();

        if (eventData.getEventType() == LTData::RACE_EVENT)
        {
            //race gap
            packet.type = LTData::RACE_GAP;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().gapColor();
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getGap());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //race gap
            packet.type = LTData::RACE_INTERVAL;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().intervalColor();
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getInterval());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //race lap time
            packet.type = LTData::RACE_LAP_TIME;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().lapTimeColor();
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getTime().toString());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //race sector 1
            packet.type = LTData::RACE_SECTOR_1;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(1);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(1));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //race sector 2
            packet.type = LTData::RACE_SECTOR_2;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(2);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(2));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //race sector 3
            packet.type = LTData::RACE_SECTOR_3;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(3);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(3));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //race num pits
            packet.type = LTData::RACE_NUM_PITS;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().numPitsColor();
            packet.longData.append(QString::number(eventData.getDriversData()[i].getNumPits()));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();
        }
        else if (eventData.getEventType() == LTData::QUALI_EVENT)
        {
            //quali 1
            packet.type = LTData::QUALI_PERIOD_1;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().qualiTimeColor(1);
            packet.longData.append(eventData.getDriversData()[i].getQualiTime(1).toString());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //quali 2
            packet.type = LTData::QUALI_PERIOD_2;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().qualiTimeColor(2);
            packet.longData.append(eventData.getDriversData()[i].getQualiTime(2).toString());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //quali 3
            packet.type = LTData::QUALI_PERIOD_3;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().qualiTimeColor(3);
            packet.longData.append(eventData.getDriversData()[i].getQualiTime(3).toString());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //quali sector 1
            packet.type = LTData::QUALI_SECTOR_1;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(1);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(1));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //quali sector 2
            packet.type = LTData::QUALI_SECTOR_2;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(2);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(2));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //quali sector 3
            packet.type = LTData::QUALI_SECTOR_3;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(3);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(3));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //quali lap
            packet.type = LTData::QUALI_LAP;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().numLapsColor();
            packet.longData.append(QString::number(eventData.getDriversData()[i].getLastLap().getLapNumber()));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();
        }
        else if (eventData.getEventType() == LTData::PRACTICE_EVENT)
        {
            //practice best
            packet.type = LTData::PRACTICE_BEST;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().lapTimeColor();
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getTime().toString());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //practice gap
            packet.type = LTData::PRACTICE_GAP;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().gapColor();
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getGap());
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //practice sector 1
            packet.type = LTData::PRACTICE_SECTOR_1;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(1);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(1));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //practice sector 2
            packet.type = LTData::PRACTICE_SECTOR_2;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(2);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(2));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //practice sector 3
            packet.type = LTData::PRACTICE_SECTOR_3;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().sectorColor(3);
            packet.longData.append(eventData.getDriversData()[i].getLastLap().getSectorTime(3));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();

            //practice lap
            packet.type = LTData::PRACTICE_LAP;
            packet.carID = eventData.getDriversData()[i].getCarID();
            packet.data = eventData.getDriversData()[i].getColorData().numLapsColor();
            packet.longData.append(QString::number(eventData.getDriversData()[i].getLastLap().getLapNumber()));
            packets.append(QPair<int, Packet>(elapsedSeconds, packet));
            packet.longData.clear();
        }
    }
}

void EventRecorder::stopRecording()
{
    saveToFile("");
    //store the last eventData
//    eventDataList.append(eventData);
}

void EventRecorder::appendPacket(const Packet &p)
{
    packets.append(QPair<int, Packet>(elapsedSeconds, p));
}

//void EventRecorder::updateEventData(const EventData &ed)
//{
//    //store the EventData object but without its driverData list - we've got here our own list
//    QList<DriverData> tmpList = eventData.driversData;
//    eventData = ed;
//    eventData.driversData = tmpList;
//}

//void EventRecorder::updateDriverData(const DriverData &dd)
//{
//    //check if have this driver in the list, if yes - update it, if no - append it
//    for (int i = 0; i < eventData.getDriversData().size(); ++i)
//    {
//        if (dd.carID == eventData.getDriversData()[i].carID)
//        {
//            eventData.getDriversData()[i] = dd;
//            storeLapData(dd);
//            eventData.getDriversData()[i].lapData.clear();
//            return;
//        }
//    }
//    eventData.driversData.append(dd);
//    storeLapData(dd);
//    eventData.driversData.last().lapData.clear();
//}

//void EventRecorder::storeLapData(const DriverData &dd)
//{
//    if (dd.carID == -1)
//        return;

//    //ok, we have to copy all driver laps, but without the ones we already have, we only update the last lap

//    //if the current driver lap data list is empty, all his laps are appended
//    bool startAppending = lapDataList[dd.carID-1].isEmpty() ? true : false;
//    for (int i = 0; i < dd.lapData.size(); ++i)
//    {
//        if (startAppending)
//            lapDataList[dd.carID-1].append(dd.lapData[i]);

//        //updating the last lap
//        if (!lapDataList[dd.carID-1].isEmpty() && dd.lapData[i].numLap == lapDataList[dd.carID-1].last().numLap)
//        {
//            lapDataList[dd.carID-1].last() = dd.lapData[i];
//            //from now on we can append the remaning laps
//            startAppending = true;
//        }
//    }
//}

void EventRecorder::timeout()
{
    ++elapsedSeconds;
    //append the system clock packet every second
    Packet packet;
    packet.carID = 0;
    packet.type = LTData::SYS_WEATHER;
    packet.data = LTData::WEATHER_SESSION_CLOCK;
    packet.longData.append(eventData.getRemainingTime().toString("h:mm:ss"));
    packet.length = packet.longData.size();

    packets.append(QPair<int, Packet>(elapsedSeconds, packet));


//    DriverData dd;
//    int qPeriod = 0;
//
//    if (eventData.eventType == LTData::QUALI_EVENT)
//    {
//    	dd = eventData.getDriverDataFromPos(1);
//    	qPeriod = (!dd.lapData.isEmpty() ? dd.lapData.last().qualiPeriod : 0);
//    	qDebug() << "quali=" << qPeriod << ", driver=" << dd.driver << ", " << elapsedTimeToStop << ", " << autoStopRecord;
//    }
    if (autoStopRecord >= 0)
    {
        if((eventData.getRemainingTime().toString("h:mm:ss") == "0:00:00" && eventData.getEventType() == LTData::PRACTICE_EVENT) ||
            (eventData.getRemainingTime().toString("h:mm:ss") == "0:00:00" && eventData.getEventType() == LTData::QUALI_EVENT && eventData.getQualiPeriod() == 3) ||
            (eventData.getCompletedLaps() == eventData.getEventInfo().laps && eventData.getEventType() == LTData::RACE_EVENT))
    		++elapsedTimeToStop;

		if (elapsedTimeToStop >= (autoStopRecord * 60))
		{
			emit recordingStopped();
			stopRecording();
		}
    }


    //on every timeout the eventData object is appended into eventDataList and the driverData list is cleared
//    eventDataList.append(eventData);
//    eventData.driversData.clear();
}

void EventRecorder::saveToFile(QString)
{
    if (packets.size() > 0)
    {
        int year = eventData.getEventInfo().raceDate.year();
        int no = eventData.getEventInfo().eventNo;
        QString shortName = eventData.getEventInfo().eventShortName.toLower();

        QString session;
        switch (eventData.getEventType())
        {
            case LTData::RACE_EVENT:
                session = "race"; break;

            case LTData::QUALI_EVENT:
                session = "quali"; break;

            default:
                session = "fp" + QString::number(LTData::getFPNumber()); break;
        }
        QString sNo = QString::number(no);
        if (no < 10)
            sNo = "0" + QString::number(no);

        QString fName = QString("ltdata/%1-%2-%3-%4.lt").arg(year).arg(sNo).arg(shortName).arg(session);

        //since we have 3 practice session we have to choose the correct session number
        if (session == "fp1" && QFile::exists(fName))
        {
            fName = QString("ltdata/%1-%2-%3-%4.lt").arg(year).arg(sNo).arg(shortName).arg("fp2");

            if (QFile::exists(fName))
                fName = QString("ltdata/%1-%2-%3-%4.lt").arg(year).arg(sNo).arg(shortName).arg("fp3");
        }
        QDir dir(F1LTCore::ltDataHomeDir());
        if (!dir.exists())
            dir.mkpath(F1LTCore::ltDataHomeDir());

        QFile f(fName);
        if (f.open(QIODevice::WriteOnly))
        {
            QDataStream stream(&f);

            const char tab[5] = "F1LT";

            stream << tab;
            //save the teams and events info
            saveLTInfo(stream);
            savePackets(stream);

            //now save events
//            saveEventData(stream);

//            saveLapData(stream);
        }
    }
}

void EventRecorder::saveLTInfo(QDataStream &stream)
{
    int size;// = ltEventList.size();
//    stream << size;

    QString sbuf;
    int ibuf;
    QPixmap pixBuf;
//    for (int i = 0; i < size; ++i)
    {
        ibuf = ltEvent.eventNo;
        stream << ibuf;

        sbuf = ltEvent.eventName;
        stream << sbuf;

        sbuf = ltEvent.eventShortName;
        stream << sbuf;

        sbuf = ltEvent.eventPlace;
        stream << sbuf;

        sbuf = ltEvent.fpDate.toString("dd-MM-yyyy");
        stream << sbuf;

        sbuf = ltEvent.raceDate.toString("dd-MM-yyyy");
        stream << sbuf;

        ibuf = ltEvent.laps;
        stream << ibuf;

        pixBuf = ltEvent.trackImg;
        stream << pixBuf;
    }

    size = ltTeamList.size();
    stream << size;
    for (int i = 0; i < size; ++i)
    {
        sbuf = ltTeamList[i].teamName;
        stream << sbuf;

        sbuf = ltTeamList[i].driver1Name;
        stream << sbuf;

        sbuf = ltTeamList[i].driver1ShortName;
        stream << sbuf;

        ibuf = ltTeamList[i].driver1No;
        stream << ibuf;

        sbuf = ltTeamList[i].driver2Name;
        stream << sbuf;

        sbuf = ltTeamList[i].driver2ShortName;
        stream << sbuf;

        ibuf = ltTeamList[i].driver2No;
        stream << ibuf;

        pixBuf = ltTeamList[i].carImg;
        stream << pixBuf;
    }
}

void EventRecorder::savePackets(QDataStream &stream)
{
    stream << packets.size();

    for (int i = 0; i < packets.size(); ++i)
    {
        stream << packets[i].first;
        stream << packets[i].second.type;
        stream << packets[i].second.carID;
        stream << packets[i].second.data;
        stream << packets[i].second.length;
        stream << packets[i].second.longData;
    }
}

//void EventRecorder::saveEventData(QDataStream &stream)
//{
//    int size = eventDataList.size();
//    stream << size;

//    QString sbuf;
//    int ibuf;
//    double dbuf;
//    for (int i = 0; i < size; ++i)
//    {
////        ibuf = eventDataList[i].key;
//        stream << ibuf;

////        sbuf = eventDataList[i].cookie;
//        stream << sbuf;

////        ibuf = eventDataList[i].frame;
//        stream << ibuf;

//        ibuf = eventDataList[i].eventNo;
//        stream << ibuf;

////        ibuf = eventDataList[i].salt;
//        stream << ibuf;

//        ibuf = eventDataList[i].eventType;
//        stream << ibuf;

//        ibuf = eventDataList[i].flagStatus;
//        stream << ibuf;

//        sbuf = eventDataList[i].remainingTime.toString("h:mm:ss");
//        stream << sbuf;

//        ibuf = eventDataList[i].lapsCompleted;
//        stream << ibuf;

//        ibuf = eventDataList[i].totalLaps;
//        stream << ibuf;

//        dbuf = eventDataList[i].airTemp;
//        stream << dbuf;

//        dbuf = eventDataList[i].humidity;
//        stream << dbuf;

//        dbuf = eventDataList[i].windSpeed;
//        stream << dbuf;

//        dbuf = eventDataList[i].windDirection;
//        stream << dbuf;

//        dbuf = eventDataList[i].pressure;
//        stream << dbuf;

//        dbuf = eventDataList[i].trackTemp;
//        stream << dbuf;

//        ibuf = eventDataList[i].wetdry;
//        stream << ibuf;

//        sbuf = eventDataList[i].commentary;
//        stream << sbuf;

//        for (int j = 0; j < 12; ++j)
//        {
//            sbuf = eventDataList[i].sec1Speed[j];
//            stream << sbuf;

//            sbuf = eventDataList[i].sec2Speed[j];
//            stream << sbuf;

//            sbuf = eventDataList[i].sec3Speed[j];
//            stream << sbuf;

//            sbuf = eventDataList[i].speedTrap[j];
//            stream << sbuf;
//        }
//        ibuf = eventDataList[i].FLNumber;
//        stream << ibuf;

//        sbuf = eventDataList[i].FLDriver;
//        stream << sbuf;

//        ibuf = eventDataList[i].FLLap;
//        stream << ibuf;

//        sbuf = eventDataList[i].FLTime.toString();
//        stream << sbuf;

//        for (int j = 0; j < 3; ++j)
//        {
//            sbuf = eventDataList[i].sec1Record[j];
//            stream << sbuf;

//            sbuf = eventDataList[i].sec2Record[j];
//            stream << sbuf;

//            sbuf = eventDataList[i].sec3Record[j];
//            stream << sbuf;
//        }

//        ibuf = eventDataList[i].getDriversData().size();
//        stream << ibuf;

//        for (int j = 0; j < eventDataList[i].getDriversData().size(); ++j)
//        {
//            ibuf = eventDataList[i].getDriversData()[j].getCarID(); stream << ibuf;
//            sbuf = eventDataList[i].getDriversData()[j].driver; stream << sbuf;
//            ibuf = eventDataList[i].getDriversData()[j].number; stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].pos; stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].numPits; stream << ibuf;
////            ibuf = eventDataList[i].getDriversData()[j].numPits; stream << ibuf;   //wczytac to 2. raz!
//            ibuf = eventDataList[i].getDriversData()[j].retired; stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].releasedFromPits; stream << ibuf;
//            sbuf = eventDataList[i].getDriversData()[j].q1.toString(); stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].q2.toString(); stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].q3.toString(); stream << sbuf;

//            ibuf = eventDataList[i].getDriversData()[j].posHistory.size();
//            stream << ibuf;

//            for (int k = 0; k < eventDataList[i].getDriversData()[j].posHistory.size(); ++k)
//            {
//                ibuf = eventDataList[i].getDriversData()[j].posHistory[k];
//                stream << ibuf;
//            }
//            for (int k = 0; k < 14; ++k)
//            {
//                ibuf = eventDataList[i].getDriversData()[j].getColorData().k];
//                stream << ibuf;
//            }

//            ibuf = eventDataList[i].getDriversData()[j].pitData.size();
//            stream << ibuf;

//            for (int k = 0; k < eventDataList[i].getDriversData()[j].pitData.size(); ++k)
//            {
//                sbuf = eventDataList[i].getDriversData()[j].pitData[k].pitTime; stream << sbuf;
//                ibuf = eventDataList[i].getDriversData()[j].pitData[k].pitLap; stream << ibuf;
//            }

//            //lastLap & bestLap
//            ibuf = eventDataList[i].getDriversData()[j].getLastLap().getCarID(); stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].bestLap.getCarID(); stream << ibuf;

////            ibuf = eventDataList[i].getDriversData()[j].getLastLap().number; stream << ibuf;
////            ibuf = eventDataList[i].getDriversData()[j].bestLap.number; stream << ibuf;

//            ibuf = eventDataList[i].getDriversData()[j].getLastLap().pos; stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].bestLap.pos; stream << ibuf;

//            sbuf = eventDataList[i].getDriversData()[j].getLastLap().gap; stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].bestLap.gap; stream << sbuf;

//            sbuf = eventDataList[i].getDriversData()[j].getLastLap().interval; stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].bestLap.interval; stream << sbuf;

//            sbuf = eventDataList[i].getDriversData()[j].getLastLap().lapTime.toString(); stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].bestLap.lapTime.toString(); stream << sbuf;

//            sbuf = eventDataList[i].getDriversData()[j].getLastLap().sector1.toString(); stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].bestLap.sector1.toString(); stream << sbuf;

//            sbuf = eventDataList[i].getDriversData()[j].getLastLap().sector2.toString(); stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].bestLap.sector2.toString(); stream << sbuf;

//            sbuf = eventDataList[i].getDriversData()[j].getLastLap().sector3.toString(); stream << sbuf;
//            sbuf = eventDataList[i].getDriversData()[j].bestLap.sector3.toString(); stream << sbuf;

//            ibuf = eventDataList[i].getDriversData()[j].getLastLap().numLap; stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].bestLap.numLap; stream << ibuf;

//            ibuf = eventDataList[i].getDriversData()[j].getLastLap().scLap; stream << ibuf;
//            ibuf = eventDataList[i].getDriversData()[j].bestLap.scLap; stream << ibuf;
//        }
//    }
//}

//void EventRecorder::saveLapData(QDataStream &stream)
//{
//    int size = lapDataList.size();
//    stream << size;

//    QString sbuf;
//    int ibuf;
//    double dbuf;

//    for (int i = 0; i < size; ++i)
//    {
//        ibuf = lapDataList[i].size();
//        stream << ibuf;

//        for (int j = 0; j < lapDataList[i].size(); ++j)
//        {
//            ibuf = lapDataList[i][j].getCarID(); stream << ibuf;

////            ibuf = lapDataList[i][j].number; stream << ibuf;

//            ibuf = lapDataList[i][j].pos; stream << ibuf;

//            sbuf = lapDataList[i][j].gap; stream << sbuf;

//            sbuf = lapDataList[i][j].interval; stream << sbuf;

//            sbuf = lapDataList[i][j].lapTime.toString(); stream << sbuf;

//            sbuf = lapDataList[i][j].sector1.toString(); stream << sbuf;

//            sbuf = lapDataList[i][j].sector2.toString(); stream << sbuf;

//            sbuf = lapDataList[i][j].sector3.toString(); stream << sbuf;

//            ibuf = lapDataList[i][j].numLap; stream << ibuf;

//            ibuf = lapDataList[i][j].scLap; stream << ibuf;
//        }
//    }
//}

