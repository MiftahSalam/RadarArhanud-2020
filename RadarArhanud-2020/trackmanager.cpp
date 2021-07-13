#include "trackmanager.h"
#include "arpatarget.h"

TracksCluster::TracksCluster(
        RadarEngineARND::RadarEngine *ri,
        RadarEngineARND::RadarEngine *ri1,
        RadarEngineARND::Position pos) :
    m_ri(ri),m_ri1(ri1),clusterPosition(pos)
{
    track_counter++; //sementara. need to recheck later
    clusterTrackNumber = track_counter; //sementara. need to recheck later

    for (int i = 0; i < ANTENE_COUNT; ++i)
    {
        m_ri->radarArpa[i]->AcquireNewMARPATarget(clusterPosition);
        int tn = m_ri->radarArpa[i]->m_number_of_targets;
        m_ri->radarArpa[i]->m_target[tn-1]->m_target_number = clusterTrackNumber;
        rTracks[0].insert(i,m_ri->radarArpa[i]->m_target[tn-1]);
//        rTracks[0].insert(i,m_ri->radarArpa[i]->m_number_of_targets-1);
//        m_ri->radarArpa[i]->m_target[rTracks[0].value(i)]->m_target_number = clusterTrackNumber; //sementara

//        m_ri1->radarArpa[i]->AcquireNewMARPATarget(clusterPosition);
//        rTracks[1].insert(i,m_ri1->radarArpa[i]->m_number_of_targets-1);
    }
    refSource.first = 0;
    refSource.second = 0;
}

Track TracksCluster::getClusterTrack() const
{
    Track track;
    track.radarId = 0; //sementara. only from radar 0, radar 1 discarded
    track.source.antheneId = 0; //sementara. only from radar 0, radar 1 discarded. Also, clustering algo not implemented yet
//    track.source.trackId = rTracks[0].first(); //sementara. only from radar 0, radar 1 discarded. Also, clustering algo not implemented yet
    return track;
}

void TracksCluster::setLostClusteredTrack()
{
    QMapIterator<int, ARPATarget*> i(rTracks[0]);
//    QMapIterator<int, int> i(rTracks[0]);
    clusterTrackStatus = LOST;

    qDebug()<<Q_FUNC_INFO<<"clusterTrackNumber"<<clusterTrackNumber<<"rTracks size"<<rTracks[0].size();

    while (i.hasNext())
    {
        i.next();
        int aId = i.key();
//        int tId = i.value();
        ARPATarget *t_ptr = i.value();
        qDebug()<<Q_FUNC_INFO<<"radar 0"<<"anthene"<<aId<<"t_ptr"<<t_ptr;
        t_ptr->SetStatusLost();
//        m_ri->radarArpa[aId]->m_target[tId]->SetStatusLost();
    }

    for (int var = 0; var < ANTENE_COUNT; ++var)
        m_ri->radarArpa[var]->RefreshArpaTargets();
}

void TracksCluster::updateClusteredTrackPointer()
{
    for (int var = 0; var < ANTENE_COUNT; ++var)
    {
        qDebug()<<Q_FUNC_INFO<<"clusterTrackNumber"<<clusterTrackNumber<<"rTracks at"<<var<<"awal"<<rTracks[0].value(var);
        for (int var1 = 0; var1 < m_ri->radarArpa[var]->m_number_of_targets; ++var1)
        {
            if(m_ri->radarArpa[var]->m_target[var1]->m_target_number == clusterTrackNumber)
            {
                ARPATarget *t_ptr = m_ri->radarArpa[var]->m_target[var1];
                rTracks[0].insert(var,t_ptr);
//                rTracks[0].insert(var,var1);
            }
        }
        qDebug()<<Q_FUNC_INFO<<"clusterTrackNumber"<<clusterTrackNumber<<"rTracks at"<<var<<"akhir"<<rTracks[0].value(var);
    }
}
void TracksCluster::updateClusteredTrackStatus()
{
    QMapIterator<int, ARPATarget*> i(rTracks[0]);
    QList<int> aId_to_delete;
    bool found = false;
//    QMapIterator<int, int> i(rTracks[0]);
    clusterTrackStatus = LOST;

//    qDebug()<<Q_FUNC_INFO<<"clusterTrackNumber"<<clusterTrackNumber<<"rTracks"<<rTracks[0].size();

    while (i.hasNext())
    {
        i.next();
        int aId = i.key();
        ARPATarget* t_ptr = i.value();
        refSource.second = -1;
//        int tId = i.value();
//        target_status tStat = m_ri->radarArpa[aId]->m_target[tId]->getStatus();
        target_status tStat = t_ptr->getStatus();

//        qDebug()<<Q_FUNC_INFO<<"radar 0"<<"anthene"<<aId<<"tId"<<tId<<"tStat"<<tStat;

        if((tStat > LOST) && !found)
        {
            refSource.first = 0; //radar 1
            refSource.second = aId;
            clusterTrackStatus = tStat;
            found = true;
        }
        else if (tStat > LOST)
            aId_to_delete.append(aId);
    }

    i.toFront();
    foreach (int cur_aId, aId_to_delete) {
        rTracks[0].remove(cur_aId); //radar 1
    }
}

