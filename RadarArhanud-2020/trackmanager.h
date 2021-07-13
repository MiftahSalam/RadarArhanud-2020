#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include <QObject>
#include "radarengine_global.h"
#include "radarengine.h"
#include "radarscene.h"

class RadarScene;

struct Track
{
    int radarId;
    struct TrackAnthene
    {
        int antheneId;
        int trackId;
    }source;
};

class TracksCluster : public QObject
{
    Q_OBJECT
public:
    TracksCluster(
            RadarEngineARND::RadarEngine *ri,
            RadarEngineARND::RadarEngine *ri1,
            RadarEngineARND::Position pos
    );
    virtual ~TracksCluster() {}

    void updateClusteredTrackStatus();
    void updateClusteredTrackPointer();
    void setLostClusteredTrack();
    RadarEngineARND::Position getClusterPosition() const;
    int getClusterTrackNumber() const ;
    int getClusterAntheneId() const { return refSource.second; }
    int getClusterRadarId() const { return refSource.second; }
    Track getClusterTrack() const;
    RadarEngineARND::target_status getClusterTrackStatus();
    RadarEngineARND::ARPATarget *getARPATargetClustered() const;

private:
    RadarEngineARND::RadarEngine *m_ri, *m_ri1;
    RadarEngineARND::ARPATarget *m_arpa;
    RadarEngineARND::Position clusterPosition;
    QMap<
        int, //anthene id
//        int //trackid
        RadarEngineARND::ARPATarget*
    > rTracks[2];
    QPair<
        int, //radarId
        int //anthene_id
    > refSource;
    int clusterTrackNumber;
    RadarEngineARND::target_status clusterTrackStatus;
};

class TrackManager : public QObject
{
    Q_OBJECT
public:
    TrackManager(
            RadarEngineARND::RadarEngine *ri,
            RadarEngineARND::RadarEngine *ri1,
            RadarScene *rs
            );
    void refreshTarget();
    void refreshTarget1();
    void updateTracks();
    void reqDelTrack(int id);
    void setTrackRangeMeter(int meter);
    void reqCreateArpa(QPointF position);

signals:
    void signal_target_param(quint32 id,
                             double rng,
                             double brn,
                             double lat,
                             double lon,
                             double spd,
                             double crs,
                             double alt,
                             QString call_sign,
                             QString country,
                             bool selected
                             );

private:
    RadarEngineARND::RadarEngine *m_ri, *m_ri1;
    QList<TracksCluster*> tracks;
    RadarScene *m_rs;
    int cur_arpa_id_count;
};

#endif // TRACKMANAGER_H