target_status TracksCluster::getClusterTrackStatus()
{
    updateClusteredTrackStatus();
    return clusterTrackStatus;
}

RadarEngineARND::ARPATarget* TracksCluster::getARPATargetClustered() const
{
    RadarEngineARND::RadarArpa *target;
    RadarEngineARND::RadarEngine *ri;

    if(refSource.first) ri = m_ri1;
    else ri = m_ri;
//    target = ri->radarArpa[refSource.second];

    if(refSource.second > -1)
    {
//        qDebug()<<Q_FUNC_INFO<<"radar source"<<refSource.first<<"anthene source"<<refSource.second<<"track id"<<rTracks[refSource.first].value(refSource.second);
        target = ri->radarArpa[refSource.second];
        return rTracks[refSource.first].value(refSource.second);
    }
    else
    {
        return nullptr;
    }

//    qDebug()<<Q_FUNC_INFO<<"radar source"<<refSource.first<<"anthene source"<<refSource.second<<"track id"<<rTracks[refSource.first].value(refSource.second);

//    return target->m_target[rTracks[refSource.first].value(refSource.second)];
}
int TracksCluster::getClusterTrackNumber() const
{
    return clusterTrackNumber;
}

RadarEngineARND::Position TracksCluster::getClusterPosition() const
{
    return clusterPosition;
}

TrackManager::TrackManager(
        RadarEngineARND::RadarEngine *ri,
        RadarEngineARND::RadarEngine *ri1,
        RadarScene *rs) :
    m_ri(ri),m_ri1(ri1),m_rs(rs)
{
    cur_arpa_id_count = 0;
}

void TrackManager::refreshTarget()
{
    for (int i = 0; i < ANTENE_COUNT; ++i)
        m_ri->radarArpa[i]->RefreshArpaTargets();
}

void TrackManager::refreshTarget1()
{
    for (int i = 0; i < ANTENE_COUNT; ++i)
        m_ri1->radarArpa[i]->RefreshArpaTargets();
}

void TrackManager::reqDelTrack(int id)
{
    qDebug()<<Q_FUNC_INFO<<(int)id;
    if(id>-10)
    {
        int i = 0;
        for (i; i < tracks.size(); ++i)
        {
            TracksCluster *track = tracks.at(i);
            qDebug()<<Q_FUNC_INFO<<"track id"<<track->getClusterTrackNumber()<<"at "<<i;
            if(track->getClusterTrackNumber() == id)
            {
                qDebug()<<Q_FUNC_INFO<<"remove track"<<(int)id<<"at "<<i;
                track->setLostClusteredTrack();
                m_rs->reqDelArpa(track);
                tracks.removeAt(i);
                break;
            }
        }

        for (i; i < tracks.size(); ++i)
        {
            qDebug()<<Q_FUNC_INFO<<"update arpa pointer. track id"<<tracks.at(i)->getClusterTrackNumber()<<"at "<<i;
            tracks.at(i)->updateClusteredTrackPointer();
        }


//        for (int h = 0; h < ANTENE_COUNT; ++h)
//        {
//            for(int i=0;i<m_ri->radarArpa[h]->m_number_of_targets;i++)
//            {
//                if(m_ri->radarArpa[h]->m_target[i]->m_target_id == (int)id)
//                {
//                    m_ri->radarArpa[h]->m_target[i]->SetStatusLost();
//                    qDebug()<<Q_FUNC_INFO<<m_ri->radarArpa[h]->m_target[i]->m_target_id<<(int)id;
//                }
//            }
//        }

//        for (int h = 0; h < ANTENE_COUNT; ++h)
//        {
//            for(int i=0;i<m_ri1->radarArpa[h]->m_number_of_targets;i++)
//            {
//                if(m_ri1->radarArpa[h]->m_target[i]->m_target_id == (int)id)
//                {
//                    m_ri1->radarArpa[h]->m_target[i]->SetStatusLost();
//                    qDebug()<<Q_FUNC_INFO<<m_ri1->radarArpa[h]->m_target[i]->m_target_id<<(int)id;
//                }
//            }
//        }
    }
    else
    {
        qDebug()<<Q_FUNC_INFO<<"delete all track";

        tracks.clear();
        for (int i = 0; i < ANTENE_COUNT; ++i)
            m_ri->radarArpa[i]->DeleteAllTargets();
        for (int i = 0; i < ANTENE_COUNT; ++i)
            m_ri1->radarArpa[i]->DeleteAllTargets();
    }
}

void TrackManager::updateTracks()
{
    int num_limit = 5;
    RadarEngineARND::ARPATarget *target;
    QList<int> target_to_delete;

    qDebug()<<Q_FUNC_INFO<<"tracks"<<tracks.size()<<"cur_arpa_id_count"<<cur_arpa_id_count;
    for (int var = 0; var < tracks.size(); ++var)
    {
        qDebug()<<Q_FUNC_INFO<<"track id"<<tracks.at(var)->getClusterTrackNumber()<<"status"<<tracks.at(var)->getClusterTrackStatus();
    }

    for (int h = 0; h < ANTENE_COUNT; ++h)
    {
        if(state_radar != RADAR_TRANSMIT)
        {
            m_ri->radarArpa[h]->DeleteAllTargets();
            m_ri1->radarArpa[h]->DeleteAllTargets();
        }

        m_ri->radarArpa[h]->RefreshArpaTargets();
        m_ri1->radarArpa[h]->RefreshArpaTargets();
    }

    if(state_radar != RADAR_TRANSMIT) tracks.clear();

    for (int var = 0; var < tracks.size(); ++var)
    {
        if(tracks.at(var)->getClusterTrackStatus() == LOST)
            target_to_delete.append(var);
    }
    for (int var = 0; var < target_to_delete.size(); ++var)
    {
        TracksCluster *track = tracks.at(target_to_delete.at(var));
//        track->setLostClusteredTrack();
        m_rs->reqDelArpa(track);
        tracks.removeAt(target_to_delete.at(var));
    }

    while ((cur_arpa_id_count < tracks.size()) && num_limit > 0)
    {
        target = tracks.at(cur_arpa_id_count)->getARPATargetClustered();
        if(tracks.at(cur_arpa_id_count)->getClusterTrackStatus() > 4)
//            if(target->m_target_id > 0)
        {
            emit signal_target_param(/*target->m_target_id,*/
                                     tracks.at(cur_arpa_id_count)->getClusterTrackNumber(),
                                     target->m_position.rng,
                                     target->m_position.brn,
                                     target->m_position.lat,
                                     target->m_position.lon,
                                     target->m_speed_kn,
                                     target->m_course,
                                     target->m_position.alt,
                                     "-","-",target->selected
                                     );
        }
        cur_arpa_id_count++;
        num_limit--;
    }
    if(cur_arpa_id_count >= tracks.size())
        cur_arpa_id_count = 0;

//    for (int h = 0; h < ANTENE_COUNT; ++h)
//    {
//        m_ri->radarArpa[h]->RefreshArpaTargets();
//        m_ri1->radarArpa[h]->RefreshArpaTargets();

//        if(m_ri->radarArpa[h]->m_number_of_targets > 0)
//        {
//            int num_limit = 5;
//            while ((cur_arpa_id_count < m_ri->radarArpa[h]->m_number_of_targets) && num_limit > 0)
//            {
//                if(m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_target_id > 0)
//                {
//                    emit signal_target_param(m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_target_id,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.rng,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.brn,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.lat,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.lon,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_speed_kn,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_course,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.alt,
//                                                  "-","-",m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->selected
//                                                  );
//                }
//                cur_arpa_id_count++;
//                num_limit--;
//            }
//            if(cur_arpa_id_count >= m_ri->radarArpa[h]->m_number_of_targets)
//                cur_arpa_id_count = 0;
//        }
//    }
}

void TrackManager::reqCreateArpa(QPointF position)
{
    //    qDebug()<<Q_FUNC_INFO<<position;

    RadarEngineARND::Position arpa_pos;
    arpa_pos.lat = position.y();
    arpa_pos.lon = position.x();

    TracksCluster *tracksCluster = new TracksCluster(m_ri,m_ri1,arpa_pos);
    tracks.append(tracksCluster);
    m_rs->reqNewArpa(true, true, tracksCluster);


    //    for (int i = 0; i < ANTENE_COUNT; ++i) {
//        m_ri->radarArpa[i]->AcquireNewMARPATarget(arpa_pos);
//        m_rs->reqNewArpa(true,true,m_ri->radarArpa[i]->m_target[m_ri->radarArpa[i]->m_number_of_targets-1]);
//    }
}

void TrackManager::setTrackRangeMeter(int meter)
{
    for (int i = 0; i < ANTENE_COUNT; ++i) {
        m_ri->radarArpa[i]->range_meters = meter;
        m_ri1->radarArpa[i]->range_meters = meter;
    }
}
